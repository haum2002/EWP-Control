// SmartCooling RobustDataLogger — resilient SD card data logging.
//
// Reka bentuk:
//  - Buffer berkelompok (batched ring) saiz LOG_BUFFER_SIZE: logData() tidak
//    menyentuh disk, hanya menyalin ke dalam buffer. flush() menulis jarang-jarang.
//  - Auto-rotate fail CSV apabila saiz melebihi FILE_ROTATION_SIZE_MB (5 MB),
//    mengekalkan maksimum MAX_LOG_FILES fail berasingan.
//  - CRC32 setiap rekod (lajur ke-13) untuk pengesanan rasuah/kerosakan.
//  - Retry bertingkat (MAX_RETRY_COUNT) dengan buka-semula fail jika tulis gagal.
//  - dikawal sepenuhnya oleh SC_FACTORY_SD_CARD_ENABLED: jika 0, logger disahaktifkan
//    secara selamat (begin()->false, logData()/flush() no-op) tanpa menyentuh SPI/SD.
//
// Catatan pin: pin SPI SD (CS/MOSI/MISO/SCK) hanya #define dalam factory_config.h
// apabila SC_FACTORY_SD_CARD == 1. Sebarang rujukan pin mesti berada di dalam
// blok #if yang sama — tidak boleh bocor ke laluan terhapus.

#include "data_logger.h"

#if SC_FACTORY_SD_CARD_ENABLED == 1
  #include <SPI.h>
#endif

// ============================================================================
// CRC32 (IEEE 802.3, polynomial dipantul 0xEDB88320, init 0xFFFFFFFF, XOR akhir
// 0xFFFFFFFF). Hasilnya serasi dengan zlib/PNG/CRC32 biasa. Setiap rekod CSV
// dihitung CRC32 ke atas 12 medan terdokumen, kemudian dilampirkan sebagai
// lajur ke-13 (hex 8 aksara) untuk pengesanan rasuah/kerosakan baris.
// ============================================================================
static uint32_t _crc32_update(uint32_t crc, const uint8_t *data, size_t len) {
    crc ^= 0xFFFFFFFFul;
    for (size_t i = 0; i < len; ++i) {
        crc ^= data[i];
        for (uint8_t b = 0; b < 8; ++b) {
            uint32_t mask = -(crc & 1u);
            crc = (crc >> 1) ^ (0xEDB88320ul & mask);
        }
    }
    return crc ^ 0xFFFFFFFFul;
}

// Instance global tunggal (diisytihar dalam header).
RobustDataLogger DataLogger;

// ============================================================================
// KONSTRUKTOR
// ============================================================================
RobustDataLogger::RobustDataLogger()
    : _sd_ready(false)
    , _buffer_idx(0)
    , _last_flush_ms(0)
    , _dropped_count(0)
    , _total_logged(0)
    , _currentFileSize(0) {
}

// ============================================================================
// BEGIN — mengaktifkan logger jika SD diaktifkan pada peringkat kilang.
// ============================================================================
bool RobustDataLogger::begin() {
#if SC_FACTORY_SD_CARD_ENABLED == 1
    if (!_initSD()) {
        _sd_ready = false;
        Serial.println(F("[DATALOG] SD init gagal — logging dinyahaktifkan."));
        return false;
    }
    _sd_ready = true;
    _currentFileName = _generateFileName();
    // Buka fail sedia ada dalam mod append (sambung), atau cipta baharu.
    _logFile = SD.open(_currentFileName.c_str(), FILE_APPEND);
    if (!_logFile) {
        // Mungkin fail rosak — cuba baiki sekali.
        if (repairCorruptedFile()) {
            Serial.print(F("[DATALOG] Fail disambung: "));
        } else {
            _sd_ready = false;
            Serial.println(F("[DATALOG] Tidak boleh membuka fail log."));
            return false;
        }
    } else {
        Serial.print(F("[DATALOG] Fail log sedia: "));
    }
    Serial.println(_currentFileName);
    _currentFileSize = _logFile.size();
    _last_flush_ms = millis();
    return true;
#else
    // SD dinyahaktifkan pada peringkat kilang — logger kekal no-op selamat.
    _sd_ready = false;
    return false;
#endif
}

// ============================================================================
// INIT SD — konfigurasi SPI dengan pin kilang & SD.begin()
// ============================================================================
bool RobustDataLogger::_initSD() {
#if SC_FACTORY_SD_CARD_ENABLED == 1
    // Pin SPI SD hanya wujud apabila SC_FACTORY_SD_CARD == 1.
    SPI.begin(SC_FACTORY_PIN_SD_SCK, SC_FACTORY_PIN_SD_MISO,
              SC_FACTORY_PIN_SD_MOSI, SC_FACTORY_PIN_SD_CS);
    if (!SD.begin(SC_FACTORY_PIN_SD_CS)) {
        return false;
    }
    Serial.println(F("[DATALOG] SD card dimulakan."));
    return true;
#else
    return false;
#endif
}

// ============================================================================
// LOG DATA — salin masuk buffer (tidak menyekat). flush() dipanggil jika penuh
// atau selang auto-flush tercapai.
// ============================================================================
void RobustDataLogger::logData(const LogEntry &entry) {
    if (!_sd_ready) {
        return;  // Logging tidak diaktifkan — diam sahaja.
    }
    _buffer[_buffer_idx] = entry;
    _buffer_idx++;
    _total_logged++;

    uint32_t now = millis();
    if (_buffer_idx >= LOG_BUFFER_SIZE) {
        flush();
    } else if ((now - _last_flush_ms) >= LOG_FLUSH_INTERVAL_MS) {
        flush();
    }
}

// ============================================================================
// FLUSH — tulis semua entri terbuffer ke disk dengan retry, kemudian semak
// saiz fail untuk putar-ganti (rotate).
// ============================================================================
void RobustDataLogger::flush() {
    if (!_sd_ready || _buffer_idx == 0) {
        _last_flush_ms = millis();
        return;
    }

    char line[192];
    for (uint8_t i = 0; i < _buffer_idx; ++i) {
        _formatCSV(line, sizeof(line), _buffer[i]);
        if (!_writeToDisk(line)) {
            _dropped_count++;  // gagal bertulis selepas retry — jatuhkan rekod.
        }
    }
    _buffer_idx = 0;
    _last_flush_ms = millis();

    // Periksa putar-ganti fail.
    if (_checkFileSize()) {
        rotateFile();
    }
}

// ============================================================================
// WRITE TO DISK — tulis satu baris CSV + newline, dengan retry bertingkat.
// Jika gagal, cuba buka semula fail.
// ============================================================================
bool RobustDataLogger::_writeToDisk(const char *line) {
#if SC_FACTORY_SD_CARD_ENABLED == 1
    if (!_sd_ready) {
        return false;
    }
    for (uint8_t attempt = 0; attempt < MAX_RETRY_COUNT; ++attempt) {
        if (!_logFile) {
            _logFile = SD.open(_currentFileName.c_str(), FILE_APPEND);
            if (!_logFile) {
                continue;  // cuba lagi
            }
        }
        size_t before = _logFile.position();
        size_t n = _logFile.println(line);
        if (n > 0) {
            _logFile.flush();  // paksa tulis ke kad SD.
            _currentFileSize = _logFile.size();
            return true;
        }
        // Tulis gagal — tutup & buka semula untuk percubaan seterusnya.
        _logFile.close();
        _logFile = SD.open(_currentFileName.c_str(), FILE_APPEND);
        (void)before;
    }
    // Selepas semua retry gagal, isyaratkan SD bermasalah.
    return false;
#else
    (void)line;
    return false;
#endif
}

// ============================================================================
// FORMAT CSV — 12 medan terdokumen + lajur ke-13 CRC32 (hex).
// Format: ts_ms,rtc_ts,setpoint,temp_c,output_pct,pump_st,fan_st,ecu_fan_st,
//         fault,risk_score,stability_idx,sensor_conf,mode,crc32
// ============================================================================
void RobustDataLogger::_formatCSV(char *buffer, size_t max_len, const LogEntry &e) {
    // Medan 1..12 (tanpa CRC dahulu) untuk dihitung CRC32.
    int len = snprintf(buffer, max_len,
        "%lu,%lu,%.2f,%.2f,%.2f,%d,%d,%d,%u,%.3f,%.3f,%u,%u",
        (unsigned long)e.timestamp_ms,
        (unsigned long)e.rtc_epoch,
        e.setpoint,
        e.temp_c,
        e.output_pct,
        e.pump_state ? 1 : 0,
        e.fan_state ? 1 : 0,
        e.ecu_fan_state ? 1 : 0,
        (unsigned)e.fault_code,
        e.risk_score,
        e.stability_idx,
        (unsigned)e.sensor_conf,
        (unsigned)e.operation_mode);

    if (len <= 0 || (size_t)len >= max_len) {
        // Tampungan terlalu kecil — potong selamat.
        if (max_len > 0) buffer[max_len - 1] = '\0';
        return;
    }

    // Hitung CRC32 ke atas 12 medan yang baru ditulis (sebelum ',crc32').
    uint32_t crc = _crc32_update(0, (const uint8_t *)buffer, (size_t)len);

    // Lampirkan lajur ke-13: ',<crc32 hex 8 aksara>'.
    int remain = (int)max_len - len;
    int add = snprintf(buffer + len, (size_t)remain, ",%08lx", (unsigned long)crc);
    if (add < 0 || (size_t)add >= (size_t)remain) {
        // Tampungan tidak cukup untuk CRC — potong.
        if (max_len > 0) buffer[max_len - 1] = '\0';
    }
}

// ============================================================================
// CHECK FILE SIZE — true jika fail melebihi had putar-ganti.
// ============================================================================
bool RobustDataLogger::_checkFileSize() {
#if SC_FACTORY_SD_CARD_ENABLED == 1
    if (!_sd_ready || !_logFile) {
        return false;
    }
    _currentFileSize = _logFile.size();
    uint32_t limit = (uint32_t)FILE_ROTATION_SIZE_MB * 1024ul * 1024ul;
    return _currentFileSize >= limit;
#else
    return false;
#endif
}

// ============================================================================
// GENERATE FILE NAME — pilih nama fail berikutnya.
// Imbuh dari indeks 0..MAX_LOG_FILES-1; pilih indeks pertama yang fail-nya
// belum wujud ATAU saiznya masih di bawah had. Jika semua penuh, pilih 00
// (akan ditulis-ganti pada rotateFile()).
// ============================================================================
String RobustDataLogger::_generateFileName() {
#if SC_FACTORY_SD_CARD_ENABLED == 1
    uint32_t limit = (uint32_t)FILE_ROTATION_SIZE_MB * 1024ul * 1024ul;
    for (uint8_t idx = 0; idx < MAX_LOG_FILES; ++idx) {
        char name[32];
        snprintf(name, sizeof(name), "/sc_u%04d_%02d.log",
                 (int)SYSTEM_UNIT_NUMBER, idx);
        String path = String(name);
        if (!SD.exists(path)) {
            return path;  // slot kosong — guna ini.
        }
        // Wujud: semak saiz. Jika masih ada ruang, sambung (append).
        File f = SD.open(path, FILE_READ);
        if (f) {
            uint32_t sz = f.size();
            f.close();
            if (sz < limit) {
                return path;
            }
        }
    }
    // Semua slot penuh — tindih yang tertua (indeks 0).
    char name0[32];
    snprintf(name0, sizeof(name0), "/sc_u%04d_00.log", (int)SYSTEM_UNIT_NUMBER);
    return String(name0);
#else
    return String();
#endif
}

// ============================================================================
// ROTATE FILE — tutup fail semasa, buka fail berikutnya (tulis baharu).
// ============================================================================
void RobustDataLogger::rotateFile() {
#if SC_FACTORY_SD_CARD_ENABLED == 1
    if (!_sd_ready) {
        return;
    }
    if (_logFile) {
        _logFile.flush();
        _logFile.close();
    }

    // Cari slot seterusnya: indeks semasa +1 mod MAX_LOG_FILES.
    // Kitar semula: jika kita pada slot N, slot berikutnya = (N+1) % MAX.
    // _generateFileName() sudah cukup bijak untuk memilih slot terbaik, jadi
    // kita panggil semula — ini memberikan slot kosong terendah atau slot 00
    // jika semuanya penuh (akan ditulis-ganti).
    String oldName = _currentFileName;
    _currentFileName = _generateFileName();

    // Jika nama sama (semua penuh & kita kembali ke fail semasa), tulis-ganti.
    if (_currentFileName == oldName) {
        _logFile = SD.open(_currentFileName.c_str(), FILE_WRITE);  // truncate
    } else {
        _logFile = SD.open(_currentFileName.c_str(), FILE_APPEND);
    }
    if (!_logFile) {
        _sd_ready = false;
        Serial.println(F("[DATALOG] rotateFile gagal — SD ditandai tidak sedia."));
        return;
    }
    _currentFileSize = 0;
    Serial.print(F("[DATALOG] Diputar ke fail: "));
    Serial.println(_currentFileName);
#else
    return;
#endif
}

// ============================================================================
// HANDLE SD FULL — cuba pulihkan selepas kegagalan disk (buka semula).
// ============================================================================
bool RobustDataLogger::handleSDFull() {
#if SC_FACTORY_SD_CARD_ENABLED == 1
    if (_logFile) {
        _logFile.close();
    }
    if (!_initSD()) {
        _sd_ready = false;
        return false;
    }
    _sd_ready = true;
    _currentFileName = _generateFileName();
    _logFile = SD.open(_currentFileName.c_str(), FILE_APPEND);
    if (!_logFile) {
        _sd_ready = false;
        return false;
    }
    _currentFileSize = _logFile.size();
    return true;
#else
    return false;
#endif
}

// ============================================================================
// REPAIR CORRUPTED FILE — buang fail rosak & buka semula yang segar.
// ============================================================================
bool RobustDataLogger::repairCorruptedFile() {
#if SC_FACTORY_SD_CARD_ENABLED == 1
    if (!_sd_ready) {
        return false;
    }
    if (_logFile) {
        _logFile.close();
    }
    if (_currentFileName.length() && SD.exists(_currentFileName)) {
        SD.remove(_currentFileName);
    }
    _logFile = SD.open(_currentFileName.c_str(), FILE_WRITE);  // cipta segar
    if (!_logFile) {
        _sd_ready = false;
        return false;
    }
    _currentFileSize = 0;
    Serial.println(F("[DATALOG] Fail rosak dibuang & dibina semula."));
    return true;
#else
    return false;
#endif
}

// ============================================================================
// EXPORT TO SERIAL — strim kandungan fail log semasa ke Serial (dipanggil
// daripada endpoint diagnostik Web UI).
// ============================================================================
bool RobustDataLogger::exportToSerial() {
#if SC_FACTORY_SD_CARD_ENABLED == 1
    if (!_sd_ready || _currentFileName.length() == 0) {
        Serial.println(F("[DATALOG] Tiada fail log untuk dieksport."));
        return false;
    }
    File f = SD.open(_currentFileName, FILE_READ);
    if (!f) {
        Serial.println(F("[DATALOG] Tidak boleh membuka fail untuk eksport."));
        return false;
    }
    Serial.println(F("===== BEGIN LOG EXPORT ====="));
    while (f.available() && f.position() < f.size()) {
        String line = f.readStringUntil('\n');
        Serial.println(line);
    }
    Serial.println(F("===== END LOG EXPORT ====="));
    f.close();
    return true;
#else
    Serial.println(F("[DATALOG] SD tidak diaktifkan — tiada eksport."));
    return false;
#endif
}

// ============================================================================
// DELETE OLD LOGS — kekalkan `keep_count` fail terkini, buang selebihnya.
// Strategi: namakan fail sebagai /sc_uXXXX_NN.log; indeks lebih rendah dianggap
// "lebih lama" selepas putar-ganti penuh. Kita kekal `keep_count` indeks
// tertinggi, buang selebihnya.
// ============================================================================
bool RobustDataLogger::deleteOldLogs(uint8_t keep_count) {
#if SC_FACTORY_SD_CARD_ENABLED == 1
    if (keep_count >= MAX_LOG_FILES) {
        return true;  // kekalkan semuanya — tiada kerja.
    }
    uint8_t removed = 0;
    // Indeks tertinggi = terbaru (kami baru sahaja menulis kepadanya sebelum
    // putar-ganti). Jadi indeks paling rendah (0,1,..) adalah paling lama.
    // Untuk penentukuran mesra-pengguna, kekalkan keep_count indeks tertinggi
    // iaitu (MAX_LOG_FILES - keep_count .. MAX_LOG_FILES - 1)? Namun kerana
    // indeks dikitar, lebih selamat: buang indeks 0 .. (MAX - keep_count - 1).
    uint8_t first_keep = (uint8_t)(MAX_LOG_FILES - keep_count);
    for (uint8_t idx = 0; idx < first_keep && idx < MAX_LOG_FILES; ++idx) {
        char name[32];
        snprintf(name, sizeof(name), "/sc_u%04d_%02d.log",
                 (int)SYSTEM_UNIT_NUMBER, idx);
        String path = String(name);
        if (SD.exists(path)) {
            if (SD.remove(path)) {
                removed++;
            }
        }
    }
    Serial.print(F("[DATALOG] Fail lama dibuang: "));
    Serial.println(removed);
    return removed > 0 || keep_count >= MAX_LOG_FILES;
#else
    (void)keep_count;
    return false;
#endif
}

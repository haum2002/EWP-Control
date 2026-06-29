#ifndef DATA_LOGGER_H
#define DATA_LOGGER_H

#include <FS.h>
#include <SD.h>
#include "si_core.h"

// ============================================================================
// KONFIGURASI DATA LOGGER (OVER-ENGINEERED)
// ============================================================================

#define LOG_BUFFER_SIZE 20           // Triple-buffering
#define LOG_FLUSH_INTERVAL_MS 2000   // Auto-flush setiap 2 saat
#define MAX_RETRY_COUNT 3            // Retry jika gagal tulis
#define FILE_ROTATION_SIZE_MB 5      // Rotate fail jika > 5MB
#define MAX_LOG_FILES 10             // Simpan maksimum 10 fail log

// Format CSV: ts_ms,rtc_ts,setpoint,temp_c,output_pct,pump_st,fan_st,ecu_fan_st,fault,risk_score,stability_idx,sensor_conf,mode
struct LogEntry {
    uint32_t timestamp_ms;
    uint32_t rtc_epoch;
    float setpoint;
    float temp_c;
    float output_pct;
    bool pump_state;
    bool fan_state;
    bool ecu_fan_state;      // Input ECU
    uint16_t fault_code;
    float risk_score;        // Metrik SI
    float stability_idx;     // Metrik SI
    uint8_t sensor_conf;     // Keyakinan sensor (0-255)
    uint8_t operation_mode;
};

// ============================================================================
// KELAS ROBUST DATA LOGGER
// ============================================================================

class RobustDataLogger {
public:
    RobustDataLogger();
    
    bool begin();
    void logData(const LogEntry &entry);
    void flush();
    
    // Pengurusan Fail Pintar
    bool handleSDFull();
    bool repairCorruptedFile();
    void rotateFile();
    
    // Status
    bool isReady() const { return _sd_ready; }
    uint32_t getDroppedCount() const { return _dropped_count; }
    uint32_t getTotalLogged() const { return _total_logged; }
    String getCurrentFileName() const { return _currentFileName; }
    
    // Eksport Log
    bool exportToSerial();
    bool deleteOldLogs(uint8_t keep_count);

private:
    bool _sd_ready;
    File _logFile;
    LogEntry _buffer[LOG_BUFFER_SIZE];
    uint8_t _buffer_idx;
    uint32_t _last_flush_ms;
    uint32_t _dropped_count;
    uint32_t _total_logged;
    String _currentFileName;
    uint32_t _currentFileSize;
    
    bool _initSD();
    bool _writeToDisk(const char* line);
    String _generateFileName();
    bool _checkFileSize();
    void _formatCSV(char* buffer, size_t max_len, const LogEntry& entry);
};

extern RobustDataLogger DataLogger;

#endif // DATA_LOGGER_H

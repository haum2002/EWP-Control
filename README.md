# 🌡️ SmartCooling
### Sistem Penyejukan Pintar Generasi Akan Datang untuk ESP32-S3

[![Platform](https://img.shields.io/badge/Platform-ESP32--S3-blue?logo=espressif)](https://www.espressif.com/)
[![Language](https://img.shields.io/badge/Language-C++-orange?logo=cplusplus)](https://isocpp.org/)
[![Connectivity](https://img.shields.io/badge/WiFi-2.4GHz-green?logo=wifi)](https://en.wikipedia.org/wiki/Wi-Fi)
[![Status](https://img.shields.io/badge/Status-Production%20Ready-brightgreen)](.)

**SmartCooling** bukan sekadar pengawal suhu; ia adalah ekosistem pintar yang menggabungkan kawalan PID adaptif, ketahanan kuasa peringkat industri, dan antara muka web yang memukau. Dibina khusus untuk papan **Super Mini ESP32 S3 HW-747**, sistem ini direka untuk kestabilan maksimum dalam persekitaran yang mencabar.

---

## ✨ Mengapa SmartCooling?

| Ciri Utama | Penerangan |
| :--- | :--- |
| 🧠 **Kecerdasan Buatan (SI)** | Algoritma *Adaptive PID* & *Feed-Forward* yang belajar daripada persekitaran. |
| ⚡ **Ketahanan Kuasa** | Teknologi *RTC Memory Persistence*; sistem ingat keadaan terakhir walaupun elektrik putus. |
| 🎨 **Web UI Premium** | Antara muka "Liquid Smooth", minimalis, dwibahasa, dan tiada branding luar. |
| 🔒 **Keselamatan Teras** | Nombor siri unik automatik, PIN untuk tetapan kritikal, dan validasi OTA pintar. |
| 🛠️ **Konfigurasi Kilang** | Fleksibel untuk pelbagai jenis pam (PWM/SSR), kipas, dan sensor melalui `factory_config.h`. |

---

## 🚀 Ciri-ciri Terperinci

### 1. Perkakasan & Fleksibiliti Kilang
Dibina untuk menyesuaikan diri dengan keperluan spesifik anda sebelum *flashing*:
- **Pam & Kipas:** Sokongan penuh untuk kawalan kelajuan (PWM) atau suis lembut (SSR).
- **Sensor Persekitaran:** Serasi dengan BME280, AHT30, BMP280, BMP180, atau mod tanpa sensor.
- **Data Logging:** Integrasi Micro SD card (pilihan) untuk rekod data terperinci dan latihan AI masa depan.
- **Pengurusan Pin Dinamik:** Pemetaan pin automatik mengikut konfigurasi untuk mengelakkan konflik.

### 2. Identiti & Keselamatan Sistem
Setiap unit adalah unik dan terlindung:
- **Nombor Siri Automatik:** Dijana semasa *boot* dalam format `VVMMYYK####` (Contoh: `010629K0042`).
- **OTA Pintar:** Mekanisme kemaskini *Over-The-Air* dengan pengesahan keserasian firmware dan popup konfirmasi berganda.
- **WebApp Secure:** Perlindungan *rate-limiting*, sanitasi input, dan wajibkan PIN untuk mengubah kata laluan admin.

### 3. Kecerdasan Adaptif (Super Intelligent)
Algoritma kawalan yang melampaui PID tradisional:
- **Adaptive PID:** Parameter `Kp`, `Ki`, `Kd` laras sendiri berdasarkan kestabilan sistem masa nyata.
- **Prediksi Anomali:** Mengira *Risk Score* untuk mengesan potensi kegagalan sebelum ia berlaku.
- **Optimasi Matematik:** Pengiraan pantas tanpa menghalang gelung kawalan utama (*non-blocking*).

### 4. Antara Muka Pengguna (Web UI)
Reka bentuk ulang total untuk pengalaman pengguna terbaik:
- **Navigasi Hamburger:** Menu sisi yang licin dengan indikator RGB status sistem.
- **Dashboard Real-time:** Graf suhu, output, dan voltan yang bergerak lancar.
- **Mod Gelap/Cerah:** Tukar tema serta-merta mengikut keselesaan mata.
- **Log Keluar & Keamanan:** Butang log keluar yang jelas dan sesi yang terurus.

---

## 🔋 Ketahanan Kuasa (Power Loss Resilience)

Sistem dilengkapi dengan mekanisme penyelamatan data ke **RTC Memory**:
1.  **Auto-Save:** Keadaan sistem disimpan setiap 500ms.
2.  **Validasi Checksum:** Memastikan data tidak rosak semasa gangguan kuasa mendadak.
3.  **Recovery Pantas:** Apabila kuasa kembali, sistem memulakan semula dari keadaan terakhir secara automatik tanpa perlu konfigurasi semula.

---

## 📊 Struktur Data & Log

Jika Micro SD diaktifkan, data direkodkan dalam format CSV yang efisien:
```csv
ts_ms,rtc_ts,setpoint,temp_c,output_pct,pump_st,fan_st,fault,mode
1719648201,1024,65.0,64.8,45,1,1,0,auto
```
*Direka untuk analisis data besar dan latihan model Machine Learning di masa hadapan.*

---

## 🛠️ Mula Sekarang

### Prasyarat
- Board: **Super Mini ESP32 S3 HW-747**
- IDE: PlatformIO atau Arduino IDE (dengan sokongan ESP32)
- Konfigurasi: Edit fail `src/factory_config.h` mengikut perkakasan anda.

### Langkah Pemasangan
1.  **Clone Repositori:**
    ```bash
    git clone https://github.com/haum2002/SmartCoolingv2.git
    cd SmartCoolingv2
    ```
2.  **Konfigurasi Kilang:**
    Buka `src/factory_config.h` dan pilih jenis pam, kipas, serta sensor anda.
3.  **Flash Firmware:**
    Muat naik kod ke papan ESP32-S3 anda.
4.  **Akses Web UI:**
    Sambungkan ke WiFi AP `SmartCooling_XXXXXX` dan layari `192.168.4.1`.

---

## 📑 Dokumentasi Lanjut

Untuk maklumat teknikal yang mendalam, sila rujuk dokumentasi berikut:
- 📘 **[Panduan Pembangun](docs/PANDUAN_PEMBANGUN.md)** - Spesifikasi pin, protokol, dan arsitektur kod.
- 📕 **[Manual Penggunaan](docs/MANUAL_PENGGUNAAN.md)** - Panduan lengkap WebApp, konfigurasi AP, dan tafsiran lampu RGB.

---

## 📈 Status Pembangunan

| Fasa | Komponen | Status |
| :--- | :--- | :--- |
| **Fasa 1** | Infrastruktur Data Logger | ✅ Selesai |
| **Fasa 2** | Model SI & Logik Adaptif | ✅ Selesai |
| **Fasa 3** | Web UI Rombakan Total | ✅ Selesai |
| **Fasa 4** | Ketahanan Kuasa (RTC) | ✅ Selesai |
| **Fasa 5** | Dokumentasi & Penyelarasan | ✅ Selesai |

---

> **Dibina dengan presisi untuk kestabilan maksimum.**
> *SmartCooling © 2024-2025. Hak Cipta Terpelihara.*

## Ciri Utama

### 1. Ketahanan Kuasa (RTC Memory)
- Sistem menyimpan keadaan operasi ke RTC Memory setiap 500ms
- Pemulihan automatik selepas gangguan kuasa atau brownout
- Checksum CRC16 untuk validasi integriti data
- Boot counter berasingan dalam RTC untuk diagnostik
- Buffer 8 peristiwa terakhir disimpan dalam RTC

### 2. Nombor Siri Automatik
- Format: `VVMMYYK####` (Versi, Bulan, Tahun, Kod Spec, Nombor Urut)
- Dijana automatik berdasarkan tarikh compilation dan MAC address
- Contoh: `010629K1234` = Versi 01, Jun 2026, Kod K, Unit 1234
- Digunakan untuk pengesahan OTA dan identiti sistem

### 3. SmartCooling SI (Super Intelligent)
- **Adaptive PID**: Parameter Kp, Ki, Kd berubah dinamik berdasarkan kestabilan
- **Feed-Forward Control**: Menjangka keperluan output berdasarkan kadar perubahan suhu
- **Prediksi Suhu**: Anggaran suhu 60 saat masa hadapan untuk tindak balas awal
- **Risk Score**: Skor risiko 0-100 untuk mengesan anomali awal (Predictive Fault)
- **Stability Index**: Metrik kestabilan sistem real-time
- **Data Logging**: Struktur CSV piawai untuk latihan AI masa depan (jika SD aktif)

### 4. Web UI Rombakan Total
- **Reka Bentuk**: Minimalis, profesional, estetik, "liquidity" (licin), tiada branding luar
- **Menu Hamburger**: Slide dari tepi dengan indikator RGB status
- **Header Ringkas**: Status WiFi AP, Mod, Fault/OK sahaja
- **Dashboard Fokus**: Suhu Semasa, Output, Voltan, graf real-time, carta trend
- **Log Keluar**: Butang jelas di bahagian bawah menu sidebar
- **UI Dinamik**: Hanya paparkan menu/ciri yang wujud dalam konfigurasi kilang
- **Indikator RGB**: Warna smooth (hijau=normal, kuning=amaran, merah=kesalahan)
- **Dwibahasa**: Melayu (ms) dan English (en)
- **Tema**: Gelap dan Cerah

## Struktur Projek

| Laluan | Fungsi |
| --- | --- |
| `web/index.html` | UI WebApp V2, termasuk bahasa `ms` dan `en`, menu hamburger, RGB, logout |
| `src/main.cpp` | Firmware utama, AP, keselamatan, endpoint, kawalan EWP/kipas, RTC Memory, SI |
| `include/web_assets.h` | Fail jana automatik daripada `web/index.html` |
| `include/routes.h` | Fail jana automatik daripada `config/routes.json` |
| `config/routes.json` | Punca rasmi laluan endpoint release rawak |
| `tools/pio_embed_assets.py` | Script jana aset WebApp dan route header |
| `docs/MANUAL_PENGGUNAAN.md` | Manual lengkap untuk pengguna |
| `docs/PANDUAN_PEMBANGUN.md` | Nota pembangun dan peta endpoint |

Repository ini hanya mengandungi bahan projek utama yang diperlukan untuk build,
flash, penggunaan, dan pembangunan.

## Build Dan Upload

Clone:

```powershell
git clone https://github.com/haum2002/SmartCoolingv2.git
cd SmartCoolingv2
```

Build:

```powershell
pio run -e super_mini_esp32_s3_hw-747
```

Upload:

```powershell
pio run -e super_mini_esp32_s3_hw-747 -t upload
```

Untuk Serial Monitor:

```powershell
pio device monitor --port COM8 --baud 115200
```

Jika port berbeza, ubah `upload_port` dalam `platformio.ini` atau gunakan
`--upload-port COMx`.

## Ringkasan Penggunaan

1. Hidupkan board.
2. Sambung telefon atau komputer kepada Wi-Fi `EWP-SYSTEM-PRO`.
3. Buka `http://smartcooling.local/`.
4. Log masuk menggunakan kata laluan WebApp.
5. Jika terlupa kata laluan, gunakan panel pemulihan dengan PIN `747747` dan
   kata laluan baharu. Nombor siri tidak diperlukan untuk pemulihan.

AP akan dimatikan selepas 5 minit jika tiada pelanggan tersambung. Jika ada
pelanggan tersambung, AP kekal aktif walaupun WebApp tidak digunakan.

## Dokumentasi

Baca dokumen berikut untuk arahan lengkap:

- `docs/MANUAL_PENGGUNAAN.md` - Panduan penggunaan WebApp, konfigurasi AP, prosedur OTA, makna RGB, RTC Memory
- `docs/PANDUAN_PEMBANGUN.md` - Spesifikasi pin, konfigurasi kilang, protokol, RTC Memory, nombor siri automatik

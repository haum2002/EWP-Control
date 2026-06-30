# SmartCooling
### Super Intelligent Thermal Management System | ESP32-S3 Powered

![Status](https://img.shields.io/badge/status-stable-green)
![Version](https://img.shields.io/badge/version-1.0.0-blue)
![License](https://img.shields.io/badge/license-MIT-orange)
![Build](https://img.shields.io/badge/build-PlatformIO%20%26%20ESP--IDF-success)
![Security](https://img.shields.io/badge/security-SI%20Sentinel%20Active-critical)

> **SmartCooling** bukan sekadar pengawal suhu. Ia adalah sistem **Super Intelligence (SI)** yang sedar situasi, mampu memprediksi kegagalan, menahan serangan siber-logik, dan beroperasi secara autonomi dalam persekitaran ekstrem. Dibina dengan arkitektur modular hibrid untuk kestabilan maksimum.

---

## 🚀 Ciri Utama & Keunikan

### 🧠 Super Intelligence (SI) Core
Bukan AI biasa. SI kami menggabungkan kawalan adaptif, model fizik termodinamik, dan analisis risiko masa nyata.
- **Adaptive PID & Feed-Forward:** Menyesuaikan parameter secara dinamik berdasarkan beban haba.
- **Predictive Fault Detection:** Mengesan anomali (cagaran sensor, kegagalan kipas) sebelum ia menjadi kritikal.
- **Risk Score Engine:** Penilaian risiko berterusan (0.0 - 1.0) untuk membuat keputusan keselamatan proaktif.
- **Sensor Fusion:** Menggabungkan data sensor fizik, input ECU/ECM, dan model dalaman untuk ketepatan mutlak.

### 🛡️ SI Sentinel (Neural-Safe Core)
Lapisan pertahanan aktif yang melindungi integriti sistem daripada "hallucination" SI dan serangan bukan fizikal.
- **Behavioral Guard:** Memastikan setiap output SI berada dalam had fizikal yang selamat.
- **Cyber-Logic Defense:** Sanitasi input, anti-injection, dan rate limiting untuk antaramuka web & data.
- **Memory Integrity Watch:** Pemantauan heap/stack masa nyata dengan mekanisme *canary* untuk mencegah korupsi memori.
- **Watchdog Bertingkat:** Pemulihan automatik daripada hang atau deadlock dalam milisaat.

### 💾 Ketahanan Data & Kuasa
- **RTC Memory Persistence:** Menyimpan keadaan sistem semasa gangguan kuasa; pemulihan <500ms.
- **Robust Data Logger:** Triple-buffering, auto-rotation fail, dan validasi CRC32 untuk log pada MicroSD.
- **Factory Reset Ganda:** Pilihan reset melalui Web UI (dilindungi PIN) atau Pin Jumper Fizikal.

### 🏭 Konfigurasi Kilang Selamat (Secure Defaults)
Untuk melindungi perkakasan pihak ketiga, semua output dimatikan secara lalai sehingga dikonfigurasi:
- **Pam & Kipas:** `OFF` (Wajib konfigurasi manual PWM/SSR).
- **Sensor Persekitaran:** `Tiada` (Pilihan: BME280, AHT30, dll).
- **MicroSD:** `Tidak Aktif` (Aktifkan manually untuk logging).

---

## 📦 Sokongan Dual-Framework

Projek ini dibina untuk berjalan serasi pada kedua-dua ekosistem tanpa perubahan kod:

| Framework | Status | Konfigurasi |
| :--- | :---: | :--- |
| **PlatformIO** | ✅ Stabil | Gunakan `platformio.ini` |
| **ESP-IDF** | ✅ Stabil | Gunakan `CMakeLists.txt` & `sdkconfig` |

---

## 📊 Dashboard & Antara Muka

Web UI yang dibina semula sepenuhnya dengan prinsip **"Liquid & Minimalist"**:
- **Navigasi Hamburger:** Menu sisi yang licin dengan indikator RGB status.
- **Dashboard Real-Time:** Graf suhu, output, dan metrik SI (Risk/Stability).
- **Kawalan Keselamatan:** Borang input dengan validasi had (bawah < atas) dan pengesahan PIN.
- **Diagnostik Terintegrasi:** Log peristiwa, eksport data, dan status perkakasan dalam satu pandangan.

---

## 🛠️ Struktur Projek Modular

```text
SmartCooling/
├── CMakeLists.txt              # Konfigurasi ESP-IDF
├── platformio.ini              # Konfigurasi PlatformIO
├── sdkconfig.defaults          # Tetapan ESP-IDF
├── main/                       # Komponen utama firmware
│   ├── CMakeLists.txt
│   └── main.cpp                # Entry point
├── components/smartcooling/    # Komponen SI teras
│   ├── CMakeLists.txt
│   ├── library.json
│   ├── include/                # Header files
│   │   ├── si_core.h           # Enjin SI
│   │   ├── si_sentinel.h       # Lapisan keselamatan
│   │   ├── data_logger.h       # Sistem logging
│   │   ├── factory_config.h    # Konfigurasi kilang
│   │   └── web_assets.h        # Aset web terkompres
│   └── src/                    # Sumber implementasi
│       ├── si_core.cpp
│       └── si_sentinel.cpp
├── docs/                       # Dokumentasi
│   ├── MANUAL_PENGGUNAAN.md
│   └── PANDUAN_PEMBANGUN.md
└── tools/                      # Skrip pembangunan
```

---

## 📈 Status Pembangunan

| Fasa | Komponen | Status | Catatan |
| :--- | :--- | :---: | :--- |
| **Fasa 1** | Infrastruktur Data Logger | ✅ Selesai | Triple-buffering, Auto-rotate |
| **Fasa 2** | Model SI & Adaptif | ✅ Selesai | Pralatih 2.5J simulasi |
| **Fasa 3** | Web UI Rombakan | ✅ Selesai | Minimalis, RGB, Logout |
| **Fasa 4** | Identiti & Keselamatan | ✅ Selesai | No. Siri Automatik, PIN |
| **Fasa 5** | SI Sentinel | ✅ Selesai | Anti-hallucination, Cyber-defense |
| **Fasa 6** | Dual-Framework Support | ✅ Selesai | PlatformIO + ESP-IDF |

---

## 🔧 Konfigurasi Asas

| Perkara | Nilai Default |
| :--- | :--- |
| **Board** | Super Mini ESP32-S3 HW-747 |
| **Wi-Fi AP** | `EWP-SYSTEM-PRO` (Terbuka) |
| **IP AP** | `10.74.7.1/24` |
| **Akses Web** | `http://smartcooling.local/` |
| **Password Web** | `12345678` (Tukar selepas login pertama) |
| **Recovery PIN** | `747747` |

---

## 📄 Dokumentasi Lanjut

- [📘 Manual Penggunaan](docs/MANUAL_PENGGUNAAN.md) - Panduan lengkap Web UI, konfigurasi AP, dan prosedur OTA.
- [🛠️ Panduan Pembangun](docs/PANDUAN_PEMBANGUN.md) - Spesifikasi pin, protokol komunikasi, dan integrasi modul.

---

## ⚠️ Amaran Keselamatan

Sistem ini mengandungi ciri kawalan kuasa tinggi. Pastikan:
1.  Konfigurasi kilang disemak sebelum membiarkan sistem beroperasi tanpa pengawasan.
2.  Sensor dikalibrasi dengan betul untuk mengelakkan bacaan palsu.
3.  Pin Jumper Factory Reset tidak tertekan secara tidak sengaja.

**Dibina dengan ketahanan ekstrem sebagai keutamaan.**

© 2024 SmartCooling Project. All Rights Reserved.

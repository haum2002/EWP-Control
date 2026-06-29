# SmartCooling V2

SmartCooling V2 ialah firmware dan WebApp untuk pengawal EWP, kipas radiator,
diagnostik suhu, keselamatan asas, dan tetapan operasi berasaskan ESP32-S3
Super Mini HW-747 dengan ciri kecerdasan buatan (SI), ketahanan kuasa RTC,
dan nombor siri automatik.

> Amaran keselamatan: uji sistem di atas meja sebelum dipasang pada kenderaan.
> ESP32 tidak boleh menerima 12 V terus pada pin atau bekalan. Gunakan fius,
> common ground, buck converter yang stabil, driver beban yang sesuai, dan
> perlindungan bebas untuk keadaan suhu melampau.

## Akses Rasmi

| Perkara | Nilai |
| --- | --- |
| Repository | `https://github.com/haum2002/SmartCoolingv2.git` |
| Board | ESP32-S3 Super Mini HW-747 |
| Environment PlatformIO | `super_mini_esp32_s3_hw-747` |
| SSID AP | `EWP-SYSTEM-PRO` |
| Kata laluan Wi-Fi | Tiada, AP terbuka |
| Domain WebApp | `http://smartcooling.local/` |
| IP AP | `10.74.7.1` |
| Kata laluan WebApp lalai | `12345678` |
| PIN pemulihan lalai | `747747` |
| Nombor Siri Format | `VVMMYYK####` (contoh: `010629K1234`) |

Akses melalui IP ditolak oleh firmware. Gunakan domain rasmi
`http://smartcooling.local/`.

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
- **Reka Bentuk**: Minimalis, profesional, estetik, "liquidity" (licin)
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

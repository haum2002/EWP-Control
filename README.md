# SmartCooling V2

SmartCooling V2 ialah firmware dan WebApp untuk pengawal EWP, kipas radiator,
diagnostik suhu, keselamatan asas, dan tetapan operasi berasaskan ESP32-S3
Super Mini HW-747.

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

Akses melalui IP ditolak oleh firmware. Gunakan domain rasmi
`http://smartcooling.local/`.

## Struktur Projek

| Laluan | Fungsi |
| --- | --- |
| `web/index.html` | UI WebApp V3, termasuk bahasa `ms` dan `en` |
| `src/main.cpp` | Firmware utama, AP, keselamatan, endpoint, kawalan EWP/kipas |
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

- `docs/MANUAL_PENGGUNAAN.md`
- `docs/PANDUAN_PEMBANGUN.md`

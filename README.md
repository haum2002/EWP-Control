# SmartCooling V2

Firmware dan WebApp untuk kawalan penyejukan SmartCooling pada board
Super Mini ESP32-S3 HW-747.

Repositori ini disusun untuk build PlatformIO, audit release, dan flashing
terus ke board pembangunan. Semua konfigurasi penting berada dalam fail
yang jelas supaya perubahan boleh disemak sebelum firmware dimuat naik.

## Ringkasan

| Perkara | Nilai release |
| --- | --- |
| Board | Super Mini ESP32-S3 HW-747 |
| Environment PlatformIO | `super_mini_esp32_s3_hw-747` |
| Wi-Fi AP | `EWP-SYSTEM-PRO` |
| AP password | Tiada, AP terbuka |
| Akses WebApp rasmi | `http://smartcooling.local/` |
| IP AP | `10.74.7.1/24` untuk rangkaian dalaman sahaja |
| Web password awal | `12345678` |
| Recovery PIN awal | `747747` |

## Struktur Projek

```text
config/routes.json           Laluan endpoint rawak release
docs/MANUAL_PENGGUNAAN.md    Manual pengguna
docs/PANDUAN_PEMBANGUN.md    Nota teknikal dan pemetaan endpoint
include/factory_config.h     Konfigurasi kilang HW-747
include/routes.h             Header laluan endpoint
include/web_assets.h         WebApp terbenam yang dijana
src/main.cpp                 Firmware utama
tools/verify_release.py      Gate statik release
tools/verify_control_math.py Ujian simulasi math/kawalan
web/index.html               Sumber UI WebApp
```

## Konfigurasi Release HW-747

Release V2 menggunakan konfigurasi konservatif:

- Pam: SSR pada GPIO 2.
- Kipas: PWM pada GPIO 4.
- NTC: GPIO 1.
- ECU input: GPIO 6.
- Sensor persekitaran: belum aktif.
- Micro SD: belum aktif.
- AP terbuka tanpa kata laluan, mati selepas 5 minit hanya jika tiada pelanggan.

Perubahan pada pin, sensor, SD, SSID, recovery PIN, atau polisi AP perlu
melalui build dan ujian hardware sebenar sebelum diflash.

## Build

```bash
pio run -e super_mini_esp32_s3_hw-747
```

## Semakan Release

```bash
python tools/verify_release.py
python tools/verify_control_math.py
```

`verify_release.py` menyemak struktur, endpoint rawak, i18n `ms/en`, storage
browser, naming V2, dan konfigurasi kilang. `verify_control_math.py` menjalankan
simulasi kawalan untuk penapisan sensor, spike rejection, fail-safe sensor,
fail-safe suhu kritikal, dan larian panjang.

## Erase Dan Flash

Gantikan `COM8` jika board muncul pada port lain.

```bash
pio run -e super_mini_esp32_s3_hw-747 -t erase --upload-port COM8
pio run -e super_mini_esp32_s3_hw-747 -t upload --upload-port COM8
```

Selepas board hidup:

1. Sambung ke Wi-Fi `EWP-SYSTEM-PRO`.
2. Buka `http://smartcooling.local/`.
3. Log masuk dengan password awal `12345678`.
4. Tukar password selepas login.

Akses melalui IP tidak dianggap laluan rasmi WebApp dan firmware release
direka untuk menerima akses melalui domain/mDNS.

## Ketahanan Reset Dan Brownout

Firmware menyimpan diagnostik ringkas ke RTC slow memory secara berkala untuk
warm reset dan beberapa keadaan brownout. Data ini membantu firmware melaporkan
status boot, sebab reset, suhu terakhir, ramalan suhu, fault, dan boot counter.

RTC memory bukan pengganti perlindungan elektrik fizikal. Untuk aplikasi
keselamatan sebenar, sistem masih memerlukan bekalan kuasa, fius, wiring,
driver pam/kipas, sensor, dan perlindungan hardware yang disahkan.

## Dokumentasi

- [Manual Penggunaan](docs/MANUAL_PENGGUNAAN.md)
- [Panduan Pembangun](docs/PANDUAN_PEMBANGUN.md)

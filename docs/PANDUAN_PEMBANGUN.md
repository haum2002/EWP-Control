# Panduan Pembangun SmartCooling V3

Dokumen ini ialah rujukan rasmi untuk pembangun. Peta endpoint release disimpan
di sini supaya UI awam dan firmware tidak menggunakan nama endpoint yang jelas.

## Struktur Projek

| Laluan | Peranan |
| --- | --- |
| `web/index.html` | Sumber UI WebApp |
| `src/main.cpp` | Firmware utama |
| `config/routes.json` | Sumber tunggal laluan endpoint release |
| `include/web_assets.h` | Jana automatik, jangan edit manual |
| `include/routes.h` | Jana automatik, jangan edit manual |
| `include/wifi_provisioning/` | Shim kecil untuk build Arduino WiFi jika cache SDK PlatformIO rosak |
| `tools/pio_embed_assets.py` | Menyuntik route JSON ke UI dan menjana header |
| `docs/MANUAL_PENGGUNAAN.md` | Manual pengguna |
| `docs/PANDUAN_PEMBANGUN.md` | Rujukan pembangun |
| `docs/archive/` | Fail lama untuk rujukan sejarah |

## Peta Endpoint Release

Peta ini dijana daripada `config/routes.json`.

| Nama dalaman | Laluan release |
| --- | --- |
| `login` | `/A7xQm2` |
| `recover` | `/g9LzT4` |
| `password` | `/Pq3Rz8` |
| `device` | `/m2VhK9` |
| `status` | `/Z8nLp3` |
| `config` | `/u4CwT7` |
| `preview` | `/R6sJq1` |
| `save` | `/d5YpM8` |
| `rgb` | `/H3vXk6` |
| `events` | `/t9BaN2` |
| `ota` | `/K4qWs7` |
| `ws` | `/x2FzQ9` |
| `reboot` | `/N8jDv5` |

Jangan tambah laluan jelas atau semantik pada release. Jika perlu debug semasa
pembangunan, gunakan build lokal yang berasingan dan pastikan release kekal
berdasarkan `config/routes.json`.

## Cara Route Dijana

`tools/pio_embed_assets.py` berjalan semasa PlatformIO build.

Prosesnya:

1. Baca `web/index.html`.
2. Baca `config/routes.json`.
3. Sahkan semua route wajib wujud.
4. Sahkan setiap route berbentuk rawak 6 aksara selepas `/`.
5. Ganti token route `__SMARTCOOLING_ROUTES__` dalam UI.
6. Jana `include/web_assets.h`.
7. Jana `include/routes.h`.

UI dan firmware mesti merujuk route daripada sumber yang sama.

## Rangkaian

| Tetapan | Nilai |
| --- | --- |
| SSID | `EWP-SYSTEM-PRO` |
| Kata laluan AP | Tiada |
| IP AP | `10.74.7.1` |
| Subnet | `255.255.255.0` |
| DHCP mula | `10.74.7.20` |
| DHCP maksimum sasaran | `10.74.7.80` |
| Domain rasmi | `smartcooling.local` |
| mDNS host | `smartcooling` |

Firmware tidak menyediakan captive portal dan tidak menjawab DNS wildcard. DNS
hanya digunakan untuk nama rasmi. Ini membantu telefon mengekalkan data mudah
alih untuk internet apabila Wi-Fi AP tidak mempunyai internet.

## Polisi Akses Domain

Firmware menyemak header `Host`.

Dibenarkan:

- `smartcooling.local`
- `smartcooling`

Permintaan WebApp melalui IP ditolak dengan status HTTP `421`. Tujuannya adalah
memastikan pengguna sentiasa menggunakan akses rasmi.

## AP Timeout

AP bermula selepas boot.

- Jika `WiFi.softAPgetStationNum()` melebihi 0, timer idle diset semula.
- Jika tiada pelanggan selama 300000 ms, DNS/mDNS/AP dimatikan.
- AP tidak dimatikan berdasarkan aktiviti WebApp.
- AP hidup semula melalui restart, power-cycle, atau ACC-ON kembali.

## Keselamatan Login

Kata laluan WebApp disimpan dalam Preferences namespace `sc` pada key
`webpass`. Nilai lalai ialah `12345678`.

Polisi login:

- Tetingkap kiraan gagal: 5 minit.
- 5 cubaan gagal: kunci 60 saat.
- Kegagalan berulang menggandakan tempoh kunci.
- Tempoh maksimum: 15 minit.
- Mesej ralat kekal umum.

Token sesi dijana selepas login berjaya dan dihantar semula oleh UI melalui
header `Authorization: Bearer <token>`.

## Pemulihan Kata Laluan

PIN pemulihan lalai ialah `747747`.

Polisi pemulihan:

- Nombor siri tidak digunakan.
- 3 cubaan gagal mengunci pemulihan selama 10 minit.
- Kata laluan baharu minimum 6 aksara.
- Mesej ralat kekal umum.

## Endpoint Ringkas

Semua endpoint release menggunakan nama rawak dalam jadual di atas.

| Fungsi | Kaedah |
| --- | --- |
| UI utama | `GET /` |
| Identiti peranti | `GET device` |
| Login | `POST login` |
| Pemulihan | `POST recover` |
| Tukar kata laluan | `POST password` |
| Status runtime | `GET status` |
| Baca konfigurasi | `GET config` |
| Pratonton konfigurasi | `POST preview` |
| Simpan konfigurasi | `POST save` |
| RGB no-op | `POST rgb` |
| Log peristiwa | `GET events` |
| OTA | `POST ota` |
| WebSocket | `GET ws` |
| Reboot | `POST reboot` |

Nota: perkataan fungsi di atas ialah nama dalaman dokumentasi, bukan laluan HTTP
release.

## Status JSON

Status runtime mengandungi medan seperti:

- `coolant_c`
- `coolant_valid`
- `pump_pct`
- `fan_pct`
- `ssr_active`
- `ecu_request`
- `mode`
- `faults`
- `heap_free`
- `heap_min_free`
- `uptime_ms`
- `wifi_ap_running`
- `wifi_clients`
- `ap_auto_off_s`
- `outputs_forced`
- `safety_state`
- `reset_reason`
- `boot_count`
- `config_recovered`

Tambahan SmartCooling SI:

- `si_coolant_rate_c_s`: kadar perubahan suhu dalam darjah Celsius sesaat.
- `si_pred_temp_60_c`: ramalan suhu 60 saat berdasarkan trend tapis.
- `si_risk_score`: skor risiko 0-100 berdasarkan suhu semasa dan suhu ramalan.
- `sensor_quality_pct`: kualiti diagnostik bacaan 0-100 selepas mengambil kira noise ADC dan residual penapis.
- `sensor_degraded`: benar apabila bacaan sedang tidak stabil atau spike berulang berlaku.
- `si_noise_c`: anggaran noise suhu dalam darjah Celsius.
- `si_adc_raw` dan `si_adc_spread`: purata ADC tapis dan julat sampel mentah.

Firmware membaca NTC menggunakan 17 sampel ADC, membuang sampel ekstrem, kemudian
menggunakan penapis alpha-beta berunit C/s. Spike suhu tinggi diterima lebih
cepat untuk keselamatan, manakala spike rendah yang tidak munasabah ditolak dan
boleh menjadi fault sensor jika berulang.

`si_confidence` masih dihantar untuk keserasian UI lama, tetapi release baharu
menggunakan `sensor_quality_pct`. Jangan tafsir mana-mana medan ini sebagai
jaminan keselamatan sistem.

## Kalibrasi Sensor

Konversi NTC menggunakan model Beta yang boleh dikonfigurasi:

- `ntc_series_ohm`
- `ntc_nominal_ohm`
- `ntc_beta_k`
- `ntc_nominal_c`
- `temp_offset_c`
- `temp_gain`

Nilai default disediakan untuk NTC 10k B3950. `temp_offset_c` digunakan untuk
membetulkan ralat tetap, manakala `temp_gain` digunakan hanya apabila bacaan
menunjukkan ralat cerun selepas dibandingkan dengan termometer rujukan.

UI perlu menerima medan tambahan tanpa gagal supaya firmware boleh berkembang
tanpa memecahkan paparan.

## Konfigurasi JSON

Konfigurasi utama:

- `mode`
- `force_pump`
- `force_fan`
- `auto_target_c`
- `auto_full_c`
- `min_temp_c`
- `window_s`
- `slew_pct_s`
- `warning_temp_c`
- `critical_temp_c`
- `battery_cutoff_v`
- `run_on_s`
- `run_on_stop_c`
- `low_voltage_delay_ms`
- `ntc_series_ohm`
- `ntc_nominal_ohm`
- `ntc_beta_k`
- `ntc_nominal_c`
- `temp_offset_c`
- `temp_gain`
- `manual_steps`

Nilai disanitasi di firmware sebelum disimpan. `preview` tidak menyimpan nilai,
manakala `save` menulis ke Preferences.

Config disimpan dalam dua slot Preferences, `cfg_a` dan `cfg_b`, bersama nombor
urutan dan CRC32 payload. Ketika boot, firmware memilih slot sah dengan nombor
urutan tertinggi. Jika kedua-dua slot rosak, firmware kembali kepada default
selamat, menetapkan fault `FAULT_CONFIG_RECOVERED`, dan melaporkan
`config_recovered=true`.

## Watchdog Dan Reset

Firmware mendaftarkan loop utama kepada task watchdog 4 saat. Jika loop utama
tersekat melebihi had ini, board akan reset dan sebab reset boleh dilihat melalui
`reset_reason`.

Nilai reset penting:

- `power_on`: board baru menerima kuasa.
- `brownout`: voltan jatuh sehingga ESP32 reset.
- `task_watchdog`: loop utama tersekat.
- `panic`: exception runtime.

Untuk keselamatan operasi panjang, logger luaran atau telemetri berkala masih
disarankan kerana reset akibat bekalan hilang hanya boleh diketahui selepas
kuasa kembali.

## Had Firmware

Firmware boleh memilih output selamat apabila sensor gagal, suhu kritikal,
config rosak, atau loop tersekat. Firmware tidak boleh menjamin aliran coolant
apabila bekalan fizikal ke pam, kipas, relay, driver, atau MCU hilang.

Pemasangan yang melibatkan risiko kerosakan enjin perlu ada kawalan perkakasan
bebas seperti fius bersaiz betul, relay normally-on jika sesuai dengan reka
bentuk, thermal switch bebas, bekalan sokongan untuk run-on, dan ujian haba
jangka panjang di bawah beban sebenar.

## OTA

OTA menggunakan route release rawak dan memerlukan token sesi. UI menghantar
fail firmware sebagai multipart upload. Selepas OTA berjaya, firmware reboot.

Pastikan partition table menyokong OTA:

```csv
nvs,      data, nvs,     0x9000,   0x5000
otadata,  data, ota,     0xe000,   0x2000
app0,     app,  ota_0,   0x10000,  0x1F0000
app1,     app,  ota_1,   0x200000, 0x1F0000
```

## Build

```powershell
pio run -e super_mini_esp32_s3_hw-747
```

Build wajib menjana semula:

- `include/web_assets.h`
- `include/routes.h`

## Nota Shim Wi-Fi Provisioning

Projek ini tidak menggunakan Wi-Fi provisioning. Pada mesin pembangunan ini,
folder cache PlatformIO untuk header provisioning pernah dilaporkan oleh Windows
sebagai rosak dan tidak boleh dibaca. Arduino WiFi core tetap memasukkan header
tersebut walaupun provisioning tidak digunakan.

Untuk memastikan build stabil, projek menyediakan shim minimum di
`include/wifi_provisioning/`. `platformio.ini` menambah `-Iinclude` supaya shim
ini digunakan juga semasa library Arduino WiFi dikompilasi. Shim ini hanya
mengandungi simbol yang diperlukan oleh Arduino WiFi core dan tidak mengaktifkan
fungsi provisioning.

## Semakan Pantas

Gunakan semakan ini sebelum release:

```powershell
rg -n "[A]erospace|[A]utomotive HMI|SSID: [S]martCooling|[r]ecSerial" web src config docs README.md --glob "!docs/archive/**"
rg -n "ROUTE_" include/routes.h src/main.cpp
python tools/verify_release.py
python tools/verify_control_math.py
pio run -e super_mini_esp32_s3_hw-747
```

Semak juga WebApp pada desktop dan telefon untuk:

- Login.
- Pemulihan kata laluan.
- Tema gelap dan cerah.
- Bahasa `ms` dan `en`.
- Dashboard responsif.
- Tetapan dan diagnostik.

## Nota Operasi

Semasa membangunkan di komputer, jangan putuskan sambungan internet mesin kerja
untuk menguji AP. Ujian AP sebenar hendaklah dibuat secara sengaja pada peranti
uji, bukan dengan menukar sambungan Wi-Fi komputer kerja secara automatik.

# Manual Penggunaan SmartCooling V2

Dokumen ini menerangkan cara menggunakan WebApp dan firmware SmartCooling V2
untuk ESP32-S3 Super Mini HW-747.

## Maklumat Asas

| Perkara | Nilai |
| --- | --- |
| Nama Wi-Fi | `EWP-SYSTEM-PRO` |
| Kata laluan Wi-Fi | Tiada, AP terbuka |
| Alamat WebApp | `http://smartcooling.local/` |
| IP AP | `10.74.7.1` |
| Bahasa UI | Melayu (`ms`) dan English (`en`) |
| Kata laluan WebApp lalai | `12345678` |
| PIN pemulihan lalai | `747747` |
| Profil kilang firmware | `super_mini_esp32_s3_hw747` |

Gunakan domain `http://smartcooling.local/` sahaja. Firmware menolak akses WebApp
melalui IP supaya tabiat penggunaan kekal konsisten dan lebih mudah disokong.

Profil kilang V2 mengekalkan pin HW-747 yang telah diuji, AP terbuka tanpa kata
laluan, dan PIN pemulihan `747747`. Jika firmware dibina semula oleh pembangun,
pastikan profil ini tidak diubah tanpa semakan hardware.

## Komponen Utama

SmartCooling terdiri daripada:

- ESP32-S3 Super Mini HW-747.
- Firmware kawalan EWP, kipas, SSR, input ECU, keselamatan, AP, mDNS, dan OTA.
- WebApp responsif untuk desktop dan telefon.
- Sensor NTC 10 kohm B3950 untuk bacaan suhu.
- Output SSR untuk pam EWP.
- Output PWM untuk kipas radiator.
- Input ECU 3.3 V yang selamat melalui level shifter atau divider.
- Dokumen pengguna dan dokumen pembangun.

## Konfigurasi Kilang (Untuk Pembangun)

Sebelum flash firmware, pembangun boleh memilih konfigurasi output berikut dalam
`include/factory_config.h` selepas semakan hardware:

### 1. Jenis Pam
- **SSR (0)**: On/Off lembut menggunakan relay/SSR
- **PWM (1)**: Kawalan kelajuan menggunakan isyarat PWM

### 2. Jenis Kipas
- **SSR (0)**: On/Off menggunakan relay/SSR
- **PWM (1)**: Kawalan kelajuan menggunakan isyarat PWM

Sensor persekitaran dan Micro SD masih disediakan sebagai ruang reserved untuk
varian akan datang. Release V2 akan menolak build jika pilihan itu diaktifkan
sebelum driver dan ujian rasmi tersedia.

## Sambungan Wi-Fi

1. Hidupkan board.
2. Tunggu Wi-Fi `EWP-SYSTEM-PRO` muncul.
3. Sambung telefon atau komputer kepada Wi-Fi tersebut.
4. Buka pelayar dan pergi ke `http://smartcooling.local/`.

AP tidak menggunakan kata laluan. Keselamatan akses WebApp dibuat pada lapisan
login, lockout, laluan endpoint rawak, dan penggunaan domain rasmi.

## Internet Telefon Semasa Sambung AP

AP SmartCooling direka sebagai rangkaian tempatan sahaja:

- Tiada captive portal.
- Tiada DNS wildcard.
- DNS hanya menjawab nama rasmi `smartcooling.local`.
- Telefon yang menyokong data mudah alih bersama Wi-Fi tanpa internet biasanya
  masih boleh menggunakan 4G/5G untuk internet.

Jika telefon masih mematikan data mudah alih ketika tersambung kepada Wi-Fi:

- Android: aktifkan pilihan seperti `Mobile data always active`, `Switch to
  mobile data`, atau `Use mobile data when Wi-Fi has no internet`.
- iPhone: pastikan `Wi-Fi Assist` aktif dan kekalkan data mudah alih.
- Jika sistem telefon bertanya sama ada mahu kekal pada Wi-Fi tanpa internet,
  pilih untuk kekal tersambung.

Tingkah laku ini bergantung kepada OS telefon dan polisi pengeluar.

## AP Timeout

AP hidup selepas boot, restart, power-cycle, atau ACC-ON kembali.

- Jika tiada pelanggan tersambung selama 5 minit, AP dimatikan.
- Jika sekurang-kurangnya satu pelanggan tersambung, AP kekal hidup walaupun
  tiada aktiviti WebApp.
- Selepas AP dimatikan, hidupkan semula board untuk mengaktifkan AP kembali.

## Login

1. Buka `http://smartcooling.local/`.
2. Pilih bahasa `MS` atau `EN`.
3. Pilih tema gelap atau cerah jika perlu.
4. Masukkan kata laluan WebApp.
5. Tekan `Log Masuk`.

Nombor siri board dipaparkan kecil pada halaman login sebagai identiti board.
Nombor siri juga dipaparkan di halaman Tetapan/Identiti.

## Perlindungan Login

Firmware melindungi login daripada cubaan berulang:

- 5 cubaan gagal dalam 5 minit akan mengunci login selama 60 saat.
- Jika kegagalan berulang, tempoh kunci meningkat sehingga maksimum 15 minit.
- Mesej ralat dibuat umum supaya sebab sebenar tidak bocor.

Tunggu sehingga tempoh kunci tamat sebelum mencuba semula.

## Pemulihan Kata Laluan

Panel pemulihan hanya meminta:

- PIN pemulihan.
- Kata laluan baharu.
- Butang `Tetapkan Semula`.
- Butang `Batal`.

PIN lalai ialah `747747`. Nombor siri tidak diperlukan untuk pemulihan.

Perlindungan pemulihan:

- 3 cubaan PIN gagal akan mengunci pemulihan selama 10 minit.
- Gunakan kata laluan baharu sekurang-kurangnya 6 aksara.

Selepas berjaya, kembali ke halaman login dan gunakan kata laluan baharu.

## Dashboard

Dashboard memaparkan keadaan operasi semasa:

- Suhu coolant.
- Output pam.
- Output kipas.
- Keadaan SSR.
- Input ECU.
- Fault keselamatan.
- Keadaan keselamatan output.
- Sebab reset terakhir seperti `power_on`, `brownout`, atau `task_watchdog`.
- Status AP, pelanggan, heap, uptime, dan status config.

Data dikemas kini melalui WebSocket apabila tersedia dan melalui polling jika
WebSocket terputus.

## Kawalan

Halaman Kawalan membenarkan tetapan:

- Mod `auto`, `manual`, `force`, `bleed`, atau `flush`.
- Peratus manual pam.
- Peratus manual kipas.
- Sasaran suhu.
- Had keselamatan suhu tinggi.
- Window SSR.

Gunakan `Pratonton` untuk menyemak tetapan sebelum simpan. Gunakan `Simpan`
untuk menghantar tetapan ke firmware.

## Mod Operasi

| Mod | Kegunaan |
| --- | --- |
| `auto` | Firmware mengawal pam dan kipas berdasarkan suhu dan input ECU |
| `manual` | Pengguna menetapkan peratus pam dan kipas secara manual |
| `force` | Pam dan kipas dipaksa 100% |
| `bleed` | Pam berjalan untuk membantu proses buang angin |
| `flush` | Pam dan kipas berjalan tinggi untuk proses flushing |

Jika suhu melepasi had keselamatan, firmware memaksa output keselamatan
walaupun mod lain sedang dipilih.

## Manual Dalam WebApp

Tab Manual dalam WebApp menerangkan langkah ringkas:

1. Semak pendawaian dan fius.
2. Hidupkan board.
3. Sambung ke Wi-Fi `EWP-SYSTEM-PRO`.
4. Buka `http://smartcooling.local/`.
5. Log masuk.
6. Semak dashboard.
7. Laraskan tetapan.
8. Gunakan pratonton sebelum simpan.
9. Jalankan ujian tanpa beban berat.
10. Pantau suhu dan output.
11. Uji input ECU.
12. Uji mod manual.
13. Uji mod force secara ringkas.
14. Uji keadaan suhu tinggi secara terkawal.
15. Semak log peristiwa.
16. Tukar kata laluan selepas pemasangan.
17. Simpan konfigurasi akhir.
18. Catat nombor siri board.

## Keselamatan

Perkara penting:

- Jangan sambungkan 12 V terus ke ESP32.
- Semua input ke GPIO mestilah maksimum 3.3 V.
- Gunakan common ground antara ESP32 dan litar kawalan.
- Jangan pacu pam atau kipas terus dari GPIO.
- Gunakan SSR, MOSFET, diode, fius, dan wayar mengikut arus sebenar.
- Uji sistem di atas meja sebelum dipasang pada kenderaan.

Had keselamatan yang perlu difahami:

- Firmware boleh mengesan sensor rosak, config rosak, reset watchdog, dan reset
  brownout selepas board hidup semula.
- Firmware tidak boleh menjalankan pam atau kipas jika bekalan kuasa fizikal
  kepada board, relay, pam, atau kipas hilang.
- Untuk sistem penyejukan yang mesti terus beroperasi selepas ACC dimatikan atau
  selepas bekalan utama terputus, gunakan reka bentuk perkakasan berasingan
  seperti bekalan sokongan, litar run-on bebas, fius yang betul, dan suis suhu
  bebas yang boleh menghidupkan kipas tanpa bergantung kepada MCU.
- Jika kegagalan MCU tidak boleh diterima, sediakan laluan fail-safe perkakasan:
  relay normally-on, thermal switch bebas, atau pengawal sandaran yang diuji.
- Jangan anggap nombor kualiti sensor sebagai jaminan keselamatan. Ia ialah
  petunjuk diagnostik untuk noise ADC, spike, dan kestabilan bacaan.

## Kalibrasi Sensor

Tetapan kalibrasi berada dalam halaman Keselamatan dan had sistem.

Gunakan nilai fizikal sebenar komponen:

- `Resistor siri NTC ohm`: nilai resistor tetap pada voltage divider.
- `NTC nominal ohm`: rintangan NTC pada suhu nominal.
- `Beta NTC K`: nilai beta NTC.
- `Suhu nominal NTC C`: suhu rujukan NTC.
- `Offset suhu C`: pelarasan kecil selepas banding dengan termometer rujukan.
- `Gain suhu`: pelarasan cerun bacaan jika ralat berubah mengikut suhu.

Selepas kalibrasi, semak bacaan pada sekurang-kurangnya dua suhu stabil sebelum
menyambungkan beban sebenar. Jika bacaan melompat atau kualiti sensor rendah,
semak pendawaian NTC, common ground, nilai resistor, dan noise bekalan.

## Tetapan Dan Identiti

Halaman Tetapan memaparkan:

- Identiti board.
- Nombor siri.
- Domain rasmi.
- SSID AP.
- Status AP.
- Pelanggan AP.
- Butang tukar kata laluan WebApp.
- Muat naik firmware OTA.

Gunakan OTA hanya jika bekalan kuasa stabil. Jangan matikan board ketika proses
OTA sedang berjalan.

## Diagnostik Dan Log

Diagnostik membantu menyemak:

- Heap bebas.
- Uptime.
- Sebab reset terakhir.
- Bilangan boot (NVS dan RTC).
- Status AP.
- Bilangan pelanggan.
- Status lockout.
- Fault sensor atau suhu.
- Kualiti sensor, noise suhu, ramalan suhu 60 saat, dan skor risiko.
- Sama ada output sedang dipaksa ke keadaan selamat.
- Sama ada config pernah dipulihkan daripada storan terlindung.
- Status RTC Memory (valid/invalid, recovery count).
- Nombor siri automatik sistem.

Log peristiwa merekod peristiwa seperti boot, login, pemulihan, simpan tetapan,
OTA, AP dimatikan, fault keselamatan, dan recovery RTC.

SmartCooling SI menapis bacaan NTC dengan beberapa sampel ADC dan mengabaikan
spike yang tidak munasabah. Jika bacaan sensor tiba-tiba jatuh secara palsu dan
berulang, firmware akan menganggapnya sebagai fault dan memaksa output selamat.

### Diagnostik RTC Memory

Firmware menyimpan diagnostik ringkas ke RTC Memory secara berkala:

- **Auto-save**: Setiap 5 saat.
- **Checksum**: CRC16 untuk mengesan data rosak.
- **Boot counter**: Membantu mengenal pasti reset berulang.
- **Recovery diagnostik**: Mode terakhir, suhu terakhir, ramalan suhu, fault,
  safety state, dan sequence boleh dipulihkan jika data RTC masih sah.

Jika board reset atau mengalami brownout, sistem akan:
1. Boot semula dan semak magic word, versi struktur, dan checksum RTC.
2. Jika valid, pulihkan diagnostik terakhir.
3. Jika invalid, gunakan keadaan selamat dan catat fault.
4. Paparkan status melalui diagnostik WebApp.

Nota penting: RTC Memory bukan perlindungan penuh untuk kehilangan kuasa sebenar.
Sistem penyejukan yang kritikal masih memerlukan bekalan kuasa stabil, fius,
driver pam/kipas yang sesuai, sensor yang disahkan, dan wiring yang kemas.

### Nombor Siri Automatik

Setiap board mempunyai nombor siri unik format `VVMMYYK####`:

- **VV**: Versi firmware (2 digit)
- **MM**: Bulan pembuatan (2 digit)
- **YY**: Tahun pembuatan (2 digit)
- **K**: Kod spec (1 huruf)
- **####**: Nombor urut unit (4 digit)

Rujukan format: `010629K1234` = Versi 01, Jun 2026, Kod K, Unit 1234

Nombor siri dijana automatik berdasarkan:
- Tarikh compilation firmware
- MAC address unik ESP32
- Konfigurasi kilang

Nombor siri digunakan untuk:
- Pengesahan OTA (hanya firmware serasi boleh diupload)
- Identiti sistem dalam WebApp
- Log diagnostik dan tracking
- Validasi konfigurasi

## Build Dan Flash

Keperluan:

- Visual Studio Code.
- PlatformIO.
- Kabel USB data.
- Board ESP32-S3 Super Mini HW-747.

Build:

```powershell
pio run -e super_mini_esp32_s3_hw-747
```

Upload:

```powershell
pio run -e super_mini_esp32_s3_hw-747 -t upload
```

Serial Monitor:

```powershell
pio device monitor --port COM8 --baud 115200
```

Jika upload gagal:

1. Tekan dan tahan `BOOT`.
2. Tekan dan lepaskan `RESET`.
3. Lepaskan `BOOT`.
4. Jalankan upload sekali lagi.

## Penyelesaian Masalah

| Masalah | Tindakan |
| --- | --- |
| WebApp kosong | Pastikan URL ialah `http://smartcooling.local/`, bukan IP |
| Domain tidak dibuka | Putus dan sambung semula Wi-Fi AP, kemudian cuba semula |
| AP hilang | AP mungkin mati kerana tiada pelanggan 5 minit; restart board |
| Login dikunci | Tunggu tempoh lockout tamat |
| Pemulihan dikunci | Tunggu 10 minit selepas 3 cubaan gagal |
| Internet telefon hilang | Aktifkan tetapan telefon untuk guna data mudah alih ketika Wi-Fi tiada internet |
| OTA gagal | Pastikan fail firmware betul dan bekalan kuasa stabil |

## Nota Bahasa

UI menyediakan dua bahasa rasmi:

- `MS` untuk Bahasa Melayu.
- `EN` untuk English.

Semua mesej utama, butang, ralat, toast, manual, diagnostik, tetapan, dan log
disediakan dalam kedua-dua bahasa.

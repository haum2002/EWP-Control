/*
 * SmartCooling - Factory Configuration Header
 * Versi: 3.1.0 (Spec Code System V3.0 - 12+1 Konfigurasi)
 * Sasaran: Super Mini ESP32 S3 HW-747
 *
 * PENTING: Fail ini WAJIB dikonfigurasi sebelum flashing firmware.
 * 
 * ============================================================================
 * SISTEM KOD SPEC V3.0 - 12+1 KONFIGURASI PRA-TAKRIF
 * ============================================================================
 * Cukup UBAH SATU HURUF sahaja untuk konfigurasi automatik penuh!
 * 
 * #define SC_FACTORY_SPEC_CODE 'X'  // Ganti 'X' dengan kod pilihan
 * 
 * KATEGORI 1: KAWALAN KELAJUAN PENUH (PWM)
 * ---------------------------------------------------------
 * 'P' - Standard PWM         : Pam PWM + Kipas PWM (Aplikasi umum)
 * 'H' - High Performance     : Pam PWM Hi-Res + Kipas PWM Hi-Res (Ketepatan tinggi)
 * 'L' - Low Power            : Pam PWM Low Freq + Kipas PWM Low Freq (Jimat tenaga)
 * 
 * KATEGORI 2: SUIS DIGITAL (SSR/RELAY)
 * ---------------------------------------------------------
 * 'S' - Standard SSR         : Pam SSR + Kipas SSR (Beban rintangan tinggi)
 * 'Q' - Quick Cycle          : Pam SSR Fast + Kipas SSR Fast (Beban termal kecil)
 * 
 * KATEGORI 3: HIBRID (CAMPURAN)
 * ---------------------------------------------------------
 * 'PS'- PWM Pump + SSR Fan   : Pam PWM + Kipas SSR (Pam berubah, kipas ON/OFF)
 * 'SP'- SSR Pump + PWM Fan   : Pam SSR + Kipas PWM (Pam tetap, kipas halus)
 * 
 * KATEGORI 4: APLIKASI KHUSUS
 * ---------------------------------------------------------
 * 'C' - Cooling Only         : Pam PWM + Kipas PWM Auto (Mod sejuk sahaja)
 * 'W' - Heating Only         : Pam PWM (Heater) + Kipas SSR Safety (Pemanas industri)
 * 'V' - Ventilation Focus    : Pam OFF + Kipas PWM (Ekstraksi udara)
 * 'Z' - Zero Output          : Pam OFF + Kipas OFF (Diagnostik/sensor sahaja)
 * 
 * KATEGORI 5: MOD KHAS
 * ---------------------------------------------------------
 * 'D' - Developer Mode       : Dinamik (Pemetaan semula pin melalui Web UI)
 * 
 * ============================================================================
 * PARAMETER TAMBAHAN (Manual)
 * ============================================================================
 * SC_FACTORY_ENV_SENSOR: 0=Tiada, 1=BME280, 2=AHT30, 3=BMP280, 4=BMP180
 * SC_FACTORY_SD_CARD: 0=Tidak Aktif, 1=Aktif
 * SYSTEM_UNIT_NUMBER: Nombor unit 4 digit (contoh: 0001, 0042, 1234)
 */

#ifndef SMARTCOOLING_FACTORY_CONFIG_H
#define SMARTCOOLING_FACTORY_CONFIG_H

#define SC_FACTORY_PROFILE "super_mini_esp32_s3_hw747"
#define SC_FACTORY_HARDWARE_REV "hw-747"

// ============================================================================
// 1. SISTEM KOD SPEC V3.0 - PILIHAN KONFIGURASI UTAMA
// ============================================================================

// ✅ UBAH HURUF INI SAHAJA UNTUK KONFIGURASI AUTOMATIK
#define SC_FACTORY_SPEC_CODE 'P'  // Pilihan: P, H, L, S, Q, PS, SP, C, W, V, Z, D

// Parameter tambahan (manual)
#define SC_FACTORY_ENV_SENSOR 0      // 0=Tiada, 1=BME280, 2=AHT30, 3=BMP280, 4=BMP180
#define SC_FACTORY_SD_CARD 0         // 0=Tidak Aktif, 1=Aktif
#define SYSTEM_UNIT_NUMBER 0001      // Nombor unit 4 digit

// ============================================================================
// 2. TERJEMAHAN KOD SPEC KE KONFIGURASI DALAMAN
// ============================================================================

// Jenis Pam & Kipas berdasarkan Kod Spec
#if SC_FACTORY_SPEC_CODE == 'P' || SC_FACTORY_SPEC_CODE == 'H' || SC_FACTORY_SPEC_CODE == 'L'
  #define SC_INTERNAL_PUMP_TYPE 2  // PWM
  #define SC_INTERNAL_FAN_TYPE 2   // PWM
#elif SC_FACTORY_SPEC_CODE == 'S' || SC_FACTORY_SPEC_CODE == 'Q'
  #define SC_INTERNAL_PUMP_TYPE 1  // SSR
  #define SC_INTERNAL_FAN_TYPE 1   // SSR
#elif SC_FACTORY_SPEC_CODE == 'PS' || SC_FACTORY_SPEC_CODE == 'C' || SC_FACTORY_SPEC_CODE == 'W'
  #define SC_INTERNAL_PUMP_TYPE 2  // PWM
  #define SC_INTERNAL_FAN_TYPE 1   // SSR
#elif SC_FACTORY_SPEC_CODE == 'SP'
  #define SC_INTERNAL_PUMP_TYPE 1  // SSR
  #define SC_INTERNAL_FAN_TYPE 2   // PWM
#elif SC_FACTORY_SPEC_CODE == 'V'
  #define SC_INTERNAL_PUMP_TYPE 0  // OFF
  #define SC_INTERNAL_FAN_TYPE 2   // PWM
#elif SC_FACTORY_SPEC_CODE == 'Z'
  #define SC_INTERNAL_PUMP_TYPE 0  // OFF
  #define SC_INTERNAL_FAN_TYPE 0   // OFF
#elif SC_FACTORY_SPEC_CODE == 'D'
  #define SC_INTERNAL_PUMP_TYPE 0  // Developer pilih sendiri
  #define SC_INTERNAL_FAN_TYPE 0   // Developer pilih sendiri
#else
  #error "Kod Spec tidak sah! Gunakan: P, H, L, S, Q, PS, SP, C, W, V, Z, atau D"
#endif

// Tetapan khas berdasarkan spec
#if SC_FACTORY_SPEC_CODE == 'H'
  #define SC_INTERNAL_PWM_FREQ 20000  // High frequency untuk prestasi
  #define SC_INTERNAL_PWM_RES 10      // Higher resolution
#elif SC_FACTORY_SPEC_CODE == 'L'
  #define SC_INTERNAL_PWM_FREQ 5000   // Low frequency untuk jimat tenaga
  #define SC_INTERNAL_PWM_RES 8
#elif SC_FACTORY_SPEC_CODE == 'Q'
  #define SC_INTERNAL_SSR_FAST_CYCLE 1  // Enable fast cycling untuk SSR
#else
  #define SC_INTERNAL_PWM_FREQ 15000  // Standard frequency
  #define SC_INTERNAL_PWM_RES 8
#endif

// ============================================================================
// 3. DEFINISI PIN DINAMIK BERDASARKAN KONFIGURASI
// ============================================================================

#define SC_FACTORY_PIN_NTC 1
#define SC_FACTORY_PIN_ECU 6

// Pin Pam - Dinamik berdasarkan jenis
#if SC_INTERNAL_PUMP_TYPE == 2  // PWM
  #define SC_FACTORY_PIN_PUMP_PWM 3
  #define SC_FACTORY_PIN_PUMP_SSR -1
#elif SC_INTERNAL_PUMP_TYPE == 1  // SSR
  #define SC_FACTORY_PIN_PUMP_PWM -1
  #define SC_FACTORY_PIN_PUMP_SSR 2
#else  // OFF atau Developer
  #define SC_FACTORY_PIN_PUMP_PWM -1
  #define SC_FACTORY_PIN_PUMP_SSR -1
#endif

// Pin Kipas - Dinamik berdasarkan jenis
#if SC_INTERNAL_FAN_TYPE == 2  // PWM
  #define SC_FACTORY_PIN_FAN_PWM 4
  #define SC_FACTORY_PIN_FAN_SSR -1
#elif SC_INTERNAL_FAN_TYPE == 1  // SSR
  #define SC_FACTORY_PIN_FAN_PWM -1
  #define SC_FACTORY_PIN_FAN_SSR 5
#else  // OFF atau Developer
  #define SC_FACTORY_PIN_FAN_PWM -1
  #define SC_FACTORY_PIN_FAN_SSR -1
#endif

// Alias untuk keserasian kod lama
#define SC_FACTORY_PIN_SSR SC_FACTORY_PIN_PUMP_SSR
#define SC_FACTORY_PIN_PWM SC_FACTORY_PIN_FAN_PWM

// Pin SD Card - Hanya jika diaktifkan
#if SC_FACTORY_SD_CARD == 1
  #define SC_FACTORY_PIN_SD_CS 7
  #define SC_FACTORY_PIN_SD_MOSI 11
  #define SC_FACTORY_PIN_SD_MISO 13
  #define SC_FACTORY_PIN_SD_SCK 12
#endif

// Pin I2C Sensor - Hanya jika sensor diaktifkan
#if SC_FACTORY_ENV_SENSOR != 0
  #define SC_FACTORY_PIN_I2C_SDA 8
  #define SC_FACTORY_PIN_I2C_SCL 9
#endif

// Pin Factory Reset Fizikal (GPIO ditekan semasa boot)
#define SC_FACTORY_RESET_PIN GPIO_NUM_0
#define SC_FACTORY_RESET_ACTIVE_LOW true

// ============================================================================
// 4. TETAPAN SISTEM ASAS DAN IDENTITI
// ============================================================================

#define SC_FACTORY_AP_SSID "EWP-SYSTEM-PRO"
#define SC_FACTORY_AP_OPEN 1
#define SC_FACTORY_DOMAIN_HOST "smartcooling.local"
#define SC_FACTORY_MDNS_HOST "smartcooling"

#define SC_FACTORY_DEFAULT_WEB_PASSWORD "12345678"
#define SC_FACTORY_RECOVERY_PIN "747747"

// Nombor Siri Automatik Format: VVMMYYK####
// Versi(2) + Bulan(2) + Tahun(2) + Kod Spec(1) + Unit(4)
#define SC_SYSTEM_VERSION_MAJOR 0
#define SC_SYSTEM_VERSION_MINOR 1
// Kod Spec diambil dari SC_FACTORY_SPEC_CODE untuk nombor siri
#define SC_SYSTEM_SPEC_CODE SC_FACTORY_SPEC_CODE

#define SC_FACTORY_TICK_MS 500UL
#define SC_FACTORY_PWM_FREQ SC_INTERNAL_PWM_FREQ
#define SC_FACTORY_PWM_RES SC_INTERNAL_PWM_RES
#define SC_FACTORY_PWM_CHAN 0
#define SC_FACTORY_AP_IDLE_OFF_MS 300000UL
#define SC_FACTORY_STATUS_PUSH_MS 1000UL
#define SC_FACTORY_ADC_SAMPLE_COUNT 17
#define SC_FACTORY_ADC_TRIM_COUNT 3
#define SC_FACTORY_SENSOR_GOOD_LIMIT 2
#define SC_FACTORY_SENSOR_BAD_LIMIT 3
#define SC_FACTORY_SENSOR_SPIKE_LIMIT 3
#define SC_FACTORY_CONTROL_WDT_TIMEOUT_S 4
#define SC_FACTORY_ENABLE_NVS_ERASE_RESET 0

// ============================================================================
// 5. VALIDASI KONFIGURASI KESELAMATAN
// ============================================================================

#if SC_FACTORY_AP_OPEN != 1
#error "SmartCooling release mesti mengekalkan AP terbuka tanpa kata laluan."
#endif

// Validasi pin - hanya jika peranti diaktifkan
#if SC_INTERNAL_PUMP_TYPE != 0
  #if SC_FACTORY_PIN_PUMP_PWM == SC_FACTORY_PIN_NTC || SC_FACTORY_PIN_PUMP_SSR == SC_FACTORY_PIN_NTC
  #error "Pin Pam bercanggah dengan pin NTC."
  #endif
#endif

#if SC_INTERNAL_FAN_TYPE != 0
  #if SC_FACTORY_PIN_FAN_PWM == SC_FACTORY_PIN_NTC || SC_FACTORY_PIN_FAN_SSR == SC_FACTORY_PIN_NTC
  #error "Pin Kipas bercanggah dengan pin NTC."
  #endif
#endif

#if SC_FACTORY_PIN_NTC == SC_FACTORY_PIN_ECU
#error "Pin NTC bercanggah dengan pin ECU."
#endif

#if SC_FACTORY_ADC_SAMPLE_COUNT <= (SC_FACTORY_ADC_TRIM_COUNT * 2)
#error "Tetapan ADC trim tidak sah."
#endif

#if SC_FACTORY_SD_CARD == 1
  #if SC_FACTORY_PIN_SD_CS == SC_FACTORY_PIN_NTC || SC_FACTORY_PIN_SD_CS == SC_FACTORY_PIN_ECU
  #error "Pin SD CS bercanggah dengan pin lain."
  #endif
#endif

#if SC_FACTORY_ENV_SENSOR != 0
  #if SC_FACTORY_PIN_I2C_SDA == SC_FACTORY_PIN_I2C_SCL
  #error "Pin I2C SDA dan SCL mesti berbeza."
  #endif
#endif

/*
 * SmartCooling - Factory Configuration Header
 * Versi: 3.0.0 (Stabil + Secure Defaults + SI Sentinel)
 * Sasaran: Super Mini ESP32 S3 HW-747
 *
 * PENTING: Fail ini WAJIB dikonfigurasi sebelum flashing firmware.
 * Sila pilih opsyen yang sesuai dengan perkakasan fizikal anda.
 *
 * CIRI UTAMA VERSI INI:
 * 1. TETAPAN KESELAMATAN KILANG: Semua output OFF secara lalai
 * 2. SI Sentinel aktif untuk perlindungan hallucination & serangan
 * 3. RTC Memory untuk ketahanan kuasa & pemulihan automatik
 * 4. Sokongan dual-framework: PlatformIO + ESP-IDF
 * 5. Web UI moden, responsif, tanpa branding luar
 */

#ifndef SMARTCOOLING_FACTORY_CONFIG_H
#define SMARTCOOLING_FACTORY_CONFIG_H

#define SC_FACTORY_PROFILE "super_mini_esp32_s3_hw747"
#define SC_FACTORY_HARDWARE_REV "hw-747"

// ============================================================================
// 1. KONFIGURASI KESELAMATAN KILANG (SECURE DEFAULTS)
// ============================================================================
// AMARAN: Untuk melindungi perkakasan pihak ketiga, semua output DIMATIKAN
// secara lalai. Pengguna WAJIB konfigurasi manual selepas pemasangan pertama.

// Jenis Pam: 0 = TIADA (OFF), 1 = SSR (On/Off lembut), 2 = PWM (kawalan kelajuan)
#define SC_FACTORY_PUMP_TYPE_NONE 0
#define SC_FACTORY_PUMP_TYPE_SSR 1
#define SC_FACTORY_PUMP_TYPE_PWM 2
#define SC_FACTORY_PUMP_TYPE SC_FACTORY_PUMP_TYPE_NONE  // ✅ SELAMAT: OFF secara lalai

// Jenis Kipas: 0 = TIADA (OFF), 1 = SSR (On/Off), 2 = PWM (kawalan kelajuan)
#define SC_FACTORY_FAN_TYPE_NONE 0
#define SC_FACTORY_FAN_TYPE_SSR 1
#define SC_FACTORY_FAN_TYPE_PWM 2
#define SC_FACTORY_FAN_TYPE SC_FACTORY_FAN_TYPE_NONE  // ✅ SELAMAT: OFF secara lalai

// Sensor Persekitaran:
// 0 = TIADA, 1 = BME280, 2 = AHT30, 3 = BMP280, 4 = BMP180
#define SC_FACTORY_SENSOR_NONE   0
#define SC_FACTORY_SENSOR_BME280 1
#define SC_FACTORY_SENSOR_AHT30  2
#define SC_FACTORY_SENSOR_BMP280 3
#define SC_FACTORY_SENSOR_BMP180 4
#define SC_FACTORY_ENV_SENSOR_TYPE SC_FACTORY_SENSOR_NONE  // ✅ SELAMAT: Tiada sensor lalai

// Micro SD Card: 0 = Tidak Aktif, 1 = Aktif
#define SC_FACTORY_SD_CARD_ENABLED 0  // ✅ SELAMAT: Tidak aktif secara lalai

// ============================================================================
// 2. PENGURUSAN PIN (DYNAMIC MAPPING)
// ============================================================================

#define SC_FACTORY_PIN_NTC 1
#define SC_FACTORY_PIN_ECU 6

// Pin Pam - Dinamik berdasarkan jenis
#if SC_FACTORY_PUMP_TYPE == SC_FACTORY_PUMP_TYPE_PWM
  #define SC_FACTORY_PIN_PUMP_PWM 3
  #define SC_FACTORY_PIN_PUMP_SSR -1
#elif SC_FACTORY_PUMP_TYPE == SC_FACTORY_PUMP_TYPE_SSR
  #define SC_FACTORY_PIN_PUMP_PWM -1
  #define SC_FACTORY_PIN_PUMP_SSR 2
#else
  #define SC_FACTORY_PIN_PUMP_PWM -1
  #define SC_FACTORY_PIN_PUMP_SSR -1
#endif

// Pin Kipas - Dinamik berdasarkan jenis
#if SC_FACTORY_FAN_TYPE == SC_FACTORY_FAN_TYPE_PWM
  #define SC_FACTORY_PIN_FAN_PWM 4
  #define SC_FACTORY_PIN_FAN_SSR -1
#elif SC_FACTORY_FAN_TYPE == SC_FACTORY_FAN_TYPE_SSR
  #define SC_FACTORY_PIN_FAN_PWM -1
  #define SC_FACTORY_PIN_FAN_SSR 5
#else
  #define SC_FACTORY_PIN_FAN_PWM -1
  #define SC_FACTORY_PIN_FAN_SSR -1
#endif

// Alias untuk keserasian kod lama
#define SC_FACTORY_PIN_SSR SC_FACTORY_PIN_PUMP_SSR
#define SC_FACTORY_PIN_PWM SC_FACTORY_PIN_FAN_PWM

// Pin SD Card - Hanya jika diaktifkan
#if SC_FACTORY_SD_CARD_ENABLED == 1
  #define SC_FACTORY_PIN_SD_CS 7
  #define SC_FACTORY_PIN_SD_MOSI 11
  #define SC_FACTORY_PIN_SD_MISO 13
  #define SC_FACTORY_PIN_SD_SCK 12
#endif

// Pin I2C Sensor - Hanya jika sensor diaktifkan
#if SC_FACTORY_ENV_SENSOR_TYPE != SC_FACTORY_SENSOR_NONE
  #define SC_FACTORY_PIN_I2C_SDA 8
  #define SC_FACTORY_PIN_I2C_SCL 9
#endif

// Pin Factory Reset Fizikal (GPIO ditekan semasa boot)
#define SC_FACTORY_RESET_PIN GPIO_NUM_0
#define SC_FACTORY_RESET_ACTIVE_LOW true

// ============================================================================
// 3. TETAPAN SISTEM ASAS DAN IDENTITI
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
#define SC_SYSTEM_SPEC_CODE 'K'

#define SC_FACTORY_TICK_MS 500UL
#define SC_FACTORY_PWM_FREQ 15000
#define SC_FACTORY_PWM_RES 8
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
// 4. VALIDASI KONFIGURASI KESELAMATAN
// ============================================================================

#if SC_FACTORY_AP_OPEN != 1
#error "SmartCooling release mesti mengekalkan AP terbuka tanpa kata laluan."
#endif

// Validasi jenis pam
#if SC_FACTORY_PUMP_TYPE != SC_FACTORY_PUMP_TYPE_NONE && \
    SC_FACTORY_PUMP_TYPE != SC_FACTORY_PUMP_TYPE_SSR && \
    SC_FACTORY_PUMP_TYPE != SC_FACTORY_PUMP_TYPE_PWM
#error "Jenis pam kilang tidak sah. Pilih NONE (0), SSR (1), atau PWM (2)."
#endif

// Validasi jenis kipas
#if SC_FACTORY_FAN_TYPE != SC_FACTORY_FAN_TYPE_NONE && \
    SC_FACTORY_FAN_TYPE != SC_FACTORY_FAN_TYPE_SSR && \
    SC_FACTORY_FAN_TYPE != SC_FACTORY_FAN_TYPE_PWM
#error "Jenis kipas kilang tidak sah. Pilih NONE (0), SSR (1), atau PWM (2)."
#endif

// Validasi pin - hanya jika peranti diaktifkan
#if SC_FACTORY_PUMP_TYPE != SC_FACTORY_PUMP_TYPE_NONE
  #if SC_FACTORY_PIN_PUMP_PWM == SC_FACTORY_PIN_NTC || SC_FACTORY_PIN_PUMP_SSR == SC_FACTORY_PIN_NTC
  #error "Pin Pam bercanggah dengan pin NTC."
  #endif
#endif

#if SC_FACTORY_FAN_TYPE != SC_FACTORY_FAN_TYPE_NONE
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

#if SC_FACTORY_SD_CARD_ENABLED == 1
  #if SC_FACTORY_PIN_SD_CS == SC_FACTORY_PIN_NTC || SC_FACTORY_PIN_SD_CS == SC_FACTORY_PIN_ECU
  #error "Pin SD CS bercanggah dengan pin lain."
  #endif
#endif

#if SC_FACTORY_ENV_SENSOR_TYPE != SC_FACTORY_SENSOR_NONE
  #if SC_FACTORY_PIN_I2C_SDA == SC_FACTORY_PIN_I2C_SCL
  #error "Pin I2C SDA dan SCL mesti berbeza."
  #endif
#endif

#endif

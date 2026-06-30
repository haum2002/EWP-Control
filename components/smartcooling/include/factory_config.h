/*
 * SmartCooling - Factory Configuration Header
 * Versi: 2.5.0 (Stabil + Gate Release V2)
 * Sasaran: Super Mini ESP32 S3 HW-747
 *
 * PENTING: Fail ini WAJIB dikonfigurasi sebelum flashing firmware.
 * Sila pilih opsyen yang sesuai dengan perkakasan fizikal anda.
 *
 * CIRI UTAMA VERSI INI:
 * 1. Konfigurasi konservatif untuk board HW-747 sebenar.
 * 2. Gate release menghalang ciri yang belum disahkan pada firmware V2.
 * 3. RTC Memory untuk diagnostik warm reset dan brownout.
 * 4. Struktur ESP-IDF component dengan build PlatformIO yang disahkan.
 */

#ifndef SMARTCOOLING_FACTORY_CONFIG_H
#define SMARTCOOLING_FACTORY_CONFIG_H

#define SC_FACTORY_PROFILE "super_mini_esp32_s3_hw747"
#define SC_FACTORY_HARDWARE_REV "hw-747"

// ============================================================================
// 1. KONFIGURASI PERKAKASAN RELEASE HW-747
// ============================================================================

// Jenis Pam: 0 = SSR (On/Off lembut), 1 = PWM (kawalan kelajuan)
#define SC_FACTORY_PUMP_TYPE_SSR 0
#define SC_FACTORY_PUMP_TYPE_PWM 1
#define SC_FACTORY_PUMP_TYPE SC_FACTORY_PUMP_TYPE_SSR

// Jenis Kipas: 0 = SSR (On/Off), 1 = PWM (kawalan kelajuan)
#define SC_FACTORY_FAN_TYPE_SSR 0
#define SC_FACTORY_FAN_TYPE_PWM 1
#define SC_FACTORY_FAN_TYPE SC_FACTORY_FAN_TYPE_PWM

// Sensor Persekitaran:
// 0 = TIADA, 1 = BME280, 2 = AHT30, 3 = BMP280, 4 = BMP180
#define SC_FACTORY_SENSOR_NONE   0
#define SC_FACTORY_SENSOR_BME280 1
#define SC_FACTORY_SENSOR_AHT30  2
#define SC_FACTORY_SENSOR_BMP280 3
#define SC_FACTORY_SENSOR_BMP180 4
#define SC_FACTORY_ENV_SENSOR_TYPE SC_FACTORY_SENSOR_NONE

// Micro SD Card: 0 = Tidak Aktif, 1 = Aktif
#define SC_FACTORY_SD_CARD_ENABLED 0

// ============================================================================
// 2. PENGURUSAN PIN
// ============================================================================

#define SC_FACTORY_PIN_NTC 1
#define SC_FACTORY_PIN_ECU 6

#if SC_FACTORY_PUMP_TYPE == SC_FACTORY_PUMP_TYPE_PWM
  #define SC_FACTORY_PIN_PUMP_PWM 3
  #define SC_FACTORY_PIN_PUMP_SSR -1
#else
  #define SC_FACTORY_PIN_PUMP_PWM -1
  #define SC_FACTORY_PIN_PUMP_SSR 2
#endif

#if SC_FACTORY_FAN_TYPE == SC_FACTORY_FAN_TYPE_PWM
  #define SC_FACTORY_PIN_FAN_PWM 4
  #define SC_FACTORY_PIN_FAN_SSR -1
#else
  #define SC_FACTORY_PIN_FAN_PWM -1
  #define SC_FACTORY_PIN_FAN_SSR 5
#endif

// Alias konservatif untuk gate release dan kod lama.
#define SC_FACTORY_PIN_SSR SC_FACTORY_PIN_PUMP_SSR
#define SC_FACTORY_PIN_PWM SC_FACTORY_PIN_FAN_PWM

#if SC_FACTORY_SD_CARD_ENABLED == 1
  #define SC_FACTORY_PIN_SD_CS 7
  #define SC_FACTORY_PIN_SD_MOSI 11
  #define SC_FACTORY_PIN_SD_MISO 13
  #define SC_FACTORY_PIN_SD_SCK 12
#endif

#if SC_FACTORY_ENV_SENSOR_TYPE != SC_FACTORY_SENSOR_NONE
  #define SC_FACTORY_PIN_I2C_SDA 8
  #define SC_FACTORY_PIN_I2C_SCL 9
#endif

// Pin factory reset fizikal hanya digunakan jika NVS erase reset diaktifkan.
#define SC_FACTORY_RESET_PIN 0
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
// 4. VALIDASI KONFIGURASI
// ============================================================================

#if SC_FACTORY_AP_OPEN != 1
#error "SmartCooling release mesti mengekalkan AP terbuka tanpa kata laluan."
#endif

#if SC_FACTORY_SD_CARD_ENABLED != 0
#error "Micro SD belum aktif dalam firmware V2 release."
#endif

#if SC_FACTORY_ENV_SENSOR_TYPE != SC_FACTORY_SENSOR_NONE
#error "Sensor persekitaran belum aktif dalam firmware V2 release."
#endif

#if SC_FACTORY_PUMP_TYPE != SC_FACTORY_PUMP_TYPE_SSR && SC_FACTORY_PUMP_TYPE != SC_FACTORY_PUMP_TYPE_PWM
#error "Jenis pam kilang tidak sah."
#endif

#if SC_FACTORY_FAN_TYPE != SC_FACTORY_FAN_TYPE_SSR && SC_FACTORY_FAN_TYPE != SC_FACTORY_FAN_TYPE_PWM
#error "Jenis kipas kilang tidak sah."
#endif

#if SC_FACTORY_PIN_NTC == SC_FACTORY_PIN_ECU
#error "Pin NTC bercanggah dengan pin ECU."
#endif

#if SC_FACTORY_PUMP_TYPE == SC_FACTORY_PUMP_TYPE_PWM && SC_FACTORY_PIN_PUMP_PWM == SC_FACTORY_PIN_NTC
#error "Pin PWM Pam bercanggah dengan pin NTC."
#endif

#if SC_FACTORY_PUMP_TYPE == SC_FACTORY_PUMP_TYPE_SSR && SC_FACTORY_PIN_PUMP_SSR == SC_FACTORY_PIN_NTC
#error "Pin SSR Pam bercanggah dengan pin NTC."
#endif

#if SC_FACTORY_FAN_TYPE == SC_FACTORY_FAN_TYPE_PWM && SC_FACTORY_PIN_FAN_PWM == SC_FACTORY_PIN_NTC
#error "Pin PWM Kipas bercanggah dengan pin NTC."
#endif

#if SC_FACTORY_FAN_TYPE == SC_FACTORY_FAN_TYPE_SSR && SC_FACTORY_PIN_FAN_SSR == SC_FACTORY_PIN_NTC
#error "Pin SSR Kipas bercanggah dengan pin NTC."
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

/*
 * SmartCooling V2 factory profile.
 *
 * Fail ini ialah pusat tetapan kilang untuk varian hardware HW-747.
 * Nilai di sini sengaja konservatif supaya build lalai kekal sama seperti
 * firmware V2 yang telah diuji pada Super Mini ESP32-S3.
 * 
 * KONFIGURASI WAJIB SEBELUM FLASH:
 * - Pilih jenis pam: PWM (kawalan kelajuan) atau SSR (On/Off lembut)
 * - Pilih jenis kipas: PWM (kawalan kelajuan) atau SSR (On/Off)
 * - Pilih sensor persekitaran: BME280, AHT30, BMP280, BMP180, atau TIADA
 * - Micro SD Card: Aktif/Tidak untuk simpan log & data latihan SI
 */
#ifndef SMARTCOOLING_FACTORY_CONFIG_H
#define SMARTCOOLING_FACTORY_CONFIG_H

#define SC_FACTORY_PROFILE "super_mini_esp32_s3_hw747"
#define SC_FACTORY_HARDWARE_REV "hw-747"

// ============================================================================
// 1. KONFIGURASI PERKAKASAN (WAJIB DIPILIH SEBELUM FLASH)
// ============================================================================

// Jenis Pam: 0 = SSR (On/Off lembut), 1 = PWM (kawalan kelajuan)
#define SC_FACTORY_PUMP_TYPE_SSR 0
#define SC_FACTORY_PUMP_TYPE_PWM 1
#define SC_FACTORY_PUMP_TYPE SC_FACTORY_PUMP_TYPE_SSR  // <-- UBAH DI SINI

// Jenis Kipas: 0 = SSR (On/Off), 1 = PWM (kawalan kelajuan)
#define SC_FACTORY_FAN_TYPE_SSR 0
#define SC_FACTORY_FAN_TYPE_PWM 1
#define SC_FACTORY_FAN_TYPE SC_FACTORY_FAN_TYPE_PWM  // <-- UBAH DI SINI

// Sensor Persekitaran (Pilihan Kilang):
// 0 = TIADA, 1 = BME280, 2 = AHT30, 3 = BMP280, 4 = BMP180
#define SC_FACTORY_SENSOR_NONE   0
#define SC_FACTORY_SENSOR_BME280 1
#define SC_FACTORY_SENSOR_AHT30  2
#define SC_FACTORY_SENSOR_BMP280 3
#define SC_FACTORY_SENSOR_BMP180 4
#define SC_FACTORY_ENV_SENSOR_TYPE SC_FACTORY_SENSOR_NONE  // <-- UBAH DI SINI

// Micro SD Card: 0 = Tidak Aktif, 1 = Aktif (untuk log & data latihan SI)
#define SC_FACTORY_SD_CARD_ENABLED 0  // <-- UBAH DI SINI

// ============================================================================
// 2. PENGURUSAN PIN (Pemetaan pin dinamik mengikut konfigurasi)
// ============================================================================

// Pin asas untuk semua konfigurasi
#define SC_FACTORY_PIN_NTC 1
#define SC_FACTORY_PIN_ECU 6

// Pin dinamik berdasarkan jenis pam
#if SC_FACTORY_PUMP_TYPE == SC_FACTORY_PUMP_TYPE_PWM
  #define SC_FACTORY_PIN_PUMP_PWM 3   // GPIO untuk PWM pam
  #define SC_FACTORY_PIN_PUMP_SSR -1  // Tidak digunakan
#else
  #define SC_FACTORY_PIN_PUMP_PWM -1  // Tidak digunakan
  #define SC_FACTORY_PIN_PUMP_SSR 2   // GPIO untuk SSR pam
#endif

// Pin dinamik berdasarkan jenis kipas
#if SC_FACTORY_FAN_TYPE == SC_FACTORY_FAN_TYPE_PWM
  #define SC_FACTORY_PIN_FAN_PWM 4    // GPIO untuk PWM kipas
  #define SC_FACTORY_PIN_FAN_SSR -1   // Tidak digunakan
#else
  #define SC_FACTORY_PIN_FAN_PWM -1   // Tidak digunakan
  #define SC_FACTORY_PIN_FAN_SSR 5    // GPIO untuk SSR kipas
#endif

// Pin untuk SD Card (jika diaktifkan)
#if SC_FACTORY_SD_CARD_ENABLED == 1
  #define SC_FACTORY_PIN_SD_CS 7
  #define SC_FACTORY_PIN_SD_MOSI 11
  #define SC_FACTORY_PIN_SD_MISO 13
  #define SC_FACTORY_PIN_SD_SCK 12
#endif

// Pin untuk sensor persekitaran (I2C)
#if SC_FACTORY_ENV_SENSOR_TYPE != SC_FACTORY_SENSOR_NONE
  #define SC_FACTORY_PIN_I2C_SDA 8
  #define SC_FACTORY_PIN_I2C_SCL 9
#endif

// ============================================================================
// 3. TETAPAN SISTEM ASAS
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

// ============================================================================
// 4. VALIDASI KONFIGURASI (Compile-time checks)
// ============================================================================

#if SC_FACTORY_AP_OPEN != 1
#error "SmartCooling V2 release mesti mengekalkan AP terbuka tanpa kata laluan."
#endif

// Validate pin conflicts
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

// Validate SD card pins if enabled
#if SC_FACTORY_SD_CARD_ENABLED == 1
  #if SC_FACTORY_PIN_SD_CS == SC_FACTORY_PIN_NTC || SC_FACTORY_PIN_SD_CS == SC_FACTORY_PIN_ECU
  #error "Pin SD CS bercanggah dengan pin lain."
  #endif
#endif

// Validate I2C pins if sensor enabled
#if SC_FACTORY_ENV_SENSOR_TYPE != SC_FACTORY_SENSOR_NONE
  #if SC_FACTORY_PIN_I2C_SDA == SC_FACTORY_PIN_I2C_SCL
  #error "Pin I2C SDA dan SCL mesti berbeza."
  #endif
#endif

#endif

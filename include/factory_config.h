/*
 * SmartCooling V2 factory profile.
 *
 * Fail ini ialah pusat tetapan kilang untuk varian hardware HW-747.
 * Nilai di sini sengaja konservatif supaya build lalai kekal sama seperti
 * firmware V2 yang telah diuji pada Super Mini ESP32-S3.
 */
#ifndef SMARTCOOLING_FACTORY_CONFIG_H
#define SMARTCOOLING_FACTORY_CONFIG_H

#define SC_FACTORY_PROFILE "super_mini_esp32_s3_hw747"
#define SC_FACTORY_HARDWARE_REV "hw-747"

#define SC_FACTORY_AP_SSID "EWP-SYSTEM-PRO"
#define SC_FACTORY_AP_OPEN 1
#define SC_FACTORY_DOMAIN_HOST "smartcooling.local"
#define SC_FACTORY_MDNS_HOST "smartcooling"

#define SC_FACTORY_DEFAULT_WEB_PASSWORD "12345678"
#define SC_FACTORY_RECOVERY_PIN "747747"

#define SC_FACTORY_PIN_NTC 1
#define SC_FACTORY_PIN_SSR 2
#define SC_FACTORY_PIN_PWM 4
#define SC_FACTORY_PIN_ECU 6

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

#if SC_FACTORY_AP_OPEN != 1
#error "SmartCooling V2 release mesti mengekalkan AP terbuka tanpa kata laluan."
#endif

#if SC_FACTORY_PIN_NTC == SC_FACTORY_PIN_SSR || SC_FACTORY_PIN_NTC == SC_FACTORY_PIN_PWM || SC_FACTORY_PIN_NTC == SC_FACTORY_PIN_ECU
#error "Pin NTC bercanggah dengan pin output/input lain."
#endif

#if SC_FACTORY_PIN_SSR == SC_FACTORY_PIN_PWM || SC_FACTORY_PIN_SSR == SC_FACTORY_PIN_ECU || SC_FACTORY_PIN_PWM == SC_FACTORY_PIN_ECU
#error "Pin SSR, PWM, dan ECU mesti berbeza."
#endif

#if SC_FACTORY_ADC_SAMPLE_COUNT <= (SC_FACTORY_ADC_TRIM_COUNT * 2)
#error "Tetapan ADC trim tidak sah."
#endif

#endif

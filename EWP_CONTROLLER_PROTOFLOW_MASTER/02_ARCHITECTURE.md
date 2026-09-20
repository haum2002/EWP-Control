# ARCHITECTURE

```text
BATTERY -> PROTECTION/FUSE/REVERSE POLARITY/TVS/EMI -> CONTROLLED POWER
                                                   |
                         +-------------------------+------------------+
                         |                                            |
                      RP2350B                                    ESP32-S3
                         |                                            |
                 real-time/safety                           Wi-Fi/BLE/WebApp/OTA
                         |
       +-----------------+------------------+
       |        |        |        |         |
   Temp/RPM  OEM FAN   Pump      Fan   Current/Voltage

RP2350B <---- monitored MCU communication ----> ESP32-S3
```

## Physical zones
A. Power entry/protection
B. High-current pump/fan switching
C. RP2350B digital/control
D. ESP32-S3 RF/memory
E. Precision analogue
F. Safety/supervisor
G. RTC/SD/service

## Required schematic pages
P01_POWER_ENTRY
P02_POWER_RAILS
P03_HARDWARE_SAFETY
P04_RP2350B_CONTROL
P05_RP2350B_FLASH_CLOCK_RESET_DEBUG
P06_ESP32S3
P07_ESP32S3_FLASH_PSRAM_RF
P08_MCU_COMMUNICATION
P09_PRECISION_ADC_DAC
P10_SENSOR_RPM_OEM_INPUT
P11_PUMP_CHANNEL
P12_FAN_CHANNEL
P13_CURRENT_VOLTAGE_MONITORING
P14_RTC_SD_SERVICE
P15_EXTERNAL_WIRE_LANDINGS

Every page must contain actual electrical circuitry. A section box is not a circuit.

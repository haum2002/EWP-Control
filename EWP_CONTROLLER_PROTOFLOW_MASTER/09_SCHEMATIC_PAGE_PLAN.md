# COMPLETE SCHEMATIC PAGE PLAN

P01_POWER_ENTRY — battery connector, fuse/protection, reverse polarity, TVS, EMI, protected rail.
P02_POWER_RAILS — protected battery, 5V if required, 3V3, analogue, RP2350 regulator, ESP32 supply, memory/sensor/RTC rails.
P03_HARDWARE_SAFETY — supervisor, watchdog, thermal comparator/interlock, reset/safe-state.
P04_RP2350B_CONTROL — bare RP2350B, supplies, critical I/O.
P05_RP2350B_FLASH_CLOCK_RESET_DEBUG — boot/QSPI flash, crystal, reset, SWD/service.
P06_ESP32S3 — bare SoC, power, straps, reset, crystal, service/USB if required.
P07_ESP32S3_FLASH_PSRAM_RF — flash, PSRAM, high-speed series elements, RF matching and antenna.
P08_MCU_COMMUNICATION — RP2350B↔ESP32-S3 bus, heartbeat, CRC, timeout, reset/status.
P09_PRECISION_ADC_DAC — external ADC, reference, filtering, DAC where required.
P10_SENSOR_RPM_OEM_INPUT — coolant sensor, environmental sensor, RPM conditioning, OEM fan request, key/ignition.
P11_PUMP_CHANNEL — complete universal 2/3/4-wire pump power/control/feedback/protection.
P12_FAN_CHANNEL — complete universal 2/3/4-wire fan power/control/feedback/protection.
P13_CURRENT_VOLTAGE_MONITORING — pump/fan current and voltage, Kelvin shunts, monitors.
P14_RTC_SD_SERVICE — RTC/backup, SD, service/test.
P15_EXTERNAL_WIRE_LANDINGS — all external wire interfaces with wire names, solder landing IDs, protection, net names and current/clearance notes. NO connector/socket footprints.

No page may be an empty box. Every page must contain actual circuitry.

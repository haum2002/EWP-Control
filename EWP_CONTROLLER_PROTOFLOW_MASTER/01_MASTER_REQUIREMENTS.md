# MASTER REQUIREMENTS

## Architecture
- Primary real-time MCU: RP2350B bare IC, QFN-80.
- Secondary UI/connectivity MCU: bare ESP32-S3 SoC.
- No Pico, Pico 2, WROOM, MINI, XIAO, DevKit, or MCU module.
- RP2350B remains capable of critical cooling control without ESP32-S3, Wi-Fi, WebApp, SD, or AI.

## PCB
- 4 layers mandatory.
- Compact as practical, without sacrificing thermal, RF, analogue, safety, clearance, or current requirements.
- SMD-first. Through-hole COMPONENTS are prohibited unless a documented mechanical/electrical requirement makes them necessary and no suitable SMD solution exists. PCB wire-entry holes/slots for direct-soldered external wires are allowed and are not connectors.
- L1 components/critical signals/RF; L2 continuous GND; L3 power/selected signals; L4 signals. Follow vendor guidance where more specific.

## Required subsystems
1. Battery entry/protection
2. Power rails
3. Hardware safety/watchdog/supervisor
4. RP2350B
5. RP2350B clock/reset/QSPI/debug
6. Bare ESP32-S3
7. ESP32-S3 flash/PSRAM/RF
8. Secure/robust MCU-to-MCU bus
9. External precision ADC >=24-bit nominal target
10. External DAC >=24-bit nominal target where required
11. RTC with dedicated backup
12. SD storage
13. Dedicated coolant temperature sensing
14. Environmental temperature/humidity/pressure sensor
15. Universal RPM input
16. OEM ECU fan-request input
17. Universal pump channel
18. Universal fan channel
19. Pump current/voltage monitoring
20. Fan current/voltage monitoring
21. Key/ignition sensing
22. Low-battery hard load disconnect
23. Direct-wire external solder landings; NO external connectors/sockets
24. CAN/service interface where required by the approved architecture, implemented without a detachable connector
25. Production programming through exposed PCB test pads only; no USB/SWD/UART/service connector footprints
26. Factory calibration through exposed test/calibration pads and a temporary external fixture; calibration data versioned, CRC-protected and stored in non-volatile memory
27. Bare ESP32-S3 and RP2350B programming/recovery access must be planned before PCB routing

## Power states
MODE 1: KEY OFF / ENGINE OFF; after-run may continue according to programmed shutdown logic.
MODE 2: KEY ON / ENGINE ON OR ENGINE OFF; engine state inferred from RPM. KEY ON + RPM=0 is a substate.
MODE 3: LOW BATTERY / KEY OFF; main controller load physically disconnected. RTC remains separately powered.

## Sensors
Controller uses its own dedicated coolant sensor. Do not use the motorcycle ECU temperature sensor.

## RPM
One protected universal physical RPM input may support, when electrically valid: coil negative, coil positive, ECU tach/RPM, Hall/digital, 3.3V/5V square wave, protected 12V square wave, external sensor. Never connect ignition coil directly to an MCU.

## Pump/fan
Same PCB supports 2-wire, 3-wire and 4-wire configurations. Conceptual connector: +12V, GND, CONTROL, FEEDBACK. Actual pin use must be documented. Provide appropriate high-side/low-side power switching, ON/OFF, PWM where applicable, feedback and current/voltage monitoring. Do not invent a PWM protocol for the 2-wire Bosch EWP.

## Memory
ESP32-S3 physical targets: NOR Flash >=128 MB and PSRAM >=64 MB. 128 Mb = 16 MB, so 128 Mb does NOT satisfy 128 MB. Verify supported memory topology before placement.

## OTA
One coordinated package for RP2350B + ESP32-S3, with manifest, hardware compatibility, versions, integrity/signature data and rollback. Cooling must remain safe during updates.

## Security
Use only documented vendor-supported security mechanisms: RP2350 secure boot/signing/OTP/debug controls as supported; ESP32-S3 Secure Boot, Flash Encryption, signed OTA and production debug restrictions as supported.

## Direct-wire external connections
- External wires are soldered directly to dedicated PCB copper landings.
- No battery/pump/fan/sensor/RPM/ignition/CAN/service connector socket is permitted.
- High-current landings and traces must be sized from actual current, copper thickness, temperature-rise and voltage-drop requirements.
- Wire gauge/cross-section must be documented per external circuit.
- Provide PCB/enclosure strain relief so the solder fillet is not the sole mechanical load path.
- The complete PCB target remains below 60 mm x 60 mm unless an electrical/thermal requirement forces a documented exception.

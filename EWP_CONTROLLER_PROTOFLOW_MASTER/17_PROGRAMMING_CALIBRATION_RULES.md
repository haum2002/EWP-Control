# PROGRAMMING / CALIBRATION / SERVICE RULES

## Core manufacturing rule
The finished PCB must NOT contain a USB receptacle, SWD connector, UART connector,
programming header, pogo-pin receptacle, or other detachable service connector solely
for programming, calibration, or debugging.

Programming and calibration are performed using exposed PCB test/program pads contacted
by a temporary external fixture (for example a bed-of-nails/pogo-pin fixture) during
manufacturing or service.

The temporary fixture is external to the product PCB. No mating connector footprint is
placed on the product PCB.

## RP2350B programming/debug interface
Reserve exposed test pads for:
- RP_SWDIO
- RP_SWCLK
- RP_RUN
- GND
- 3V3_TARGET_SENSE

These are service/program pads only. Do not route them through a detachable connector.
RP2350 exposes SWCLK and SWDIO on its dedicated SWD interface, which provides debug
access to the processors. Keep the pads accessible to the manufacturing/service fixture.

Reference: Raspberry Pi RP2350 datasheet, SWD section.

## ESP32-S3 programming interface
Primary production download interface: UART0 test pads.
Reserve exposed pads for:
- ESP_UART0_TXD / GPIO43
- ESP_UART0_RXD / GPIO44
- ESP_GPIO0_BOOT
- ESP_CHIP_PU_EN
- GND
- 3V3_TARGET_SENSE

Use an external USB-to-UART tool only as the temporary manufacturing/service fixture.
Do NOT place a USB-to-UART bridge on the PCB unless another requirement explicitly
requires it.

ESP32-S3 UART0 is the documented firmware-download/log interface. GPIO43/GPIO44 are
UART0 by default, and GPIO0 / CHIP_PU are retained for download/reset control.

Reference: Espressif ESP32-S3 Hardware Design Guidelines and Download Guidelines.

## ESP32-S3 USB
No USB receptacle is permitted.

The ESP32-S3 USB D-/D+ function may remain electrically available on compact exposed
test pads only when needed for service/recovery:
- ESP_USB_D-
- ESP_USB_D+

These pads are optional service points, not a connector, and must not compromise RF,
GPIO allocation, flash/PSRAM routing, or board compactness.

Primary production flashing remains UART0 unless a later verified manufacturing process
explicitly selects USB.

## Factory programming order
1. Apply controlled external target power through the test fixture.
2. Verify target ground and 3V3/1V1 rails are within the required limits.
3. Program RP2350B firmware using SWD.
4. Program/configure ESP32-S3 firmware using UART0 download mode.
5. Verify firmware versions and hardware identity.
6. Write manufacturing calibration data.
7. Perform power-cycle and boot verification.
8. Run functional safety / actuator checks.
9. Run final verification and record a production test result.
10. Apply production security restrictions only after programming/calibration/verification
    succeeds.

## Calibration architecture
Calibration data must be stored in a dedicated versioned non-volatile data region,
not hard-coded into source code.

Required calibration record structure:
- calibration schema/version
- board hardware revision
- sensor/channel identifier
- coefficient(s)
- offset
- gain, where applicable
- valid range / test conditions
- calibration timestamp or production record identifier, when available
- CRC/integrity field
- validity marker / commit state

Never store calibration data without integrity checking.
Do not use one calibration coefficient for multiple channels unless the hardware and
measurement chain have been proven identical.

## Calibration channels
At minimum, provide service/test access for measurement verification of:
- controller 12V input measurement
- pump voltage measurement
- pump current measurement
- fan voltage measurement
- fan current measurement
- precision ADC reference/input verification
- dedicated coolant temperature input
- environmental sensor communication/identity
- RPM capture input
- DAC output verification where a DAC is actually implemented

Exact test points and injection topology must be derived from the final schematic and
actual selected components. Do not invent a calibration circuit merely to create pads.

For analog calibration, the production fixture must use known, traceable reference
conditions appropriate to the channel. The PCB itself should expose measurement/test
nodes while retaining input protection and normal operating safety.

## Calibration isolation
Calibration/test pads must not become ordinary external field connectors.
Do not place pads where vibration, solder bridges, or accidental wire attachment can
create a field failure.

High-current motor paths must not be routed through small calibration pads.
Use Kelvin sense points for current shunts where appropriate.

## Security interaction
Factory/debug access may be available before production security locking.
After successful provisioning, apply the approved production security configuration.
The architecture must retain a documented secure recovery/update path rather than
assuming that permanently open debug access is acceptable.

Do not permanently lock a debug interface until:
- RP2350 firmware is verified
- ESP32-S3 firmware is verified
- calibration is verified and stored
- rollback/recovery path is verified
- production test is passed

## Completion criterion
The schematic is not complete until all required programming, calibration and service
pads are explicitly named, physically placed in the PCB plan, and documented.
No USB/programming/service connector footprint may appear.

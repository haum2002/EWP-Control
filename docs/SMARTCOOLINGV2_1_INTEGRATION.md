# SmartCoolingv2.1 Integration Contract

## Scope

SmartCoolingv2.1 combines the EWP Controller hardware project with the SmartCooling high-level firmware. The current repository is the integration source of truth.

The two upstream GitHub URLs supplied for comparison returned HTTP 404 during this integration pass, so their contents were not treated as verified inputs.

## Processor ownership

- RP2350B (U1) owns safety-critical pump, fan, coolant sensing, voltage/current monitoring, watchdog and autonomous fallback.
- ESP32-S3 (U2) owns the web UI, Wi-Fi, BLE, diagnostics, high-level logging and OTA orchestration.
- Loss of ESP32-S3 must not stop critical cooling control.

## Inter-MCU protocol

The transport-neutral frame contract is defined in `components/smartcooling/include/ewp_link_protocol.h`.

It defines:

- frame magic and protocol version
- message type
- payload length
- sequence counter
- payload CRC field
- heartbeat, status, command, configuration, fault and acknowledgement message classes
- heartbeat timeout policy

No GPIO or UART assignment is made by the header. Pin allocation remains blocked until the schematic assigns a verified RP2350B/U2 link.

## Current ESP32-S3 profile

The existing HW-747 factory profile remains active:

- spec `SP`
- pump SSR on GPIO2
- fan PWM on GPIO4
- coolant NTC on GPIO1
- ECU input on GPIO6
- I2C sensor bus on GPIO8/GPIO9

These values are firmware defaults for the Super Mini HW-747 profile. They are not evidence that the current bare-chip EWP schematic uses the same GPIO allocation.

## Verification gates

Before enabling the transport in firmware:

1. Verify the U1/U2 physical link in the KiCad schematic.
2. Assign only pins that are free after flash, PSRAM, RF, UART and boot-strapping audits.
3. Add the RP2350B-side frame parser, CRC check, sequence validation and heartbeat timeout.
4. Test ESP32-S3 loss while RP2350B continues autonomous cooling.
5. Re-run ERC, then build and exercise the firmware status endpoint.

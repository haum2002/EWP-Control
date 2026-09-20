# CHANGELOG

## 2026-09-19
Created the master ProtoFlow recovery/design package after repeated incomplete module-oriented schematic builds and project-folder loss.

Objectives:
- bare-IC architecture
- SMD-first compact 4-layer PCB
- complete schematic page plan
- strict no-guessing rules
- verification gates
- mandatory error correction before completion
- explicit project recovery workflow


## 2026-09-19 — v1.1
Added a mandatory component-substitution policy so temporary catalog/CAD/rate-limit failures do not unnecessarily stop the project. Verified SMD equivalents are allowed; random/convenience substitutions remain prohibited. Added mandatory BOM/component audit fields and special checks for RF/antenna, memory, precision analog and power/protection components.


## 2026-09-19 — v1.2
Added a mandatory no-external-connector architecture. Battery, pump, fan, sensor, RPM,
ignition and other field wires are soldered directly to dedicated PCB landings. Added
wire-size/copper/thermal/voltage-drop/spacing/strain-relief requirements and a <60 x 60 mm
board target. Clarified that plated wire-entry holes/slots are PCB features, not connectors.
Removed connector-dependent SD/service assumptions and replaced them with onboard storage
or compact service/test pads where applicable.


## 2026-09-19 — v1.2 final patch
Corrected the architecture page label to P15_EXTERNAL_WIRE_LANDINGS so the page plan no longer names external connectors.

## 2026-09-19 — v1.3
Added production programming/calibration architecture based on exposed PCB test pads
and a temporary external fixture. No USB receptacle, SWD header, UART header, or other
detachable service connector is permitted. RP2350B uses SWD pads; ESP32-S3 uses UART0
pads with GPIO0/CHIP_PU control. Optional ESP32-S3 USB D+/D- pads may exist only as
compact service test points.

Added mandatory current-build audit because ProtoFlow reported U2 as an ESP32-S3 module
and used the name VREG_VOUT. The master architecture requires a bare ESP32-S3 SoC, and
the official RP2350 pin set uses VREG_VIN/VREG_FB/VREG_LX/VREG_PGND/VREG_AVDD with DVDD
as the 1.1 V digital core supply. These items must be verified before PCB finalization.

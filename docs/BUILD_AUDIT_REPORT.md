# BUILD AUDIT REPORT — EWP Control / SmartCooling v2.1

> Report generated from static analysis of `EWP Control.kicad_sch`, `EWP Control.kicad_pcb`, and `ERC.rpt`.
> Audit follows master instruction §44 format and ProtoFlow `18_CURRENT_BUILD_AUDIT_FIX.md` report contract.

---

## STATUS

| Field | Value |
|---|---|
| **Current gate** | Gate 9 — Complete schematic (NOT PASSED) |
| **Gate progress** | Gate 0–8 partially satisfied; Gate 9 BLOCKED by ERC errors + connector violations; Gate 10–11 not started |
| **State** | **BLOCKED** — cannot proceed to PCB routing until ERC errors are resolved and connector violations removed |

---

## COMPONENTS

### Inventory summary

| Prefix | Count | Category |
|---|---|---|
| U | 16 | ICs (2 MCU + memory + power + ADC/DAC + sensor + RTC + monitor + watchdog + power switches) |
| C | 55 | Capacitors (0805 SMD) |
| R | 32 | Resistors (0805 SMD) |
| X | 3 | Crystals/oscillators |
| CN | 1 | Connector (VIOLATION) |
| Q | 1 | MOSFET |
| L | 1 | Inductor |
| D | 1 | Schottky diode |
| SW | 1 | Push button |
| **Total** | **110** | |

PCB footprints placed: **93** (some schematic symbols are power-port instances, not physical components).

### STOP CONDITION 1 — U2 bare-chip verification: **RESOLVED**

| Field | Value |
|---|---|
| U2 MPN | ESP32-S3 |
| U2 footprint | `QFN-56_L7.0-W7.0-P0.40-TL-EP4.0` |
| U2 datasheet | LCSC C2913192 |
| Verdict | **Bare SoC** — QFN-56 land-pattern, NOT a WROOM/WROVER/XIAO/DevKit module. STOP CONDITION 1 cleared. |

### STOP CONDITION 2 — RP2350B regulator pin naming: **RESOLVED**

| Pin name | Present in schematic |
|---|---|
| `VREG_VOUT` | **No** (the wrong/aliased name is NOT used) |
| `VREG_VIN` | Yes |
| `VREG_LX` | Yes |
| `VREG_FB` | Yes |
| `VREG_PGND` | Yes |
| `VREG_AVDD` | Yes |
| `DVDD` | Yes |

Verdict: regulator pin naming follows the official RP2350B pin set. STOP CONDITION 2 cleared.

### Component detail (key ICs)

| RefDes | Value / MPN | Footprint | Function | Page |
|---|---|---|---|---|
| U1 | RP2350B (C42415655) | QFN-80 | RP2350B bare MCU | P04, P05 |
| U2 | ESP32-S3 (C2913192) | QFN-56 | ESP32-S3 bare SoC | P06, P07 |
| U3 | W25Q128JVSIQTR | SOIC-8 | RP2350B QSPI flash (128 Mbit) | P05 |
| U4 | MT25QL01GBBB8ESF | SO-16 | ESP32-S3 flash (1 Gbit) | P07 |
| U5 | APS51208N-OBR-BD | BGA-24 | ESP32-S3 PSRAM (512 Mbit) | P07 |
| U6 | AD7124-8BCPZ | LFCSP-32 | Precision ADC (8-ch, 24-bit) | P09 |
| U7 | AD5689RBRUZ | TSSOP-16 | Precision DAC (16-bit, 4-ch) | P09 |
| U8 | TPS54540DDAR | SOIC-8-EP | Buck converter (5.5 V → 3.3 V rail) | P02 |
| U9 | TLV75533PDBVR | SOT-23-5 | 3.3 V LDO | P02 |
| U10 | BME280 | LGA-8 | Environmental sensor (T/H/P) | P10 |
| U11 | DS3231MZ+ | SOIC-8 | RTC (TCXO) | P14 |
| U12 | INA226AIDGSR | MSOP-10 | Current/voltage monitor | P13 |
| U13 | TPL5010DDCR | SOT-23-6 | Watchdog timer | P03 |
| U14 | BTS50015-1TAD | PG-TO263-7 | Pump high-side power switch | P11 |
| U15 | BTS50015-1TAD | PG-TO263-7 | Fan high-side power switch | P12 |
| U16 | USB-C (TYPE-C-31-M-12) | USB-SMD TYPE-C | USB-C receptacle (**VIOLATION**) | P15 |
| Q1 | SQJ411EP-T1_GE3 | POWERVDFN-8 | Reverse-polarity MOSFET | P01 |
| L1 | 2.2 µH | LQH44PN | Buck inductor | P02 |
| D1 | MBR0520L-TP | SOD-123 | Schottky diode | P01 |
| Card1 | SD-01A | SD-SMD | SD card socket (removable) | P14 |
| CN1 | 5557-2P | CONN-TH_HAA420A | **TH connector (VIOLATION)** | P15 |

### Verified / unverified / missing

| Category | Count |
|---|---|
| Components verified (placed + MPN + footprint known) | 110 |
| Components data-insufficient | 0 |
| Components missing (required by P01–P15 but not placed) | see REMAINING WORK |
| Components substituted | 0 |

---

## CONNECTIONS

| Category | Count / Status |
|---|---|
| Nets defined in PCB | 668 (667 named) |
| Tracks routed | **0** |
| Vias placed | **0** |
| Arcs | **0** |
| Floating pins (U1 RP2350B) | 83 (mostly unassigned GPIOs — supply pin status needs per-pin verification per STOP CONDITION 3) |
| Floating pins (U2 ESP32-S3) | 57 (strap/CHIP_PU/GPIO0 network needs verification per STOP CONDITION 4) |
| Floating pins (U6 AD7124-8) | **29** — includes critical supply/reference pins: AVDD, DGND, IOVDD, AVSS, REFIN+, REFIN-, REGCAPA, REGCAPD, EXP (exposed pad), CLK, PSW, REFOUT, SYNC, AIN0–AIN15 |
| Floating pins (U7 AD5689 DAC) | 12 |
| Floating pins (U11 DS3231 RTC) | 4 |
| Floating pins (U12 INA226) | 11 |
| Incorrect nets found | 0 (not yet audited at net level — routing not started) |
| Fixed nets | 0 (no fixes applied in this audit pass) |

---

## SAFETY

| Subsystem | Status | Detail |
|---|---|---|
| Watchdog | **PARTIAL** | U13 (TPL5010) placed. Need to verify: done-pin to RP2350B interrupt, reset-chain to ESP32-S3 CHIP_PU. |
| Thermal protection | **MISSING** | No hardware thermal comparator/interlock found. Currently relies on firmware thermal model only. P03 requires a hardware thermal comparator/interlock for safe-state. |
| Power protection | **PARTIAL** | Q1 (reverse-polarity MOSFET) + D1 (Schottky) present. No TVS diode or dedicated fuse/overcurrent protector found in inventory. |
| Safe state | **FIRMWARE ONLY** | RP2350B firmware `applyOutputs()` forces pump 100% on safety fault. No hardware safe-state latch (RP2350B must keep cooling if ESP32-S3 fails — hardware interlock not yet implemented). |

---

## EXTERNAL CONNECTIONS

| Category | Status | Detail |
|---|---|---|
| Direct-wire solder landings | **MISSING** | No solder-landing pads defined for external interfaces (pump, fan, sensor, power, RPM, ignition). P15 is not complete. |
| Current path status | **NOT VERIFIED** | No copper capacity / wire-size documentation for external landings. |
| Strain-relief status | **MISSING** | No strain-relief features defined. |
| Connector violations | **2** | (1) CN1 `CONN-TH_HAA420A` — through-hole connector on external interface. Violates: no-connector rule + SMD-first rule + direct-wire rule. (2) U16 `USB-SMD TYPE-C-16PIN` — USB-C receptacle connector on external interface. Violates: no-connector-footprint rule. |
| SD card socket | **1 (decision pending)** | Card1 `SD-SMD_SD-01A` — SMD socket, removable storage. Gate 8 requires an onboard/removable-storage decision. Socket is SMD but is still a detachable connector. |

---

## PROGRAMMING

| Category | Status | Detail |
|---|---|---|
| RP2350B SWD pads | **MISSING** | No SWD test pads (SWCLK, SWDIO, GND, 3V3) found in inventory. STOP CONDITION 5 not met. |
| ESP32-S3 UART/boot pads | **MISSING** | No UART0-TX/RX/GPIO0/CHIP_PU/EN test pads found. STOP CONDITION 5 not met. |

---

## CALIBRATION

| Category | Status | Detail |
|---|---|---|
| Calibration pads | **MISSING** | No ADC-reference, NTC-calibration, or current-sense test pads found. STOP CONDITION 5 not met. |
| Calibration storage | **PRESENT (firmware)** | NVS-based calibration persistence in firmware. No hardware EEPROM/calibration storage IC. |

---

## ERC

| Category | Count |
|---|---|
| Total ERC messages | 490 |
| **Errors** | **108** |
| **Warnings** | **382** |

### Error breakdown

| Rule | Count | Severity | Key offenders |
|---|---|---|---|
| `pin_not_connected` | 103 | Error | U1 (83), U6 (29 — incl. supply/ref pins), U2 (57), U4 (18), U7 (12), U12 (11), Card1 (11), U14/U15 (8 each), U10 (9), U13 (8), U11 (4), U9 (4) |
| `power_pin_not_driven` | 2 | Error | Power input pins not driven by a power source |
| `pin_not_driven` | 3 | Error | Output pins not driven |

### Warning breakdown

| Rule | Count | Severity |
|---|---|---|
| `pin_to_pin` | 175 | Warning (Unspecified ↔ Power input type conflicts) |
| `lib_symbol_mismatch` | 149 | Warning (GND/+3V3 power symbols don't match library copies) |
| `lib_symbol_issues` | 29 | Warning |
| `isolated_pin_label` | 15 | Warning |
| `multiple_net_names` | 7 | Warning |
| `unconnected_wire_endpoint` | 3 | Warning |
| `endpoint_off_grid` | 3 | Warning |
| `pin_not_connected` | 1 | Warning |

### Fixes applied (this session)

No ERC fixes were applied — this is a read-only audit. ERC errors must be resolved before Gate 9 can pass.

---

## PCB

| Category | Status | Detail |
|---|---|---|
| Layer status | **2-LAYER (VIOLATION)** | Only `F.Cu` and `B.Cu` defined. Master requirement is **4-layer** (needs In1.Cu/In2.Cu inner layers for power/ground/RF/analogue separation). |
| Board outline | 170 × 130 mm | **EXCEEDS** the <60 × 60 mm target. Outline appears to be provisional; must be resized after placement optimization. |
| Thickness | 1.6 mm | Standard; 4-layer stack re-specification needed. |
| High-current route status | **NOT STARTED** | 0 tracks, 0 vias placed. Pump/fan power paths (U14/U15 BTS50015) unrouted. |
| RF status | **NOT STARTED** | No RF matching or antenna routing. ESP32-S3 RF feedline not routed. |
| DRC status | **NOT RUN** | DRC requires completed routing; cannot run until Gate 10. |
| Footprints placed | 93 | Components are placed but no copper routing exists. |

---

## SCHEMATIC PAGE COVERAGE (P01–P15)

| Page | Title | Status | Key components present | Gaps |
|---|---|---|---|---|
| P01 | Power entry | PARTIAL | Q1 (MOSFET), D1 (Schottky) | No TVS, no dedicated fuse, no EMI filter |
| P02 | Power rails | PRESENT | U8 (buck), U9 (LDO), L1 (2.2 µH), C19–C26, R6/R7 (feedback) | — |
| P03 | Hardware safety | PARTIAL | U13 (TPL5010 watchdog), SW1 | No voltage supervisor, no thermal comparator/interlock, no hardware safe-state latch |
| P04 | RP2350B control | PRESENT | U1 (QFN-80) | 83 floating pins — supply-pin connectivity needs per-pin verification (STOP CONDITION 3) |
| P05 | RP2350B flash/clock/reset/debug | PRESENT | U3 (W25Q128), X1/X2 (12 MHz), C8/C9 (22 pF), R2/R3 (10 k) | SWD service pads missing |
| P06 | ESP32-S3 | PRESENT | U2 (QFN-56), X4 (40 MHz), C46/C47 (22 pF) | 57 floating pins — strap/CHIP_PU/GPIO0 needs verification (STOP CONDITION 4) |
| P07 | ESP32-S3 flash/PSRAM/RF | PRESENT | U4 (MT25QL01GB 1 Gbit), U5 (APS51208 512 Mbit BGA-24) | RF matching/antenna not in schematic |
| P08 | MCU communication | PARTIAL | Net "RP_SWCLK" found (debug bus) | No dedicated heartbeat/CRC/timeout hardware; ESP32-fail-must-not-stop-cooling hardware interlock missing |
| P09 | Precision ADC/DAC | PRESENT | U6 (AD7124-8), U7 (AD5689) | **CRITICAL**: U6 has 29 floating pins including AVDD/DGND/IOVDD/REFIN/AVSS/EXP — ADC is essentially unconnected |
| P10 | Sensor/RPM/OEM input | PARTIAL | U10 (BME280), R31/R32 (2.2 k NTC divider) | RPM conditioning, OEM fan request, ignition/key input missing |
| P11 | Pump channel | PRESENT | U14 (BTS50015-1TAD) | Feedback/protection circuitry needs verification |
| P12 | Fan channel | PRESENT | U15 (BTS50015-1TAD) | Feedback/protection circuitry needs verification |
| P13 | Current/voltage monitoring | PRESENT | U12 (INA226), R24 (0.01 Ω shunt) | INA226 has 11 floating pins — needs connection |
| P14 | RTC/SD/service | PRESENT | U11 (DS3231), Card1 (SD socket) | U11 has 4 floating pins; SD socket is removable (Gate 8 decision pending) |
| P15 | External wire landings | **FAIL** | — | CN1 (TH connector) + U16 (USB-C receptacle) are **connector violations**; no direct-wire solder landings defined |

---

## BUILD GATE STATUS

| Gate | Title | Status |
|---|---|---|
| Gate 0 | Recovery | PASS — project preserved, no reset performed |
| Gate 1 | RP2350B | PARTIAL — chip + flash + crystal placed; 83 floating pins; supply-pin audit needed |
| Gate 2 | ESP32-S3 | PARTIAL — bare SoC + flash + PSRAM placed; 57 floating pins; strap/boot audit needed |
| Gate 3 | MCU bus | PARTIAL — debug net present; no heartbeat/CRC/timeout/interlock hardware |
| Gate 4 | Analogue/sensors | **BLOCKED** — U6 ADC has 29 floating pins including critical supply/reference; U7 DAC has 12 floating |
| Gate 5 | Inputs | PARTIAL — BME280 + NTC present; RPM/OEM/ignition missing |
| Gate 6 | Outputs | PARTIAL — BTS50015 switches present; feedback/protection unverified |
| Gate 7 | Safety/power states | PARTIAL — watchdog placed; no hardware thermal comparator, no hardware safe-state latch |
| Gate 8 | Service/storage | FAIL — no programming/calibration test pads; SD socket decision pending; 2 connector violations |
| Gate 9 | Complete schematic | **FAIL** — 108 ERC errors unresolved; 2 connector violations; U6 ADC critically floating |
| Gate 10 | PCB | NOT STARTED — only placement exists; 2-layer (needs 4); outline 170×130 mm (needs <60×60) |
| Gate 11 | Final verification | NOT STARTED — blocked by Gate 9 + 10 |

---

## REMAINING WORK

Exact unresolved items, ordered by priority:

1. **U6 AD7124-8 supply/reference pins** — connect AVDD, DGND, IOVDD, AVSS, REFIN+, REFIN-, REGCAPA, REGCAPD, EXP (29 floating pins total). Without this the precision ADC is non-functional. **Blocks Gate 4.**

2. **U1 RP2350B supply-pin audit** — verify every VREG_*/DVDD/IOVDD pin is connected with correct decoupling (STOP CONDITION 3). 83 floating pins need classification: which are supply (must connect) vs GPIO (may float). **Blocks Gate 1.**

3. **U2 ESP32-S3 strap/boot network** — verify CHIP_PU reset network, GPIO0 download-mode control, all VDD/VDD_SPI pins (STOP CONDITION 4). 57 floating pins. **Blocks Gate 2.**

4. **U7 AD5689 DAC** — connect 12 floating pins (VDD, VREF, DAC outputs, GND). **Blocks Gate 4.**

5. **U12 INA226** — connect 11 floating pins (VDD, SDA, SCL, ALERT, VBUS, VIN+, VIN-, GND). **Blocks Gate 6.**

6. **U11 DS3231 RTC** — connect 4 floating pins (VCC, VBAT, SDA, SCL, 32kHz). **Blocks Gate 8.**

7. **Remove connector violations** — remove CN1 (TH connector) and U16 (USB-C receptacle). Replace with direct-wire solder landings per P15 spec. **Blocks Gate 8 + Gate 9.**

8. **Add programming pads** — RP2350B SWD (SWCLK, SWDIO, GND, 3V3) + ESP32-S3 UART/boot (TX, RX, GPIO0, CHIP_PU, EN, GND). No detachable connector. STOP CONDITION 5. **Blocks Gate 8.**

9. **Add calibration pads** — ADC reference, NTC calibration, current-sense test points. **Blocks Gate 8.**

10. **Complete P03 hardware safety** — add voltage supervisor IC, hardware thermal comparator/interlock, hardware safe-state latch (RP2350B keeps cooling if ESP32-S3 dies). **Blocks Gate 7.**

11. **Complete P10 inputs** — RPM conditioning, OEM fan request, ignition/key input circuits. **Blocks Gate 5.**

12. **Complete P08 MCU bus** — dedicated RP2350B↔ESP32-S3 bus with heartbeat, CRC, timeout, and reset/status hardware. **Blocks Gate 3.**

13. **Resolve all 108 ERC errors** — after fixing items 1–12, re-run ERC until 0 errors. **Blocks Gate 9.**

14. **PCB layer stack** — change from 2-layer to 4-layer (add In1.Cu power, In2.Cu ground). Resize outline to <60×60 mm. **Blocks Gate 10.**

15. **SD card socket decision** — Gate 8 requires explicit onboard-vs-removable decision for Card1. **Blocks Gate 8.**

---

## FIRMWARE GATE STATUS (for reference)

| Gate | Script | Result |
|---|---|---|
| Static release gate | `tools/verify_release.py` | **PASS** `{"release_static_gate":"ok"}` |
| Control-math model | `tools/verify_control_math.py` | **PASS** (steady quality 94.7%, jitter 2%, sensor-fail immediate 100%, critical-fail immediate 100%) |
| CRC16-CCITT codec | Python mirror of `ewp_link_protocol.cpp` | **PASS** (vector 0x29B1, round-trip OK, corruption rejected) |
| CRC32 per-record | Python mirror of `data_logger.cpp` | **PASS** (vector 0xCBF43926, matches zlib, corruption detected) |

### Firmware components completed this session

| Component | File | Status |
|---|---|---|
| Frame codec + LinkSupervisor | `components/smartcooling/src/ewp_link_protocol.cpp` | **COMPLETE** — transport-neutral, CRC16-CCITT, heartbeat/sequence/timeout |
| RobustDataLogger | `components/smartcooling/src/data_logger.cpp` | **COMPLETE** — triple buffer, SD guarded by `SC_FACTORY_SD_CARD_ENABLED`, 5 MB auto-rotate, CRC32 per record, non-blocking |
| SI core integration | `main/main.cpp` | **COMPLETE** — `runSmartCoolingEnrichment()` called after `applyOutputs()` (safety path untouched) |
| Sentinel integration | `main/main.cpp` | **COMPLETE** — `Sentinel.begin()` + `verifyMemoryIntegrity()` in tick |
| DataLogger integration | `main/main.cpp` | **COMPLETE** — `DataLogger.begin()` + `logData()`/`flush()` in tick |
| statusToJson diagnostics | `main/main.cpp` | **COMPLETE** — `si_core`, `sentinel`, `logger` blocks added |
| CMake registration | `components/smartcooling/CMakeLists.txt` | **COMPLETE** — all 4 `.cpp` sources registered |
| Release gate strengthened | `tools/verify_release.py` | **COMPLETE** — added new files + integration assertions + CMake source checks; `SC_FACTORY_PROFILE` expected string corrected |

---

## NEXT ACTION

Fix U6 AD7124-8 floating supply/reference pins (item 1 above) — this is the single most critical schematic defect. The precision ADC cannot function with AVDD, DGND, IOVDD, AVSS, REFIN, and EXP all floating. After U6, proceed to U1 supply-pin audit (STOP CONDITION 3) and U2 strap/boot audit (STOP CONDITION 4), then resolve all 108 ERC errors before re-running the audit.

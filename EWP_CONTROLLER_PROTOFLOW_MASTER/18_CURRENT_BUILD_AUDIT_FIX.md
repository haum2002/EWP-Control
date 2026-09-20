# CURRENT BUILD AUDIT / MANDATORY FIXES

This file applies immediately to the current ProtoFlow build after the reported result:
- RP2350B power + crystal wiring added
- ESP32-S3 power/reset/boot wiring added
- many components/wires now present
- previous build was paused due to catalog/CAD availability

## STOP CONDITION 1 — verify U2 is a BARE ESP32-S3
The build report described U2 as an "ESP32-S3 module".
This conflicts with the master requirement.

Required action:
- If U2 is a module/package containing ESP32-S3, REMOVE it.
- Replace it with the selected bare ESP32-S3 SoC exact MPN.
- Rebuild only the required supporting circuitry around the bare chip.
- Do not add WROOM/WROVER/XIAO/DevKit or any other module.

Do not continue to PCB finalization until this is verified.

## STOP CONDITION 2 — audit RP2350 regulator pin naming
The build report used the phrase "U1.VREG_VOUT".
The official RP2350 pin set uses VREG_VIN, VREG_FB, VREG_LX,
VREG_PGND and VREG_AVDD; DVDD is the 1.1V digital core supply.
There is no official RP2350 package pin named VREG_VOUT in the cited pinout.

Required action:
- Inspect the actual U1 symbol pin names and pin numbers.
- Do not accept a net label or alias as evidence that a physical pin named
  VREG_VOUT exists.
- Ensure the internal switching-regulator network follows the official RP2350
  hardware design guidance.
- VREG_LX must connect to the external inductor.
- VREG_FB must sense the filtered regulator output as specified.
- VREG_PGND must return to ground with the high di/dt regulator current kept local.
- VREG_AVDD must be treated as a sensitive regulator analog supply.
- DVDD must be supplied at the required core voltage.

Do not guess component values; use the official design guidance/datasheet plus the
selected inductor/capacitor datasheets.

## STOP CONDITION 3 — verify U1 decoupling completeness
Do not accept "C1-C7 are connected" as proof of correctness.
For every U1 supply pin:
- identify exact pin number/name
- identify the capacitor connected to it
- verify voltage domain
- verify ground return
- verify physical placement intent
- verify no supply pin is accidentally left floating

## STOP CONDITION 4 — verify ESP32-S3 bare-chip power and boot network
Audit every VDD/VDD_SPI/CHIP_PU/GPIO0 connection against the exact bare-chip MPN
and Espressif hardware-design guidance.
Do not accept a module pinout for a bare-chip design.

GPIO0 must remain controllable for UART download mode, and CHIP_PU must have a defined
reset/enable network.

## STOP CONDITION 5 — add programming/calibration test pads
Before continuing to PCB routing, add the pad groups defined in:
17_PROGRAMMING_CALIBRATION_RULES.md

No detachable connector is allowed.

## STOP CONDITION 6 — do not hide incompleteness behind a component count
A component count such as 25+ does not prove completeness.
ProtoFlow must produce a component audit:
RefDes | Function | MPN | Package | Value/Rating | Source | Verification

Then compare the complete audit against P01-P15 and list missing components/nets.

## Required execution sequence
1. Audit current schematic.
2. Fix U2 bare-chip violation if present.
3. Fix any incorrect RP2350 pin/network naming or topology.
4. Complete power/reset/clock support circuits.
5. Add programming/calibration test pads.
6. Continue all remaining schematic pages.
7. Run ERC.
8. Fix all required ERC failures.
9. Re-run ERC.
10. Only after Gate 9 passes, proceed to PCB placement/routing.

If one catalog part becomes unavailable, use the verified SMD substitution policy and
continue independent work rather than stopping the entire build.

## Report format after this task
STATUS: CONTINUE / BLOCKED
COMPONENTS VERIFIED: <count>
COMPONENTS DATA_INSUFFICIENT: <count>
NETS VERIFIED: <count>
ERC ERRORS: <count>
ERC WARNINGS: <count>
REMAINING PAGES: <list>
PROGRAMMING PADS: VERIFIED / MISSING
CALIBRATION PADS: VERIFIED / MISSING
CONNECTOR VIOLATIONS: <count>
MODULE VIOLATIONS: <count>
FIXES PERFORMED: <list>
NEXT ACTION: <one concrete action>

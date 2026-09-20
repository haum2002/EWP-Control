# PROTOFLOW / EASYEDA WORKFLOW

EasyEDA Pro supports multiple schematic pages in one board and can transfer multi-page schematics to the associated PCB. Keep the schematic pages and PCB under the same board.

## Recommended board structure
EWP_CONTROLLER_4L / EWP_CTRL_REV_A / P01...P15 / PCB1

## Agent rules
- recover existing project before creating a new board
- preserve verified pages
- do not erase verified work unnecessarily
- create actual circuits, not empty functional boxes
- do not route PCB before schematic Gate 9 passes
- do not use catalog convenience as an architecture decision

## If a folder/project disappears
1. Check project/board tree.
2. Check pages under the existing board.
3. Check library/project files available to ProtoFlow.
4. Only then rebuild missing pages.
5. Reapply this package as the design contract.

## After every change
1. inspect changed nets
2. check pin connectivity
3. run the relevant ERC/rule check
4. compare against the requirement checklist
5. fix required failures
6. only then proceed

## Export for review
Produce multi-page schematic PDF, PCB PDF, BOM and verification report when available. Exported images are evidence, not the source design.

## Direct-wire interface rule
Do not place connector footprints while routing external interfaces. Replace any
proposed battery/pump/fan/sensor/RPM/service connector with a dedicated direct-wire
solder landing. Use a plated wire-entry hole/slot only when mechanically useful; it is
a PCB feature, not a connector.

For every power landing, ProtoFlow must calculate/record the wire size, current case,
PCB copper thickness, copper width/area, layer transition/via capacity, temperature-rise
check, voltage-drop check and strain-relief method.

The board outline target is less than 60 mm x 60 mm. Do not increase dimensions simply
to make wire connections easier.

## Programming / calibration checkpoint
Before PCB routing, verify the product has no detachable programming/service connectors.
Reserve compact exposed test pads for RP2350 SWD and ESP32-S3 UART0 download/reset, and
calibration test points required by the finalized measurement chain. Use a temporary
external production fixture for programming/calibration. See
17_PROGRAMMING_CALIBRATION_RULES.md.

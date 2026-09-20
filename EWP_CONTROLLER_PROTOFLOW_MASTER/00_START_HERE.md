# EWP CONTROLLER — PROTOFLOW MASTER PACKAGE

This package is the authoritative working contract for the compact universal EWP + radiator-fan controller PCB.

## NON-NEGOTIABLE GOAL
Build a real production-oriented electrical design from bare ICs and SMD components on a compact 4-layer PCB. Do not build a development board, module carrier, or visual placeholder schematic.

## READ ORDER
1. 01_MASTER_REQUIREMENTS.md
2. 02_ARCHITECTURE.md
3. 03_SMD_4LAYER_RULES.md
4. 04_STRICT_PROHIBITIONS.md
5. 05_BUILD_GATES.md
6. 06_VERIFICATION_CHECKLIST.md
7. 07_PROTOFLOW_WORKFLOW.md
8. 08_COMPONENT_DATA_RULES.md
9. 09_SCHEMATIC_PAGE_PLAN.md
10. 10_AGENT_RESPONSE_TEMPLATE.md
11. 16_DIRECT_WIRE_SOLDER_CONNECTION_RULES.md

## GOLDEN RULE
PLAN -> BUILD -> CHECK -> FIX -> RECHECK -> ONLY THEN COMPLETE.

A task is NOT complete while a required ERC/DRC error, floating required pin, missing subsystem, guessed connection, unverified footprint, unauthorized module, or other required defect remains.


## IMPORTANT: Catalog/CAD failure does not automatically stop the build
If ProtoFlow reports `CAD unavailable`, `rate_limited`, `download pending`, `not found`, or similar, use `15_COMPONENT_SUBSTITUTION_POLICY.md`. A verified SMD equivalent may be selected so the build can continue.

## IMPORTANT: No external connectors
All external field wiring is direct-soldered to the PCB. Do not add connector
footprints, terminal blocks, headers, receptacles or sockets for battery, pump,
fan, ignition, sensor, RPM or other external wiring. Use dedicated PCB solder
landings and documented strain relief. See `16_DIRECT_WIRE_SOLDER_CONNECTION_RULES.md`.

## v1.3 immediate addition
Programming and calibration are performed through exposed PCB test pads using a
temporary external fixture. No USB/service connector footprint is allowed. Read
17_PROGRAMMING_CALIBRATION_RULES.md and 18_CURRENT_BUILD_AUDIT_FIX.md before continuing
a build that reports an ESP32-S3 "module" or any ambiguous RP2350 regulator pin name.

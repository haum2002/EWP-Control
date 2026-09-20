# BUILD GATES

Every work unit must follow:
PLAN -> IMPLEMENT -> SELF-CHECK -> ERC/RULE CHECK -> REQUIREMENT CHECK -> FIX -> RECHECK -> COMPLETE

## Gate 0 — Recovery
Confirm project/board/pages and preserve verified work.

## Gate 1 — RP2350B
Complete actual power, regulator, clock, reset, debug and boot/QSPI circuitry. No required floating pins.

## Gate 2 — ESP32-S3
Bare SoC, power, strap/reset, crystal, flash, PSRAM and RF. Verify memory capacity and topology.

## Gate 3 — MCU bus
Actual bus + heartbeat + CRC + timeout + reset/status. ESP32 failure must not stop RP2350 cooling.

## Gate 4 — Analogue/sensors
ADC, reference, DAC where required, coolant sensor, environmental sensor, RTC.

## Gate 5 — Inputs
RPM, OEM fan request, ignition/key, protection and conditioning.

## Gate 6 — Outputs
Pump/fan 2/3/4-wire support, power stages, feedback, current/voltage sensing.

## Gate 7 — Safety/power states
Supervisor/watchdog, thermal safety, after-run support, low-battery hard disconnect.

## Gate 8 — Service/storage
Onboard/removable-storage decision, service/debug pads without detachable connectors, direct-wire interface audit.

## Gate 9 — Complete schematic
All pages, exact parts, footprints, ERC, no unresolved required errors.

## Gate 10 — PCB
Only after Gate 9: placement, 4-layer stack, power/ground/RF/analogue/high-current routing, direct-wire solder landings, wire-clearance and strain-relief features, and <60 x <60 mm dimensional target.

## Gate 11 — Final verification
DRC, unconnected check, schematic-vs-PCB consistency, thermal, RF, analogue, footprint and manufacturability review.

## Failure behavior
If any required check fails: STOP -> IDENTIFY -> FIX -> RECHECK. Do not declare complete. If a fix touches a verified subsystem, re-check that subsystem and its interfaces.


## Catalog failure handling gate
A catalog/CAD failure is not itself a build-gate failure. The gate fails only when no verified SMD alternative can satisfy the requirement.

## Direct-wire build gate
No connector footprint is allowed on an external interface. Each external wire landing
must have verified copper capacity, spacing, wire-size documentation and a mechanical
strain-relief plan before Gate 10 can pass.

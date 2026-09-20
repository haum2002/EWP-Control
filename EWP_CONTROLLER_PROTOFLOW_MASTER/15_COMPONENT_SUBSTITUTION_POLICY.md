# COMPONENT SUBSTITUTION POLICY — PROTOFLOW

## Purpose
Prevent the build from stopping simply because one catalog part, CAD file, or library download is temporarily unavailable. The design may use an alternative component, but only when the alternative is proven compatible and remains SMD/compact.

## HARD RULE
If a required component cannot be downloaded from the catalog, the agent MUST evaluate a verified SMD substitute before pausing the build.

A catalog failure is a supply/library problem, not automatically an engineering blocker.

## Allowed substitution
The substitute may be from another manufacturer or a different MPN. It must preserve the required function and must not reduce the required electrical, thermal, timing, safety, RF, measurement, or reliability performance.

Minimum checks:
- same intended function and circuit topology
- compatible voltage/current/power ratings
- adequate temperature rating
- compatible signal levels and interface
- compatible timing/frequency/bandwidth where relevant
- acceptable tolerance/accuracy/ESR/ESL where relevant
- correct pinout/polarity/exposed pad
- verified SMD package and land pattern
- authoritative manufacturer datasheet available

## Priority for selecting substitutes
1. Exact MPN from another verified source.
2. Same electrical specification, different manufacturer.
3. Slightly different component value/rating only when calculation/datasheet proves it remains inside the design window.
4. Alternate SMD package only when thermal, mechanical, pinout and footprint are verified.

## Parts that require special treatment
### RF / ANT1
Do not replace an unavailable antenna with a random antenna. The substitute must be compatible with the required RF band, impedance/interface, efficiency/ground requirements, tuning network, placement and keepout. The ESP32-S3 RF layout must follow Espressif guidance.

### ESP32-S3 external memory
A substitute flash/PSRAM device must be compatible with the selected ESP32-S3 interface mode, supply voltage, density, timing and supported topology. Capacity units must be written explicitly as bits vs bytes.

### Precision analog
For ADC/DAC/reference/current-sense devices, compare actual performance requirements, not just nominal bit count. Verify reference, input/output range, common-mode, accuracy/ENOB, bandwidth and calibration impact.

### Power and protection
For MOSFETs, regulators, shunts, TVS and protection devices, verify voltage margin, continuous/pulse current, dissipation, transient response and temperature behavior.

## SMD HARD REQUIREMENT
All substitute parts must be SMD and suitable for the compact 4-layer board. Do not replace an unavailable SMD part with a through-hole part, module, development board, breakout board, or oversized package.

## Do not overreach
Never invent a substitute specification. Never guess a pinout, polarity, sensor characteristic, pump protocol, fan protocol, antenna characteristic, memory topology, or PWM requirement. When authoritative data is insufficient, use `DATA_INSUFFICIENT` for that item and continue independent work.

## Build continuation rule
When a compliant substitute is found:
1. replace the unavailable catalog part;
2. update schematic/BOM;
3. verify pin mapping and footprint;
4. rerun ERC for the affected page/block;
5. re-check all relevant design gates;
6. continue to the next build unit.

Do not pause the entire project just to wait for a CAD download retry when a validated SMD substitute exists.

## Mandatory component audit
For every placed component, ProtoFlow must be able to report:
`RefDes | Function | Original requested part | Actual placed MPN | Package | Key specification | Source | Footprint | Verification`

The component is not considered complete merely because a symbol exists.

## Completion rule
The project may be marked COMPLETE only when all required functions have verified parts, verified footprints, valid connectivity, and required ERC/DRC/build checks have passed or have documented non-critical warnings.

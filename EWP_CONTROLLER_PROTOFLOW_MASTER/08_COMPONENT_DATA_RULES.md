# COMPONENT + DATA RULES

## Selection order
Requirement -> official datasheet/hardware guide -> electrical topology -> exact MPN -> package -> verified footprint -> ECAD library/CAD.

Never: EasyEDA catalog -> convenient part -> invent circuit around it.

## Critical component record
RefDes; function; manufacturer; exact MPN; package; voltage; current; temperature; relevant parameters; source; footprint source; verification status.

## Never guess
Do not guess pinout, polarity, voltage, current, resistor/capacitor/inductor values, sensor curves, OEM polarity, pump protocol, RF matching or memory topology.

## DATA_INSUFFICIENT format
DATA_INSUFFICIENT
Item:
Unknown:
Why it matters:
Authoritative source checked:
What is required:
Can independent work continue: YES/NO

## CAD unavailable / catalog rate-limit fallback
A missing, rate-limited, temporarily unavailable, or CAD-unavailable catalog part MUST NOT automatically pause the whole build when an electrically and mechanically suitable SMD substitute can be verified.

Use this substitution order:
1. Same manufacturer + same MPN from another verified library/source.
2. Different manufacturer, same function and same electrical/interface specification.
3. Different value/rating within the required engineering tolerance, only when the circuit calculation/datasheet proves compatibility.
4. Different package only when it remains SMD, pin-compatible or properly remapped, thermally/electrically adequate, and the verified land pattern is available.

For every substitute, record BOTH:
- ORIGINAL_REQUESTED_PART: the initially selected/requested MPN or catalog part.
- ACTUAL_PLACED_PART: the verified substitute MPN actually used in the schematic/BOM.

Substitution is allowed only after checking at minimum:
- function/topology compatibility
- voltage rating
- current rating
- power dissipation
- temperature range
- tolerance/accuracy where applicable
- logic thresholds / interface standard where applicable
- frequency, timing, bandwidth, ESR/ESL, capacitance/inductance, or other relevant dynamic parameters
- protection/safety rating where applicable
- package and verified SMD footprint
- pinout and pin 1/polarity/exposed-pad requirements
- availability of authoritative datasheet/manufacturer data

For critical parts, apply stricter checks:
- ESP32-S3 RF/antenna/matching parts: band, impedance, RF performance, matching network, keepout and layout compatibility must be verified.
- External flash/PSRAM: supported voltage, interface mode, density, timing and ESP32-S3 compatibility must be verified against Espressif guidance/datasheet.
- Precision ADC/DAC/reference/current-sense components: resolution is not enough; reference, ENOB/accuracy, input range, common-mode, bandwidth and calibration implications must be checked.
- Power MOSFETs, shunts, regulators, TVS and protection parts: continuous/pulse current, voltage margin, thermal dissipation and transient behavior must be checked.
- Sensors: electrical interface and supported operating range must match; do not invent sensor curves.

DO NOT substitute a component merely because it exists in the catalog. Do not redesign around an unverified part just to clear a catalog/CAD error.

If no verified SMD equivalent can be established, mark only the affected item/block as DATA_INSUFFICIENT, continue all independent blocks, and report exactly what information is missing.

## Mandatory build behavior for catalog/CAD failures
When a catalog part reports errors such as `CAD unavailable`, `rate_limited`, `download pending`, `not found`, or similar:
- First try a verified equivalent SMD part.
- If equivalent is validated, replace the part and CONTINUE the build.
- Do not wait for the retry timestamp if a compliant substitute is already available.
- Do not leave the schematic with an empty component placeholder for a required function.
- Do not claim completion until the substitute, footprint, connectivity and ERC/DRC status are verified.

## BOM transparency
The agent MUST maintain a component audit list for every placed component with at least:
RefDes | Function | Original requested part | Actual placed MPN | Package | Key rating/spec | Datasheet/source | Footprint status | Verification status | Substitute reason (if any)

This is required so the user can identify what the 25+ placed components actually are. Unknown numbered components without function/MPN/package/source are NOT considered verified components.

## Memory
capacity_bits / 8 = bytes. Explicitly distinguish Mb (megabits) from MB (megabytes).

## Footprints
Verify pin numbering, pad dimensions, exposed pad, courtyard, polarity, package outline and manufacturer land pattern where available.

## External wire landings are PCB features, not components
External direct-wire solder landings do not require a connector MPN. They must instead
be documented by PCB land geometry, copper thickness, net, current case, wire gauge,
spacing and mechanical retention/strain relief.

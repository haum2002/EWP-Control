# EWP CONTROLLER — MASTER AI AGENT INSTRUCTION
## Authoritative Project Context, Requirements, Current State, Audit Rules and Build Procedure
### Version 1.4 — Single-file master reference

> THIS FILE IS THE SINGLE AUTHORITATIVE CONTEXT FOR THE PROJECT.
> READ IT COMPLETELY BEFORE CHANGING THE CURRENT DESIGN.
> PRESERVE VALID WORK ALREADY DONE. DO NOT RESET THE PROJECT FROM SCRATCH.
>
> Core workflow:
> **PLAN → BUILD → CHECK → FIX → RECHECK → ONLY THEN COMPLETE**

---

# 1. PROJECT IDENTITY

Project: **Universal EWP + Radiator Fan Controller**

Target application:
- 12 V motorcycle / vehicle electrical system
- Universal architecture, not redesigned for each vehicle
- Particularly intended to support motorcycle applications such as Suzuki Raider R150 FI / Satria FUFI
- Production-oriented custom PCB, not a development prototype

Hard targets:
- **4-layer PCB**
- **Bare ICs / bare SoCs**
- **SMD-first / SMD-only design**
- **No development boards**
- **No modules**
- **No external connector sockets for field wiring**
- Target final PCB size: **less than 60 mm × 60 mm**
- Small size is important, but safety, current capacity, thermal performance, RF layout, electrical integrity and reliability have priority over arbitrary miniaturization
- Every required function must have a real implemented circuit
- No empty functional blocks
- No placeholder circuitry for required functions
- No “complete” status while required errors, missing nets, missing components or unverified requirements remain

---

# 2. NON-NEGOTIABLE DESIGN PRINCIPLES

## 2.1 Bare-chip rule

Required:
- RP2350B bare IC
- ESP32-S3 bare SoC

Forbidden:
- ESP32-S3-WROOM
- ESP32-S3-WROVER
- XIAO ESP32-S3
- DevKit
- breakout board
- carrier board
- any other MCU module
- any development board

If a library part is named “module” but the actual MPN is a bare SoC/IC, verify the actual MPN, package and datasheet before accepting it.

The reverse also applies:
if the library part is physically a module, it is forbidden even if the logical symbol is ESP32-S3.

---

# 3. MCU ARCHITECTURE

## 3.1 U1 — RP2350B

RP2350B is the primary real-time, safety and actuator-control MCU.

It owns critical functions:

- EWP/pump control
- radiator fan control
- coolant temperature processing
- RPM capture
- OEM ECU fan-demand input monitoring
- ignition/key state sensing
- voltage measurement
- current measurement
- power measurement
- PWM generation
- actuator feedback
- watchdog/state supervision
- thermal control
- after-run control
- low-battery shutdown logic
- protection/fallback state machine
- safe cooling behavior
- fault handling
- fault state
- communication watchdog with ESP32-S3
- critical configuration enforcement

ESP32-S3 failure must NOT disable critical cooling protection.

RP2350B must remain capable of safely controlling pump/fan if:
- ESP32-S3 reboots
- ESP32-S3 hangs
- Wi-Fi fails
- WebApp fails
- OTA UI layer fails
- SD logging fails
- high-level software crashes

## 3.2 U2 — ESP32-S3

ESP32-S3 is the UI/connectivity/high-level processor.

Responsibilities:
- WebApp
- Wi-Fi
- BLE
- configuration UI
- diagnostics UI
- high-level logging
- OTA orchestration
- data visualization
- AI/ML support
- non-critical data processing
- high-level communication with RP2350B

ESP32-S3 must NOT be the sole safety-critical actuator controller.

---

# 4. MCU-TO-MCU COMMUNICATION

A robust RP2350B ↔ ESP32-S3 control link is required.

The protocol must support at least:
- framed messages
- message type
- sequence counter
- length
- payload
- CRC/integrity
- acknowledgement where required
- heartbeat
- timeout detection
- state synchronization
- fault reporting
- firmware/version identity
- configuration version
- safe fallback

If ESP32-S3 becomes unavailable:
- RP2350B continues autonomous critical operation
- communication timeout must not disable cooling
- last safe valid command or safe fallback policy must be enforced

Do not invent a transport or pin assignment unless the current schematic and verified RP2350B/ESP32-S3 pin availability establish it.

---

# 5. POWER ARCHITECTURE

Expected conceptual chain:

BATTERY 12 V
→ fuse/protection
→ reverse-polarity protection
→ TVS / automotive transient protection
→ EMI filtering
→ protected battery bus
→ controlled power switching
→ regulated rails

Required power-state architecture:

## STATE 1 — KEY OFF / ENGINE OFF
After-run operation may continue according to configured shutdown/thermal logic.

Possible functions:
- coolant temperature monitoring
- pump operation
- radiator fan operation
- post-run cooling
- controlled logging/shutdown

## STATE 2 — KEY ON / ENGINE ON OR OFF
Full normal operation.

Engine ON/OFF is inferred from RPM.

KEY ON + RPM = 0 is an engine-OFF substate, not a fourth primary mode.

## STATE 3 — LOW BATTERY / KEY OFF
Controller load must be disconnected from the battery.

Target:
- zero normal operating load
- zero intended controller operation
- only unavoidable physical leakage current from protection/switching components may remain

Do NOT claim mathematical zero leakage unless verified from the selected hardware.

RTC must use a dedicated RTC/backup domain.
Do NOT use a general backup battery to keep the entire controller powered.

---

# 6. HARDWARE SAFETY LAYER

Safety cannot depend entirely on firmware.

Provide, as appropriate for the verified circuit:
- independent watchdog/supervisor
- reset supervision
- thermal comparator or hardware over-temperature protection
- hardware output enforcement/interlock
- safe-state control
- protection against MCU lockup
- protection against abnormal output state

Required intent:
1. If RP2350B firmware hangs, supervisor/watchdog resets it.
2. If an MCU/software fault cannot be recovered normally, hardware safety must force an appropriate safe/emergency cooling state where the actual hardware architecture permits it.
3. ESP32-S3 must never be a single point of failure for thermal protection.

Do not invent a specific comparator threshold or hardware output polarity without verified electrical requirements.

---

# 7. RP2350B POWER / CLOCK / RESET AUDIT

Previous ProtoFlow report claimed:
- RP2350B power distribution connected
- 12 MHz crystal connected
- decoupling capacitors added
- RUN pulled high
- GND connected
- regulator connections made

IMPORTANT:
Do NOT trust the previous textual report as proof of correctness.

ProtoFlow previously stated:
- `U1.VREG_VIN → 3V3`
- `U1.VREG_VOUT → 1V1`
- `U1.VREG_LX → L2`

This MUST be audited against the actual RP2350B symbol pin names, pin numbers and official regulator topology.

The RP2350 documentation identifies:
- VREG_VIN
- VREG_FB
- VREG_LX
- VREG_PGND
- VREG_AVDD
- DVDD

The official design uses the internal switching regulator to generate the 1.1 V core supply and uses external inductor/capacitor/feedback circuitry.

Therefore:
- never silently accept a pin named `VREG_VOUT` without checking the actual part
- verify VREG_FB
- verify VREG_PGND
- verify DVDD
- verify VREG_AVDD
- verify regulator inductor
- verify regulator output capacitor
- verify input capacitor
- verify decoupling
- verify power return path
- verify placement/routing intent

Official RP2350 references:
- https://datasheets.raspberrypi.com/rp2350/rp2350-datasheet.pdf
- https://datasheets.raspberrypi.com/rp2350/hardware-design-with-rp2350.pdf

---

# 8. ESP32-S3 HARDWARE AUDIT

U2 MUST be the bare ESP32-S3 SoC.

Required audit:
- all VDD pins
- all required grounds
- exposed/thermal pad handling if applicable to exact package
- CHIP_PU/reset network
- GPIO0 boot strap
- required strapping pins
- crystal
- external flash
- external PSRAM
- RF section
- RF matching
- antenna
- required decoupling
- SPI routing requirements
- test/programming pads

ESP32-S3 default UART0 download pins:
- GPIO43 = U0TXD
- GPIO44 = U0RXD

UART0 may be used for firmware download and log/service.
Do not assume the default pins are free until all memory/RF/other allocations are audited.

IMPORTANT MEMORY EFFECT:
Depending on flash/PSRAM interface and configuration, GPIO33–GPIO37 can be occupied.
Verify actual selected memory topology before assigning these pins to anything else.

Official references:
- https://docs.espressif.com/projects/esp-hardware-design-guidelines/en/latest/esp32s3/schematic-checklist.html
- https://docs.espressif.com/projects/esp-hardware-design-guidelines/en/latest/esp32s3/pcb-layout-design.html
- https://docs.espressif.com/projects/esp-hardware-design-guidelines/en/latest/esp32s3/download-guidelines.html

---

# 9. 4-LAYER PCB STACKUP

Preferred structure:

L1 / TOP:
- components
- critical signals
- RF
- short local connections

L2:
- continuous GND plane
- avoid routing normal signals here unless a verified exception is required

L3:
- power
- selected signals
- preserve GND isolation beneath RF/crystal-sensitive areas as required

L4 / BOTTOM:
- selected signals
- no unnecessary components if the layout permits

ESP32-S3 RF area must receive special treatment:
- antenna at/near board edge as appropriate
- RF path short
- controlled impedance where required
- antenna keepout
- no inappropriate copper/components in keepout
- RF matching parts close to RF path
- no casual vias in RF path

Follow current Espressif hardware design guidance rather than automatic generic PCB rules.

Official reference:
https://docs.espressif.com/projects/esp-hardware-design-guidelines/en/latest/esp32s3/pcb-layout-design.html

---

# 10. NO CONNECTOR POLICY — FINAL

This is an explicit final project requirement.

External field wires shall be soldered directly to PCB dedicated wire-landing areas.

FORBIDDEN:
- JST
- Molex
- Hirose
- terminal blocks
- screw terminals
- pin headers
- receptacles
- wire-to-board connectors
- plugs
- sockets
- USB receptacle
- SWD header
- UART header
- any connector footprint for external field wiring

Required direct-wire targets include, as applicable:
- Battery +
- Battery GND
- ignition/key
- EWP/pump
- radiator fan
- coolant temperature sensor
- RPM input
- OEM ECU fan-demand signal
- external/environmental sensor wiring
- other external signals

A direct-wire PCB solder landing is NOT a connector.

Use dedicated solder pads / copper landing geometry designed for the actual wire.

---

# 11. DIRECT-WIRE LANDING DESIGN

Do not replace connectors with ordinary tiny component pads.

For each external wire define:
- net name
- function
- expected continuous current
- expected peak/inrush current if known
- wire size
- copper thickness
- copper width/area
- landing geometry
- thermal path
- voltage drop
- clearance
- creepage as applicable
- mechanical strain relief
- test access

High-current direct-wire regions:
- battery
- pump
- fan

must be designed as actual power paths, not ordinary signal traces.

Use wide copper pours/traces and short paths as required.
Use adequate via arrays if current must change layers.
Use Kelvin sensing for current shunts where applicable.

Do not claim a PCB current capacity without calculating or verifying:
- copper thickness
- copper width
- path length
- ambient/environment
- allowed temperature rise
- component limits
- via capacity
- solder landing capacity

---

# 12. WIRE STRAIN RELIEF

Direct solder removes loose connector failure points, but wire movement can damage pads/solder joints.

Therefore provide PCB-level mechanical strain relief where practical:
- anchor hole/slot
- wire restraint feature
- mechanical tie point
- controlled wire entry path

Do not make the solder joint the only structure carrying cable tension.

The mechanical feature must remain compatible with the compact board target.

---

# 13. EWP / PUMP INTERFACE

Same PCB must support universal:
- 2-wire
- 3-wire
- 4-wire

The output architecture should be capable of, where technically appropriate:
- high-side power switching
- optional low-side switching where required
- ON/OFF
- PWM
- configurable control signal
- feedback/tach input
- voltage/current monitoring

Conceptual 4-wire mapping:
- +12 V
- GND
- CONTROL
- FEEDBACK

2-wire equipment:
- +12 V
- GND

Do not assume a 2-wire Bosch EWP has a conventional external PWM control protocol.

For a 2-wire pump, any PWM/burst-power modulation must be characterized from the actual pump and selected switching hardware.

Do NOT invent:
- PWM frequency
- control polarity
- feedback protocol
- speed-control protocol

without evidence.

The high-current path must use a proper dedicated power switching stage.
Never drive motor current directly through MCU GPIO.

---

# 14. RADIATOR FAN INTERFACE

Same universal philosophy:
- 2-wire
- 3-wire
- 4-wire where applicable

Provide appropriate:
- high-current power path
- control signal driver where required
- feedback input where available
- PWM capability where compatible
- ON/OFF capability
- current measurement
- voltage measurement

Never use MCU GPIO as a high-current fan power path.

---

# 15. OEM ECU FAN INPUT

The OEM ECU fan wire is an INPUT / MONITOR / REFERENCE signal to the controller.

It is NOT the fan power source.

The exact OEM topology/polarity is not known unless measured.

Do not assume:
- active-high
- active-low
- open collector
- pull-up
- pull-down
- voltage level

without actual circuit evidence.

Use high-impedance protected sensing and appropriate conditioning.

---

# 16. DEDICATED COOLANT TEMPERATURE SENSOR

The controller has its own dedicated coolant temperature sensor.

Do NOT use a shared ECU temperature sensor requirement.

Required handling:
- protected sensor input
- known sensor interface
- documented temperature curve/model
- filtering
- plausibility checks
- open/short detection where applicable
- calibration support

Do not invent sensor curve values.

---

# 17. ENVIRONMENTAL SENSOR

Provide one digital environmental sensor/module capable of supplying:
- temperature
- humidity
- atmospheric pressure
- calculated altitude

The sensor itself may be an IC/module according to the exact requirement, but do not introduce development boards or unnecessary external breakout hardware.

Altitude is calculated from atmospheric pressure in firmware.
Do not present altitude as a directly measured quantity unless the selected sensor explicitly provides it.

---

# 18. RPM INPUT

Only ONE physical universal RPM signal input port is required.

Potential sources:
- ignition coil negative
- ignition coil positive when waveform is suitable
- ECU tach/RPM output
- Hall sensor/digital pulse
- external RPM sensor
- 3.3 V digital pulse
- 5 V digital pulse
- 12 V square-wave signal where valid

CRITICAL:
Never connect ignition coil directly to the MCU.

Required signal chain concept:
INPUT
→ protection
→ divider/attenuation or suitable front end
→ clamp/TVS
→ filter
→ comparator/Schmitt/level conversion as required
→ RP2350B capture input

Configurable parameters:
- source type
- edge
- pulses/revolution
- filter
- input range

Automatic selection may only be used for safe documented input types.
Do not blindly auto-connect an ignition-coil waveform.

---

# 19. PRECISION ADC / DAC

External high-resolution ADC/DAC requirement remains.

Do NOT replace it merely with the MCU's internal ADC/DAC for convenience.

High nominal resolution does not automatically mean high measurement accuracy.

Verify:
- actual resolution
- ENOB where relevant
- reference
- reference stability
- input range
- common-mode range
- gain error
- offset
- noise
- bandwidth
- calibration capability
- analog layout requirements

Use external precision ADC for measurement functions that need it.
Use simpler MCU detection only where high precision is genuinely unnecessary and the design requirement permits it.

---

# 20. CURRENT / VOLTAGE / POWER MONITORING

Pump channel:
- voltage
- current
- calculated power

Fan channel:
- voltage
- current
- calculated power

Current sensing must be designed deliberately.

For shunts:
- select exact resistance
- select power rating
- verify temperature coefficient
- verify current rating
- use Kelvin sense connections where applicable

Do not treat a shunt's nominal value as sufficient proof of suitability.

---

# 21. MEMORY REQUIREMENTS

ESP32-S3-side external memory target:
- external NOR flash total: **at least 128 MB physical capacity**
- external PSRAM total: **at least 64 MB physical capacity**
- SD storage may be included as required by the architecture

IMPORTANT:
Do not confuse:
- megabits (Mb)
- megabytes (MB)

For example:
128 Mb is NOT 128 MB.

Memory selection must verify:
- exact MPN
- density
- voltage
- supported interface
- topology
- timing
- package
- ESP32-S3 compatibility
- routing constraints
- address/mapping limitations

Do not assume multiple chips can simply be paralleled to achieve capacity.
Follow the actual supported ESP32-S3 memory topology.

Physical installed capacity and processor-addressable/mapped window are different concepts.

---

# 22. EXTERNAL STORAGE / SD

SD may be provided for logging/service where required.

It must NOT become a prerequisite for real-time thermal control.

If SD fails:
- pump/fan critical control continues
- fault is recorded/reported if possible
- high-level logging can use RAM buffers or another path

---

# 23. RTC

RTC is a dedicated RTC/backup domain.

RTC backup supply must not keep the whole controller operating after the main system is intentionally disconnected.

RTC data may be used for:
- timestamps
- logs
- calibration records
- fault records

---

# 24. PROGRAMMING — FINAL ARCHITECTURE

No USB connector is required on the finished PCB.

Programming must occur directly to the chips through exposed PCB test/program pads.

## RP2350B programming pads

Provide accessible pads for at least:
- SWDIO
- SWCLK
- RUN/reset
- GND
- target 3V3 sense/reference as needed

The production/service fixture connects temporarily to these pads.

No permanent SWD header/socket.

## ESP32-S3 programming pads

Provide accessible pads for:
- UART0 TX / GPIO43
- UART0 RX / GPIO44
- GPIO0
- CHIP_PU/reset
- GND
- target 3V3 sense/reference as needed

ESP32-S3 supports UART firmware download; UART0 uses GPIO43/GPIO44 by default.
After flashing, GPIO0 must return to normal boot configuration.

References:
- https://docs.espressif.com/projects/esp-hardware-design-guidelines/en/latest/esp32s3/download-guidelines.html
- https://docs.espressif.com/projects/esp-hardware-design-guidelines/en/latest/esp32s3/schematic-checklist.html

---

# 25. CALIBRATION ARCHITECTURE

Calibration must be possible without a permanent external connector.

Use exposed PCB test pads / measurement nodes.

Calibration candidates:
- battery voltage
- pump voltage
- pump current
- fan voltage
- fan current
- precision ADC channels
- coolant temperature
- RPM
- analog inputs
- DAC outputs where applicable

Calibration record should contain, as applicable:
- board ID
- board revision
- channel ID
- calibration version/schema
- offset
- gain
- coefficients
- valid/invalid flag
- integrity check / CRC
- date/time if required

Do not hard-code individual production calibration values into firmware.

---

# 26. OTA — TWO-MCU COORDINATED UPDATE

OTA must update both RP2350B and ESP32-S3 as one controlled system.

Unified package should contain:
- manifest
- hardware compatibility
- board revision compatibility
- RP2350B firmware image
- ESP32-S3 firmware image
- dependency/version information
- integrity information
- signatures
- rollback metadata

Requirements:
- verify package before activation
- maintain known-good image
- fail safely
- rollback if validation fails
- health-check after update
- prevent version mismatch
- do not allow ESP32-S3 OTA to disable thermal protection
- ensure a safe state during RP2350B update

Security target:
- signed firmware
- secure boot where supported
- flash protection/encryption where supported
- debug/service locking at production stage
- protected update path
- recovery/rollback capability

---

# 27. FIRMWARE ENGINEERING RULES

Firmware must be modular and non-blocking.

Prefer:
- hardware timers
- interrupts
- state machines
- DMA
- queues
- ring buffers
- PIO where appropriate
- event-driven scheduling
- watchdogs

Avoid:
- long blocking delays
- busy-wait loops
- software timing for critical PWM
- unbounded operations in control loops

Configuration must have:
- version
- CRC/integrity
- range validation
- defaults
- rollback/recovery

Fault record should capture, where practical:
- event
- timestamp
- source
- severity
- system state
- recovery action
- diagnostic code

Transient electrical noise must not automatically become a permanent fault without filtering/plausibility criteria.

---

# 28. PCB POWER / THERMAL ZONING

Physical zones should be separated as much as possible:

## ZONE A — DIGITAL CONTROL
- RP2350B
- ESP32-S3
- memory
- communication

## ZONE B — ANALOG / SENSING
- precision ADC/DAC
- reference
- sensor front ends
- current-sense circuitry

## ZONE C — POWER / ACTUATOR
- battery entry
- TVS
- switching MOSFETs
- pump output
- fan output
- high-current shunts

Keep high-current switching paths away from sensitive analog/RF regions.

---

# 29. SMD PACKAGE POLICY

SMD is mandatory for normal production components.

Suggested strategy:
- 0201 where technically appropriate
- 0402 for normal passives where suitable
- 0603/0805 if voltage/power/reliability requires larger package
- QFN/DFN/SMD power devices as suitable
- SMD TVS
- SMD shunts
- SMD inductors
- SMD crystals

Do NOT force an undersized package solely for miniaturization.

The correct package is the smallest package that still meets:
- voltage
- current
- thermal
- assembly
- reliability
- manufacturing

requirements.

---

# 30. COMPONENT SUBSTITUTION POLICY

A catalog/CAD/download problem must NOT stop the whole build.

When a required component is unavailable:
1. Search for another verified component.
2. Prefer an SMD equivalent.
3. Verify the actual datasheet.
4. Verify pinout.
5. Verify voltage.
6. Verify current.
7. Verify power.
8. Verify temperature.
9. Verify signal level.
10. Verify timing/frequency where applicable.
11. Verify tolerance/accuracy/ESR/ESL where relevant.
12. Verify package.
13. Verify footprint.
14. Verify topology compatibility.

A substitute may have:
- different manufacturer
- different MPN
- different package if technically valid

A substitute may NOT be:
- through-hole
- development board
- module
- breakout
- unverified part

Do not redesign an entire subsystem merely to accommodate a convenient catalog part.

For every substitution record:

| Field | Required |
|---|---|
| ORIGINAL_MPN | Yes |
| ACTUAL_MPN | Yes |
| FUNCTION | Yes |
| PACKAGE | Yes |
| KEY_SPECIFICATION | Yes |
| DATASHEET/SOURCE | Yes |
| REASON_FOR_SUBSTITUTION | Yes |
| VERIFICATION_STATUS | Yes |

If no verified substitute can be established:
- mark only that component/block `DATA_INSUFFICIENT`
- continue all independent build work
- report the exact missing information

Never make up specifications.

---

# 31. COMPONENT IDENTIFICATION REQUIREMENT

All currently placed components must be audited.

Do NOT report only:
- C1
- C2
- C3
- U1
- U2

For every component provide:

`RefDes | Function | Manufacturer | Exact MPN | Package | Value/Rating | Purpose | Source | Substitute? | Verification`

The current count of approximately 25+ placed parts is NOT evidence that the design is complete.

---

# 32. CURRENT BUILD STATUS — DO NOT MISREPRESENT

Previous ProtoFlow sessions:
- attempted part placement
- experienced catalog/CAD availability and rate-limit failures
- placed a number of components
- added partial RP2350B wiring
- added partial ESP32-S3 wiring
- added crystal/reset/decoupling circuitry
- later continued wiring after the build had paused

Current truth:

**PARTIALLY BUILT / WIRING AND AUDIT IN PROGRESS**

Not proven complete.

The following are NOT yet considered verified simply because ProtoFlow said they were done:
- component completeness
- MPN correctness
- package correctness
- bare-chip compliance
- full RP2350B power network
- RP2350 regulator topology
- complete ESP32-S3 power/strap network
- flash
- PSRAM
- RF
- MCU communication
- external precision ADC/DAC
- safety hardware
- RPM front end
- OEM fan input
- pump output
- fan output
- current/voltage monitoring
- RTC
- SD
- programming pads
- calibration pads
- direct-wire landings
- all required nets
- ERC
- DRC
- final PCB

---

# 33. 25+ CURRENTLY PLACED COMPONENTS

The existing placed components must be treated as:
**PLACED — NOT YET PROVEN COMPLETE**

Do not delete them merely because the previous agent's report is incomplete.

First:
- inventory
- identify
- verify
- preserve valid work
- fix invalid work

Only delete or replace a part when the audit proves it is wrong or unnecessary.

---

# 34. SCHEMATIC PAGE STRUCTURE

Use separate pages under the same board/project.

## P01 — POWER ENTRY
Battery, fuse, reverse protection, TVS, EMI.

## P02 — POWER RAILS
3V3, 1V1, other required rails.

## P03 — HARDWARE SAFETY
Supervisor/watchdog/thermal safety/interlock.

## P04 — RP2350B CONTROL
Bare RP2350B.

## P05 — RP2350B CLOCK / RESET / FLASH / DEBUG
Crystal, decoupling, reset, flash, SWD.

## P06 — ESP32-S3 CORE
Bare ESP32-S3 power, crystal, boot/reset.

## P07 — ESP32-S3 FLASH / PSRAM / RF
Memory and RF.

## P08 — MCU COMMUNICATION
RP2350B ↔ ESP32-S3.

## P09 — PRECISION ADC / DAC
External precision data-conversion section.

## P10 — INPUTS / SENSORS / RPM / OEM FAN
Coolant sensor, environmental sensor, RPM front end, ignition/key, OEM fan request.

## P11 — PUMP
Universal pump power/control/feedback/current/voltage.

## P12 — FAN
Universal fan power/control/feedback/current/voltage.

## P13 — POWER MEASUREMENT
Current/voltage/power monitoring.

## P14 — RTC / STORAGE / SERVICE
RTC, storage and service/test infrastructure.

## P15 — DIRECT-WIRE LANDINGS
All external wire solder areas and their protection/strain relief.

---

# 35. BUILD GATES

## GATE 0 — CURRENT PROJECT RECOVERY
- recover current project
- preserve existing valid components/wiring

## GATE 1 — RP2350B
- exact MPN
- exact package
- power
- regulator
- clock
- reset
- decoupling
- SWD

## GATE 2 — ESP32-S3
- bare SoC
- power
- reset
- boot
- flash
- PSRAM
- RF

## GATE 3 — MCU COMMUNICATION
- pins
- protocol
- heartbeat
- CRC
- timeout
- safe fallback

## GATE 4 — ANALOG
- ADC
- DAC
- reference
- filtering
- calibration

## GATE 5 — INPUTS
- coolant temp
- environmental sensor
- RPM
- OEM fan request
- key/ignition

## GATE 6 — OUTPUTS
- pump
- fan
- power switching
- control signals
- feedback

## GATE 7 — SAFETY / POWER
- watchdog
- thermal hardware
- protection
- key-off
- low battery
- safe power cutoff

## GATE 8 — SERVICE / STORAGE
- RTC
- SD if used
- programming pads
- calibration pads

## GATE 9 — SCHEMATIC
- all required pages
- all nets
- no floating required pins
- no missing required components
- no connectors
- no modules
- ERC

## GATE 10 — PCB
- placement
- 4 layers
- RF
- analog
- power
- current paths
- thermal
- direct-wire landings
- strain relief
- size <60×60 mm target

## GATE 11 — FINAL VERIFICATION
- schematic audit
- ERC
- PCB DRC
- power review
- thermal review
- current-path review
- RF review
- programming review
- calibration review
- requirement review

If any gate fails:

**STOP → IDENTIFY → FIX → RECHECK**

Do not declare completion.

---

# 36. MANDATORY AUDIT BEFORE MORE BUILDING

Before adding major new circuitry, audit the current project.

Required audit outputs:

1. Current component inventory
2. Missing components
3. Incorrect components
4. Unverified components
5. Floating pins
6. Missing nets
7. Incorrect nets
8. Current subsystem completion
9. Bare-chip/module violations
10. connector violations
11. through-hole violations
12. programming-pad status
13. calibration-pad status
14. power-rail status
15. ERC status

Do NOT rely only on the previous agent's natural-language report.
Inspect the actual schematic/netlist/component properties.

---

# 37. ERC / DRC POLICY

ERC required errors must be:
**0 before completion**

Warnings:
- must be reviewed individually
- do not blindly suppress
- do not hide genuine problems

Any intentional unconnected pin must have a documented reason.

Do not mark pins “no connect” just to silence ERC unless the datasheet and design prove that the pin is intentionally unused.

PCB routing must not begin as the final step until the schematic passes the required audit.

After PCB routing:
- run DRC
- fix errors
- review warnings
- re-run

---

# 38. AI AGENT ANTI-HALLUCINATION RULE

The agent MUST NOT invent:
- MPN
- footprint
- GPIO assignment
- pin mapping
- sensor characteristics
- pump protocol
- fan protocol
- OEM fan polarity
- ignition waveform
- PWM frequency
- pulses/revolution
- voltage range
- current rating
- thermal limits
- memory topology
- RF antenna characteristics

When information is unavailable:
state:

`DATA INSUFFICIENT`

Then state exactly what evidence is needed.

Do not fill the gap with a plausible guess.

---

# 39. WHEN A CATALOG ERROR OCCURS

Bad behavior:
`CAD unavailable → stop entire build`

Correct behavior:
1. identify failed component
2. search verified equivalent SMD
3. verify
4. substitute
5. record substitution
6. update schematic
7. check affected nets
8. ERC affected block
9. continue

Only the blocked component/block remains blocked if no valid substitute exists.

---

# 40. WHEN THE AGENT WAS PREVIOUSLY INTERRUPTED

Do NOT restart.

First determine:
- last valid component state
- last valid wiring state
- last successful gate
- remaining work
- current errors

Then continue from the actual state.

Never erase a working subsystem simply because it was created by a previous agent.

---

# 41. DIRECT-WIRE BOARD GEOMETRY RULE

The board must be smaller than 60×60 mm target, but do not force an arbitrary exact size before:
- component count
- power paths
- thermal paths
- RF keepout
- memory placement
- direct-wire pads
- programming pads
- calibration pads

are known.

Optimize the geometry after the real requirements are satisfied.

Do not make:
- high-current paths too narrow
- solder landings too small
- component spacing unsafe
- RF keepout inadequate
- thermal copper insufficient

merely to meet a numeric board-size target.

---

# 42. HIGH-CURRENT SAFETY PRIORITY

For battery/pump/fan power paths, optimize in this order:

1. electrical safety
2. current capacity
3. thermal performance
4. voltage drop
5. mechanical reliability
6. EMI/EMC
7. size minimization

Never reverse this order merely to make the PCB smaller.

---

# 43. PRODUCTION SERVICE PHILOSOPHY

The finished PCB should have:
- no permanent external connectors
- accessible test pads
- programming pads
- calibration points
- measurement points
- production test strategy
- service recovery capability

A temporary external production fixture is acceptable.

A permanent connector is not.

---

# 44. REQUIRED FINAL REPORT FROM THE AGENT

After every significant work unit, report:

## STATUS
- Current gate
- Complete / In Progress / Blocked

## COMPONENTS
- verified count
- unverified count
- missing count
- substituted count

## CONNECTIONS
- connected required nets
- floating required nets
- incorrect nets found
- fixed nets

## SAFETY
- watchdog
- thermal protection
- power protection
- safe state

## EXTERNAL CONNECTIONS
- direct-wire landings
- current path status
- strain relief status
- connector violations

## PROGRAMMING
- RP2350B SWD pads
- ESP32-S3 UART/boot pads

## CALIBRATION
- calibration pads
- calibration storage

## ERC
- errors
- warnings
- fixes

## PCB
- layer status
- approximate dimensions
- high-current route status
- RF status
- DRC status

## REMAINING WORK
Exact unresolved items only.

Never say “complete” when any mandatory item remains unresolved.

---

# 45. IMMEDIATE NEXT ACTION FOR THE CURRENT PROJECT

Do NOT begin by deleting the project.

Do NOT rebuild everything from zero.

Do NOT jump directly to final PCB routing.

Do this exact sequence:

### STEP A — CURRENT STATE AUDIT
Inventory every currently placed component and every existing connection.

### STEP B — VERIFY MCU IDENTITY
Confirm:
- U1 = RP2350B bare IC
- U2 = ESP32-S3 bare SoC

### STEP C — VERIFY RP2350B POWER
Audit regulator topology against the official RP2350B documentation.

### STEP D — VERIFY ESP32-S3 CORE
Audit power, reset, boot, clock and all required pins.

### STEP E — COMPLETE MEMORY / RF
Verify flash, PSRAM and RF architecture.

### STEP F — COMPLETE CONTROL COMMUNICATION
Finish RP2350B ↔ ESP32-S3 link.

### STEP G — COMPLETE INPUTS
Temperature, environmental, RPM, OEM fan, ignition/key.

### STEP H — COMPLETE OUTPUTS
Pump and fan universal stages.

### STEP I — COMPLETE MEASUREMENT
Voltage/current/power.

### STEP J — COMPLETE SAFETY
Supervisor/watchdog/thermal/power state logic.

### STEP K — ADD SERVICE
Programming pads, calibration pads, measurement pads.

### STEP L — COMPLETE ALL 15 PAGES

### STEP M — ERC
Fix every required error.

### STEP N — RECHECK
Repeat the audit.

### STEP O — ONLY THEN MOVE TO PCB
Place, route, thermal review, RF review, DRC.

---

# 46. AUTHORITATIVE DECISIONS

The following are final unless the user explicitly changes them:

- 4-layer PCB
- target <60×60 mm
- bare RP2350B
- bare ESP32-S3
- SMD production design
- no development boards
- no modules
- no external field connectors
- external wires solder directly to PCB
- PCB strain relief for field wires
- direct programming through PCB test pads
- no permanent USB connector
- direct calibration through PCB test pads
- RP2350B = critical real-time/safety/thermal controller
- ESP32-S3 = UI/connectivity/AI/high-level controller
- RP2350B must operate independently of ESP32-S3
- external precision ADC/DAC requirement remains
- external flash ≥128 MB physical target
- external PSRAM ≥64 MB physical target
- dedicated coolant temperature sensor
- one universal RPM input port
- OEM ECU fan signal is monitored as a protected input
- universal 2/3/4-wire pump support
- universal 2/3/4-wire fan support
- individual pump/fan voltage/current/power monitoring
- three primary power states:
  - KEY OFF / ENGINE OFF
  - KEY ON / ENGINE ON OR OFF
  - LOW BATTERY / KEY OFF
- OTA must coordinate both MCU images
- rollback and update safety required
- ERC/DRC required
- no completion with unresolved mandatory errors

---

# 47. SOURCE-OF-TRUTH REFERENCES

Use current official documentation as the electrical authority.

## Raspberry Pi RP2350
RP2350 Datasheet:
https://datasheets.raspberrypi.com/rp2350/rp2350-datasheet.pdf

Hardware Design with RP2350:
https://datasheets.raspberrypi.com/rp2350/hardware-design-with-rp2350.pdf

## Espressif ESP32-S3
ESP32-S3 Hardware Design Guidelines:
https://docs.espressif.com/projects/esp-hardware-design-guidelines/en/latest/esp32s3/

ESP32-S3 Schematic Checklist:
https://docs.espressif.com/projects/esp-hardware-design-guidelines/en/latest/esp32s3/schematic-checklist.html

ESP32-S3 PCB Layout:
https://docs.espressif.com/projects/esp-hardware-design-guidelines/en/latest/esp32s3/pcb-layout-design.html

ESP32-S3 Download Guidelines:
https://docs.espressif.com/projects/esp-hardware-design-guidelines/en/latest/esp32s3/download-guidelines.html

## EasyEDA Pro
https://prodocs.easyeda.com/en/

---

# 48. FINAL COMMAND TO THE AI AGENT

You are continuing an existing real PCB design.

**DO NOT START OVER.**
**DO NOT DELETE VALID WORK.**
**DO NOT TRUST COMPLETION CLAIMS WITHOUT VERIFICATION.**
**DO NOT STOP THE WHOLE BUILD BECAUSE ONE CATALOG PART IS UNAVAILABLE.**
**USE VERIFIED SMD SUBSTITUTES WHEN VALID.**
**DO NOT USE MODULES.**
**DO NOT USE DEVELOPMENT BOARDS.**
**DO NOT USE FIELD CONNECTORS.**
**SOLDER EXTERNAL WIRES DIRECTLY TO THE PCB.**
**PROVIDE PROPER HIGH-CURRENT COPPER AND MECHANICAL STRAIN RELIEF.**
**PROGRAM RP2350B THROUGH SWD TEST PADS.**
**PROGRAM ESP32-S3 THROUGH UART0/BOOT TEST PADS.**
**PROVIDE CALIBRATION TEST PADS.**
**VERIFY EVERY COMPONENT AND EVERY REQUIRED NET.**
**DO NOT GUESS UNKNOWN ELECTRICAL PARAMETERS.**
**FIX ERRORS, THEN RECHECK.**
**DO NOT DECLARE COMPLETION UNTIL THE ENTIRE DESIGN PASSES THE REQUIRED AUDIT, ERC AND DRC.**

### REQUIRED OPERATING LOOP

**READ CURRENT STATE**
→ **AUDIT**
→ **IDENTIFY MISSING/WRONG ITEMS**
→ **BUILD/FIX**
→ **VERIFY**
→ **ERC**
→ **FIX**
→ **ERC AGAIN**
→ **PCB**
→ **DRC**
→ **FIX**
→ **FINAL REQUIREMENT AUDIT**
→ **ONLY THEN REPORT COMPLETE**


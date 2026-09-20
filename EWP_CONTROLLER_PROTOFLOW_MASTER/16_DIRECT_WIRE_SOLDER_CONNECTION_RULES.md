# DIRECT-WIRE / NO-CONNECTOR RULES

## Core requirement
All external field wiring is soldered directly to the PCB.

There shall be NO detachable connector, terminal block, JST/Molex/Hirose plug,
header, receptacle, socket, wire-to-board connector, or soldered connector body
for the following external connections:

- Battery +12 V
- Battery GND
- Ignition / key sense
- Pump power and control/feedback wires
- Radiator fan power and control/feedback wires
- Dedicated coolant-temperature sensor wiring
- RPM signal wiring
- OEM ECU fan-request/reference wiring
- CAN/service external wiring, where the approved architecture requires it

## Important distinction
A PCB solder pad, plated wire-entry hole, plated slot, via, test pad, or strain-relief
feature is NOT a connector and is allowed.

A wire may be soldered directly to a purpose-designed copper landing area.
When additional mechanical retention is beneficial, a plated wire-entry hole or
plated slot may be used so the conductor can pass through the PCB and be soldered
on the copper side. This is a PCB feature, not a component or socket.

Do not add a connector merely because a CAD library has one available.

## External-wire landing design
External wire landing areas MUST be designed as dedicated high-current or signal
solder zones, not ordinary small component pads.

For each external circuit, ProtoFlow must define:
- Net name
- Wire function
- Maximum continuous current
- Peak/inrush current where relevant
- Expected voltage
- Recommended wire cross-section or AWG range
- PCB copper thickness used by the power path
- Minimum copper width / polygon area
- Solder landing dimensions
- Clearance to adjacent nets
- Mechanical strain-relief method
- Thermal/current verification status

## High-current outputs
Pump and fan current must NOT enter through a small signal pad.

For every high-current wire:
1. Use a dedicated large copper landing area.
2. Keep the landing close to the relevant MOSFET/current-sense/power path.
3. Minimize the high-current path length.
4. Use wide copper or copper pours sized from actual current, copper thickness,
   allowable temperature rise and voltage-drop requirements.
5. Use via arrays/stitching whenever a high-current path changes layer.
6. Avoid thermal-relief spokes on primary high-current power landings where
   manufacturing rules allow; use solid copper connection to the power region.
7. Place current-sense shunts so Kelvin sense lines do not share the high-current
   voltage-drop path.
8. Provide local copper area for heat spreading around high-current MOSFETs,
   shunts, TVS devices and other power components.

## Wire size and PCB copper
Do NOT choose PCB trace width from visual convenience.

The design must be based on the actual wire current requirement and the actual PCB
copper stack-up.

Required design sequence:
CURRENT -> WIRE SIZE -> PCB COPPER THICKNESS -> TRACE/POLYGON WIDTH ->
VIA CAPACITY -> TEMPERATURE RISE CHECK -> VOLTAGE DROP CHECK.

If the required current is not known from authoritative hardware data,
mark DATA_INSUFFICIENT rather than guessing.

For the currently specified 12 V / 10 A-class EWP pump, treat the 10 A value as the
minimum continuous design case for the pump power path unless measured/verified
hardware data establishes a higher requirement. Do not assume a PWM/burst current
waveform or starting current; measure the actual pump when those values are needed.

## Spacing
Wire solder zones must be placed so that:
- bare wire cannot bridge adjacent conductors during soldering;
- high-current positive and ground landings have deliberate separation;
- battery entry protection is physically close to the battery landing;
- motor switching nodes are kept away from analog/RF/sensor areas;
- service/test pads do not become accidental external connection points.

Clearance and creepage must be checked against the highest voltage/transient
condition applicable to the board and the manufacturing process.

## Mechanical reliability
Direct soldering removes a connector failure point, but the solder joint and PCB
pad can still fail if the wire is pulled or vibrated.

Therefore every external wire group MUST have a mechanical strain-relief strategy.
Preferred implementation:
- solder land near the board edge;
- nearby PCB tie/anchor feature or enclosure clamp;
- wire route arranged so vibration load does not act directly on the solder fillet.

Do not make the solder joint itself the only load-bearing mechanical feature.

## Compact-board rule
Board target remains < 60 mm x 60 mm.

External wire landings must be arranged around the perimeter so they do not consume
central component-placement area. Keep high-current motor landings grouped in the
power/output zone and keep RF/MCU/precision-analog zones separated.

Do not enlarge the board merely to fit connector footprints.

## Service/programming
No USB receptacle, SWD header, UART header, or other detachable service connector
shall be placed solely for convenience.

Use compact exposed test/program pads where electrically appropriate. If a service
cable is required during manufacturing, it may be attached temporarily by direct
contact or soldered wires.

## Removable SD restriction
A removable SD/microSD socket is forbidden under the no-connector rule.
Use onboard non-removable storage or another approved soldered storage solution.
Do not add a card socket to satisfy the SD requirement.

## Verification gate
A design cannot pass schematic/PCB verification until:
- all external interfaces are identified;
- no prohibited connector symbols/footprints remain;
- every external wire landing has sufficient copper and spacing;
- high-current paths are reviewed for current capacity and temperature rise;
- wire strain relief is documented;
- direct-wire soldering is manufacturable and repairable;
- PCB remains below the dimensional target unless an explicit thermal/electrical
  justification is documented.

## Programming/calibration clarification
Programming and calibration are NOT field wiring. They use compact exposed PCB test
pads contacted temporarily by an external manufacturing/service fixture. No USB,
SWD, UART or other detachable connector footprint may be placed on the product PCB.
See 17_PROGRAMMING_CALIBRATION_RULES.md.

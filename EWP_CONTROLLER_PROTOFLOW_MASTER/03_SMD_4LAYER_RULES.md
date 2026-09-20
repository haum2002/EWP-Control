# SMD + COMPACT 4-LAYER RULES

## Default package strategy
- 0201: RF matching and suitable low-power passives
- 0402: general passives
- 0603/0805 only where power, voltage, assembly or reliability requires it
- QFN/DFN/SMD ICs where suitable
- SMD MOSFETs, TVS, shunts, inductors and crystals

Never force a tiny package beyond its voltage/current/power/thermal rating.

## Through-hole prohibition
No through-hole resistors, capacitors, ICs, MOSFETs, diodes, crystals or inductors. A through-hole part requires a documented engineering exception. External connector COMPONENTS are forbidden by project requirement. Direct wire-entry holes/slots may be used as PCB features for soldering wires directly to the board.

## 4-layer target
L1 = components + critical signals + RF
L2 = continuous GND plane
L3 = power + selected signals
L4 = signals

Prefer no bottom components unless density/thermal/mechanical reasons justify them.

## Compactness
Keep decoupling close to IC pins, RF matching close to RF pin, motor power switching close to the direct-wire board landings, current shunts in the real current path, and analogue away from motor switching. Do not make the PCB larger merely to simplify routing.

Compactness must never compromise thermal dissipation, current capacity, RF keepout, analogue noise, creepage/clearance or manufacturability.

## Direct-wire soldering features
External wires shall terminate on dedicated PCB solder landings at the board perimeter.
A plated wire-entry hole/slot and separate mechanical anchor hole/feature may be used
when it improves strain relief; these are PCB fabrication features, not connectors.

For high-current pump/fan/battery paths, use large copper areas, short paths, layer
stitching as required, and thermal/current verification. Do not size the copper from
connector footprint dimensions because connectors are not permitted.

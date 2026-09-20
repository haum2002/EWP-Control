# VERIFICATION CHECKLIST

## Architecture
- [ ] RP2350B bare IC
- [ ] ESP32-S3 bare SoC
- [ ] no development modules
- [ ] RP2350B autonomous for critical cooling

## Power
- [ ] fuse/protection
- [ ] reverse polarity
- [ ] TVS/load transient protection
- [ ] EMI filtering
- [ ] controlled load disconnect
- [ ] low-battery key-off hard disconnect
- [ ] RTC backup isolated
- [ ] every regulator complete

## RP2350B
- [ ] all required supply pins connected
- [ ] VREG_LX complete
- [ ] VREG_FB complete
- [ ] decoupling complete
- [ ] clock complete
- [ ] reset complete
- [ ] SWD/service complete
- [ ] boot/QSPI complete

## ESP32-S3
- [ ] bare SoC
- [ ] power/decoupling
- [ ] strap/reset
- [ ] crystal
- [ ] flash
- [ ] PSRAM
- [ ] physical capacity calculated
- [ ] RF matching/antenna

## Analogue/sensors
- [ ] external precision ADC
- [ ] reference/filtering
- [ ] DAC where required
- [ ] dedicated coolant sensor
- [ ] environmental sensor
- [ ] RTC

## Inputs
- [ ] universal RPM protection/conditioning
- [ ] OEM fan request protected/high impedance
- [ ] key/ignition

## Outputs
- [ ] pump power stage
- [ ] fan power stage
- [ ] 2/3/4 wire support
- [ ] feedback
- [ ] current/voltage
- [ ] no motor current through MCU GPIO

## Safety
- [ ] independent supervisor/watchdog
- [ ] thermal safety
- [ ] safe-state behavior
- [ ] ESP32 failure cannot stop cooling

## Direct-wire external interfaces
- [ ] no external connector/terminal/header/socket footprints
- [ ] battery wires solder directly to dedicated PCB landings
- [ ] pump wires solder directly to dedicated PCB landings
- [ ] fan wires solder directly to dedicated PCB landings
- [ ] sensor/RPM/ignition wires solder directly to dedicated PCB landings
- [ ] each high-current landing has verified copper area/current capacity
- [ ] wire gauge/cross-section documented for each external power circuit
- [ ] wire landing spacing prevents accidental bridging during soldering
- [ ] PCB/enclosure strain relief documented
- [ ] no solder joint is the sole mechanical load-bearing feature
- [ ] board outline remains below 60 mm x 60 mm or exception is documented

## SMD/4-layer
- [ ] SMD-first
- [ ] no unjustified through-hole
- [ ] L2 continuous GND
- [ ] RF isolated
- [ ] analogue isolated
- [ ] high-current zone separated
- [ ] thermal review

## ERC/DRC
- [ ] no required floating power pins
- [ ] no orphan nets
- [ ] no unintended shorts
- [ ] no duplicate references
- [ ] all required pins connected
- [ ] all warnings reviewed
- [ ] PCB DRC passed
- [ ] unconnected check passed

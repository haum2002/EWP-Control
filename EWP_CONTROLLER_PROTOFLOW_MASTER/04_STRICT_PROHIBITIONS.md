# STRICT PROHIBITIONS

NEVER:
- use ESP32-S3-WROOM/MINI/XIAO/DevKit modules
- use Pico/Pico 2/RP2350 development boards
- substitute a module for a bare IC
- use a net label as a substitute for an actual required circuit
- leave required power pins floating
- guess pin numbers, values, polarity, protocols, memory topology or PWM frequency
- connect ignition coil directly to an MCU
- carry motor current through an MCU GPIO
- replace the external ADC/DAC requirement with an MCU peripheral
- call 128 Mb "128 MB"
- use through-hole COMPONENTS merely because CAD is easier
- use any external connector, terminal block, header, receptacle or socket
- route high-current external wiring into a small signal pad intended for a component
- make the solder fillet itself the only mechanical strain-relief mechanism for a high-vibration external wire
- remove requirements because they are inconvenient
- start PCB routing before schematic verification
- declare completion with required ERC/DRC failures
- stop the entire project because one CAD download is temporarily unavailable
- use empty blocks/placeholders in a supposedly completed schematic
- silently substitute a component

## DATA_INSUFFICIENT
Use only when authoritative documentation cannot establish the needed fact. State the exact unknown, why it matters, the sources checked, and whether independent work can continue. Never guess.

## CAD failure
Keep the exact required MPN, mark only the footprint/CAD status as pending, and continue independent work. Never replace it with a module just because CAD is available.


## Component substitution clarification
- DO NOT pause the entire build solely because one catalog part/CAD file is unavailable.
- DO use a verified SMD equivalent when it preserves electrical, thermal, timing, RF, safety and mechanical requirements.
- DO NOT substitute by catalog convenience alone.
- DO NOT use through-hole, module, dev board, breakout board or oversized part as a shortcut.
- DO NOT leave required functions as empty placeholders when a verified SMD substitute exists.
- DO NOT hide substitutions; record original requested MPN and actual placed MPN in the component audit/BOM.

## Direct-wire requirement
External field wiring is direct-soldered to PCB landings. This is mandatory and is
not optional even when ProtoFlow finds a convenient connector footprint.

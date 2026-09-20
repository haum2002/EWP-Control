# AUTHORITATIVE SOURCES

## Raspberry Pi RP2350
Datasheet:
https://datasheets.raspberrypi.com/rp2350/rp2350-datasheet.pdf

Hardware design:
https://datasheets.raspberrypi.com/rp2350/hardware-design-with-rp2350.pdf

Microcontroller documentation:
https://www.raspberrypi.com/documentation/microcontrollers/microcontroller-chips.html

## Espressif ESP32-S3
Hardware design guidelines:
https://docs.espressif.com/projects/esp-hardware-design-guidelines/en/latest/esp32s3/

Schematic checklist:
https://docs.espressif.com/projects/esp-hardware-design-guidelines/en/latest/esp32s3/schematic-checklist.html

PCB layout:
https://docs.espressif.com/projects/esp-hardware-design-guidelines/en/latest/esp32s3/pcb-layout-design.html

## EasyEDA Pro
Guide:
https://prodocs.easyeda.com/en/

Create schematic/page:
https://prodocs.easyeda.com/en/schematic/file-new-schematic-page/

Schematic format:
https://prodocs.easyeda.com/en/format/schematic/index/

## Source priority
1. exact manufacturer datasheet
2. official hardware design/reference documentation
3. verified manufacturer application/reference design
4. this project package
5. ECAD library convenience

The current Espressif guidance recommends a 4-layer design for ESP32-S3, with L2 as a continuous GND plane and power/signals on L3; it also specifies RF, flash/PSRAM and crystal layout considerations. Use the current official guidance rather than module convenience.

EWP_CONTROLLER_PROTOFLOW_MASTER v1.3

Use this package as the design contract for the ProtoFlow/EasyEDA project.
Read 00_START_HERE.md first.

Important v1.3 additions:
- no external connectors/sockets for field wiring
- no detachable programming/service connectors
- direct-wire PCB solder landings for field wires
- temporary test-pad fixture for programming/calibration
- RP2350 SWD test pads
- ESP32-S3 UART0 + GPIO0/CHIP_PU test pads
- current-build audit for bare ESP32-S3 and RP2350 regulator pin naming

Golden rule: PLAN -> BUILD -> CHECK -> FIX -> RECHECK -> ONLY THEN COMPLETE

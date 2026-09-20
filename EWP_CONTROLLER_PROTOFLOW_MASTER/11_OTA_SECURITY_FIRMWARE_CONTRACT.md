# OTA + SECURITY + FIRMWARE CONTRACT

RP2350B is the safety-critical real-time controller. ESP32-S3 is high-level connectivity/UI.

ESP32-S3 failure must not stop pump/fan/thermal control.

## MCU bus
Provide framing, version, sequence, CRC, heartbeat, timeout, acknowledgement and fault status.

## OTA package
Manifest + hardware ID/revision + RP2350B image + ESP32-S3 image + version/dependency metadata + integrity/signature data + rollback metadata.

## OTA safety
ESP32-S3 update: RP2350B continues cooling control.
RP2350B update: enter a documented safe thermal-control state; recover to previous known-good firmware if validation fails.

## Security
Only implement and claim vendor-documented mechanisms. Production debug access must be restricted according to the selected security architecture.

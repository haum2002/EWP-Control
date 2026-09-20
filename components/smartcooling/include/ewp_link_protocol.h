#ifndef SMARTCOOLING_EWP_LINK_PROTOCOL_H
#define SMARTCOOLING_EWP_LINK_PROTOCOL_H

#include <stdint.h>

namespace ewp_link {

constexpr uint8_t PROTOCOL_VERSION = 1;
constexpr uint16_t FRAME_MAGIC = 0x4557;
constexpr uint16_t MAX_PAYLOAD_SIZE = 256;
constexpr uint32_t DEFAULT_HEARTBEAT_TIMEOUT_MS = 1500;

enum class MessageType : uint8_t {
  Heartbeat = 1,
  Status = 2,
  Command = 3,
  Configuration = 4,
  Fault = 5,
  Acknowledge = 6,
};

// Transport-neutral frame contract. GPIO/UART allocation remains a schematic gate.
struct FrameHeader {
  uint16_t magic;
  uint8_t protocol_version;
  MessageType message_type;
  uint16_t payload_length;
  uint32_t sequence;
  uint16_t payload_crc16;
};

struct HeartbeatPayload {
  uint32_t uptime_ms;
  uint16_t firmware_major;
  uint16_t firmware_minor;
  uint8_t safety_state;
  uint8_t reserved[3];
};

struct CommandPayload {
  int16_t target_celsius_x10;
  uint8_t pump_percent;
  uint8_t fan_percent;
  uint8_t mode;
  uint8_t valid;
};

constexpr bool is_valid_payload_size(uint16_t size) {
  return size <= MAX_PAYLOAD_SIZE;
}

}  // namespace ewp_link

#endif

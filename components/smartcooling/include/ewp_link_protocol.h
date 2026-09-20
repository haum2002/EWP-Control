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

// Supplementary payload contracts. All multi-byte fields are little-endian on the wire.
struct StatusPayload {
  uint32_t uptime_ms;
  int16_t coolant_c_x10;
  uint8_t pump_percent;
  uint8_t fan_percent;
  uint8_t safety_state;
  uint8_t faults_lo;
  uint8_t risk_score_pct;
  uint8_t reserved[3];
};

struct FaultPayload {
  uint16_t fault_flags;
  uint8_t severity;
  uint8_t source;
  uint8_t reserved[4];
};

struct AckPayload {
  uint8_t acked_message_type;
  uint8_t status;
  uint16_t reserved;
};

constexpr size_t HEADER_WIRE_SIZE = 12;

constexpr bool is_valid_payload_size(uint16_t size) {
  return size <= MAX_PAYLOAD_SIZE;
}

// --- Transport-neutral codec. No GPIO/UART allocation; pin assignment stays a schematic gate. ---
uint16_t crc16CCITT(const uint8_t *data, size_t len);

// Returns total wire size (header + payload) or 0 if payload too large.
size_t encodeFrame(uint8_t *out, size_t out_cap, MessageType type, uint32_t sequence,
                   const uint8_t *payload, uint16_t payload_len);

// Decodes a frame from a buffer. On success fills *type/*sequence/*payload_len and copies the
// payload into payload_out (cap = MAX_PAYLOAD_SIZE). Returns consumed wire size, or 0 on any
// integrity failure (bad magic/version/length/crc).
size_t decodeFrame(const uint8_t *in, size_t in_len, MessageType &type, uint32_t &sequence,
                   uint16_t &payload_len, uint8_t *payload_out);

// Convenience (de)serializers for the documented payload structs (little-endian).
size_t encodeHeartbeat(uint8_t *out, size_t cap, const HeartbeatPayload &p);
bool decodeHeartbeat(const uint8_t *in, size_t len, HeartbeatPayload &out);

size_t encodeCommand(uint8_t *out, size_t cap, const CommandPayload &p);
bool decodeCommand(const uint8_t *in, size_t len, CommandPayload &out);

size_t encodeStatus(uint8_t *out, size_t cap, const StatusPayload &p);
bool decodeStatus(const uint8_t *in, size_t len, StatusPayload &out);

size_t encodeFault(uint8_t *out, size_t cap, const FaultPayload &p);
bool decodeFault(const uint8_t *in, size_t len, FaultPayload &out);

size_t encodeAck(uint8_t *out, size_t cap, const AckPayload &p);
bool decodeAck(const uint8_t *in, size_t len, AckPayload &out);

// Heartbeat/sequence supervisor. Implements the ESP32-S3-loss-must-not-stop-cooling policy:
// when no valid frame is received within the timeout, the link is declared dead so the
// safety MCU falls back to autonomous operation.
class LinkSupervisor {
public:
  LinkSupervisor();

  // Call whenever a valid frame is decoded. Validates sequence monotonicity.
  void onFrameReceived(MessageType type, uint32_t sequence);

  // True while the peer is considered alive (heartbeat within timeout).
  bool isAlive(uint32_t now_ms) const;

  // Milliseconds since the last valid frame. Returns UINT32_MAX before the first frame.
  uint32_t lastFrameAgeMs(uint32_t now_ms) const;

  // True if the last received sequence broke monotonic ordering (possible replay/drop).
  bool sequenceAnomaly() const { return _seq_anomaly; }

  void setTimeoutMs(uint32_t ms) { _timeout_ms = ms; }
  uint32_t getTimeoutMs() const { return _timeout_ms; }
  uint32_t getLastSequence() const { return _last_sequence; }
  uint32_t getFrameCount() const { return _frame_count; }

private:
  uint32_t _last_frame_ms;
  uint32_t _last_sequence;
  uint32_t _frame_count;
  uint32_t _timeout_ms;
  bool _have_sequence;
  bool _seq_anomaly;
};

}  // namespace ewp_link

#endif

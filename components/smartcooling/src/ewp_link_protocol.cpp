// SmartCooling EWP link protocol — transport-neutral frame codec and link supervisor.
//
// This module implements the wire codec for the frame contract declared in
// ewp_link_protocol.h. It deliberately makes NO GPIO/UART assignment: pin allocation
// remains blocked until the schematic assigns a verified RP2350B<->ESP32-S3 link, per
// docs/SMARTCOOLINGV2_1_INTEGRATION.md. The codec is transport-neutral so it can be
// driven later by UART, SPI, or a pair of GPIOs once the hardware gate passes.

#include "ewp_link_protocol.h"

#include <Arduino.h>
#include <cstring>

namespace ewp_link {

// ---------------------------------------------------------------------------
// CRC16-CCITT (poly 0x1021, init 0xFFFF, no final XOR swap). Matches the
// payload_crc16 field that the contract already reserves in FrameHeader.
// ---------------------------------------------------------------------------
uint16_t crc16CCITT(const uint8_t *data, size_t len) {
  uint16_t crc = 0xFFFF;
  if (!data) {
    return crc;
  }
  for (size_t i = 0; i < len; ++i) {
    crc ^= (uint16_t)data[i] << 8;
    for (uint8_t b = 0; b < 8; ++b) {
      crc = (crc & 0x8000) ? (uint16_t)((crc << 1) ^ 0x1021) : (uint16_t)(crc << 1);
    }
  }
  return crc;
}

static inline void putU16LE(uint8_t *&p, uint16_t v) {
  p[0] = (uint8_t)(v & 0xFF);
  p[1] = (uint8_t)((v >> 8) & 0xFF);
  p += 2;
}

static inline void putU32LE(uint8_t *&p, uint32_t v) {
  p[0] = (uint8_t)(v & 0xFF);
  p[1] = (uint8_t)((v >> 8) & 0xFF);
  p[2] = (uint8_t)((v >> 16) & 0xFF);
  p[3] = (uint8_t)((v >> 24) & 0xFF);
  p += 4;
}

static inline uint16_t getU16LE(const uint8_t *p) {
  return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

static inline uint32_t getU32LE(const uint8_t *p) {
  return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
         ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

// ---------------------------------------------------------------------------
// Frame codec. Wire layout (little-endian):
//   0  magic            u16  (FRAME_MAGIC)
//   2  protocol_version u8
//   3  message_type     u8
//   4  payload_length   u16
//   6  sequence         u32
//  10  payload_crc16    u16
//  12  payload[payload_length]
// ---------------------------------------------------------------------------
size_t encodeFrame(uint8_t *out, size_t out_cap, MessageType type, uint32_t sequence,
                   const uint8_t *payload, uint16_t payload_len) {
  if (!out) {
    return 0;
  }
  if (payload_len > 0 && !payload) {
    return 0;
  }
  if (!is_valid_payload_size(payload_len)) {
    return 0;
  }
  size_t total = HEADER_WIRE_SIZE + payload_len;
  if (out_cap < total) {
    return 0;
  }

  uint8_t *p = out;
  putU16LE(p, FRAME_MAGIC);
  *p++ = PROTOCOL_VERSION;
  *p++ = (uint8_t)type;
  putU16LE(p, payload_len);
  putU32LE(p, sequence);
  uint16_t crc = (payload_len > 0) ? crc16CCITT(payload, payload_len) : 0;
  putU16LE(p, crc);
  if (payload_len > 0) {
    memcpy(p, payload, payload_len);
    p += payload_len;
  }
  return (size_t)(p - out);
}

size_t decodeFrame(const uint8_t *in, size_t in_len, MessageType &type, uint32_t &sequence,
                   uint16_t &payload_len, uint8_t *payload_out) {
  type = MessageType::Heartbeat;
  sequence = 0;
  payload_len = 0;
  if (!in || !payload_out) {
    return 0;
  }
  if (in_len < HEADER_WIRE_SIZE) {
    return 0;
  }
  if (getU16LE(in) != FRAME_MAGIC) {
    return 0;
  }
  if (in[2] != PROTOCOL_VERSION) {
    return 0;
  }
  uint8_t mt = in[3];
  if (mt < 1 || mt > 6) {
    return 0;
  }
  type = (MessageType)mt;
  uint16_t plen = getU16LE(in + 4);
  if (!is_valid_payload_size(plen)) {
    return 0;
  }
  size_t total = HEADER_WIRE_SIZE + plen;
  if (in_len < total) {
    return 0;
  }
  uint16_t crc_stored = getU16LE(in + 10);
  uint16_t crc_calc = (plen > 0) ? crc16CCITT(in + HEADER_WIRE_SIZE, plen) : 0;
  if (crc_stored != crc_calc) {
    return 0;
  }
  sequence = getU32LE(in + 6);
  payload_len = plen;
  if (plen > 0) {
    memcpy(payload_out, in + HEADER_WIRE_SIZE, plen);
  }
  return total;
}

// ---------------------------------------------------------------------------
// Payload (de)serializers (little-endian, packed).
// ---------------------------------------------------------------------------
size_t encodeHeartbeat(uint8_t *out, size_t cap, const HeartbeatPayload &p) {
  if (!out || cap < sizeof(HeartbeatPayload)) {
    return 0;
  }
  uint8_t *w = out;
  putU32LE(w, p.uptime_ms);
  putU16LE(w, p.firmware_major);
  putU16LE(w, p.firmware_minor);
  *w++ = p.safety_state;
  w[0] = p.reserved[0];
  w[1] = p.reserved[1];
  w[2] = p.reserved[2];
  w += 3;
  return (size_t)(w - out);
}

bool decodeHeartbeat(const uint8_t *in, size_t len, HeartbeatPayload &out) {
  if (!in || len < sizeof(HeartbeatPayload)) {
    return false;
  }
  out.uptime_ms = getU32LE(in);
  out.firmware_major = getU16LE(in + 4);
  out.firmware_minor = getU16LE(in + 6);
  out.safety_state = in[8];
  out.reserved[0] = in[9];
  out.reserved[1] = in[10];
  out.reserved[2] = in[11];
  return true;
}

size_t encodeCommand(uint8_t *out, size_t cap, const CommandPayload &p) {
  if (!out || cap < sizeof(CommandPayload)) {
    return 0;
  }
  uint8_t *w = out;
  putU16LE(w, (uint16_t)p.target_celsius_x10);
  *w++ = p.pump_percent;
  *w++ = p.fan_percent;
  *w++ = p.mode;
  *w++ = p.valid;
  return (size_t)(w - out);
}

bool decodeCommand(const uint8_t *in, size_t len, CommandPayload &out) {
  if (!in || len < sizeof(CommandPayload)) {
    return false;
  }
  out.target_celsius_x10 = (int16_t)getU16LE(in);
  out.pump_percent = in[2];
  out.fan_percent = in[3];
  out.mode = in[4];
  out.valid = in[5];
  return true;
}

size_t encodeStatus(uint8_t *out, size_t cap, const StatusPayload &p) {
  if (!out || cap < sizeof(StatusPayload)) {
    return 0;
  }
  uint8_t *w = out;
  putU32LE(w, p.uptime_ms);
  putU16LE(w, (uint16_t)p.coolant_c_x10);
  *w++ = p.pump_percent;
  *w++ = p.fan_percent;
  *w++ = p.safety_state;
  *w++ = p.faults_lo;
  *w++ = p.risk_score_pct;
  w[0] = p.reserved[0];
  w[1] = p.reserved[1];
  w[2] = p.reserved[2];
  w += 3;
  return (size_t)(w - out);
}

bool decodeStatus(const uint8_t *in, size_t len, StatusPayload &out) {
  if (!in || len < sizeof(StatusPayload)) {
    return false;
  }
  out.uptime_ms = getU32LE(in);
  out.coolant_c_x10 = (int16_t)getU16LE(in + 4);
  out.pump_percent = in[6];
  out.fan_percent = in[7];
  out.safety_state = in[8];
  out.faults_lo = in[9];
  out.risk_score_pct = in[10];
  out.reserved[0] = in[11];
  out.reserved[1] = in[12];
  out.reserved[2] = in[13];
  return true;
}

size_t encodeFault(uint8_t *out, size_t cap, const FaultPayload &p) {
  if (!out || cap < sizeof(FaultPayload)) {
    return 0;
  }
  uint8_t *w = out;
  putU16LE(w, p.fault_flags);
  *w++ = p.severity;
  *w++ = p.source;
  w[0] = p.reserved[0];
  w[1] = p.reserved[1];
  w[2] = p.reserved[2];
  w[3] = p.reserved[3];
  w += 4;
  return (size_t)(w - out);
}

bool decodeFault(const uint8_t *in, size_t len, FaultPayload &out) {
  if (!in || len < sizeof(FaultPayload)) {
    return false;
  }
  out.fault_flags = getU16LE(in);
  out.severity = in[2];
  out.source = in[3];
  out.reserved[0] = in[4];
  out.reserved[1] = in[5];
  out.reserved[2] = in[6];
  out.reserved[3] = in[7];
  return true;
}

size_t encodeAck(uint8_t *out, size_t cap, const AckPayload &p) {
  if (!out || cap < sizeof(AckPayload)) {
    return 0;
  }
  uint8_t *w = out;
  *w++ = p.acked_message_type;
  *w++ = p.status;
  putU16LE(w, p.reserved);
  return (size_t)(w - out);
}

bool decodeAck(const uint8_t *in, size_t len, AckPayload &out) {
  if (!in || len < sizeof(AckPayload)) {
    return false;
  }
  out.acked_message_type = in[0];
  out.status = in[1];
  out.reserved = getU16LE(in + 2);
  return true;
}

// ---------------------------------------------------------------------------
// LinkSupervisor
// ---------------------------------------------------------------------------
LinkSupervisor::LinkSupervisor()
    : _last_frame_ms(0)
    , _last_sequence(0)
    , _frame_count(0)
    , _timeout_ms(DEFAULT_HEARTBEAT_TIMEOUT_MS)
    , _have_sequence(false)
    , _seq_anomaly(false) {
}

void LinkSupervisor::onFrameReceived(MessageType type, uint32_t sequence) {
  uint32_t now = millis();
  _last_frame_ms = now;
  _frame_count++;

  if (_have_sequence) {
    // Accept same-or-forward sequence. A strict rollback (sequence < last with no
    // wrap assumption) is flagged as an anomaly for the host to inspect; it does not
    // by itself kill the link because short retransmits can legitimately repeat a seq.
    if (sequence < _last_sequence) {
      _seq_anomaly = true;
    } else {
      _seq_anomaly = false;
    }
  }
  _last_sequence = sequence;
  _have_sequence = true;
  (void)type;
}

bool LinkSupervisor::isAlive(uint32_t now_ms) const {
  if (_last_frame_ms == 0) {
    return false;
  }
  uint32_t delta = now_ms - _last_frame_ms;
  return delta <= _timeout_ms;
}

uint32_t LinkSupervisor::lastFrameAgeMs(uint32_t now_ms) const {
  if (_last_frame_ms == 0) {
    return UINT32_MAX;
  }
  return now_ms - _last_frame_ms;
}

}  // namespace ewp_link

#include "routes.h"
#include "web_assets.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <AsyncWebSocket.h>
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <Preferences.h>
#include <Update.h>
#include <WiFi.h>
#include <esp_netif.h>
#include <esp_system.h>
#include <esp_task_wdt.h>
#include <esp_wifi.h>
#include <math.h>

/*
 * SmartCooling Console V3
 * Target: ESP32-S3 Super Mini HW-747
 */

// Pinout
#define PIN_NTC 1
#define PIN_SSR 2
#define PIN_PWM 4
#define PIN_ECU 6

// Timing and output
#define TICK_MS 500UL
#define PWM_FREQ 15000
#define PWM_RES 8
#define PWM_CHAN 0
#define AP_IDLE_OFF_MS 300000UL
#define STATUS_PUSH_MS 1000UL
#define ADC_SAMPLE_COUNT 17
#define ADC_TRIM_COUNT 3
#define SENSOR_GOOD_LIMIT 2
#define SENSOR_BAD_LIMIT 3
#define SENSOR_SPIKE_LIMIT 3
#define CONTROL_WDT_TIMEOUT_S 4

static constexpr const char *APP_VERSION = "2026.06.26-v3";
static constexpr const char *AP_SSID = "EWP-SYSTEM-PRO";
static constexpr const char *DOMAIN_HOST = "smartcooling.local";
static constexpr const char *MDNS_HOST = "smartcooling";
static constexpr const char *DEFAULT_WEB_PASSWORD = "12345678";
static constexpr const char *RECOVERY_PIN = "747747";

static const IPAddress AP_IP(10, 74, 7, 1);
static const IPAddress AP_GW(10, 74, 7, 1);
static const IPAddress AP_MASK(255, 255, 255, 0);
static const IPAddress AP_LEASE_START(10, 74, 7, 20);
static const IPAddress AP_LEASE_END(10, 74, 7, 80);

esp_netif_t *get_esp_interface_netif(esp_interface_t interface);

static constexpr float ADC_MAX_COUNTS = 4095.0f;
static constexpr float NTC_MIN_VALID_C = -30.0f;
static constexpr float NTC_MAX_VALID_C = 170.0f;

enum FaultBits : uint32_t {
  FAULT_COOLANT_SENSOR = 1 << 0,
  FAULT_UNDERVOLTAGE = 1 << 1,
  FAULT_PUMP_CURRENT = 1 << 2,
  FAULT_FAN_CURRENT = 1 << 3,
  FAULT_DRIVER_HEAT = 1 << 4,
  FAULT_CONTROL_STALE = 1 << 5,
  FAULT_CONFIG_RECOVERED = 1 << 6,
};

enum SafetyState : uint8_t {
  SAFETY_BOOT = 0,
  SAFETY_NORMAL = 1,
  SAFETY_SENSOR_FAILSAFE = 2,
  SAFETY_CRITICAL_FAILSAFE = 3,
};

struct ManualStep {
  float temperature_c;
  int fan_pct;
  int fan_on_s;
  int pump_pct;
  int pump_on_s;
};

struct Config {
  int mode = 0;
  int force_pump = 0;
  int force_fan = 0;
  int auto_target_c = 80;
  int auto_full_c = 110;
  int min_temp_c = 70;
  int window_s = 30;
  int slew_pct_s = 5;
  int sensor_mask = 65;
  float battery_cutoff_v = 12.3f;
  int run_on_s = 120;
  int run_on_stop_c = 75;
  int critical_temp_c = 120;
  int warning_temp_c = 108;
  int low_voltage_delay_ms = 3000;
  float ntc_series_ohm = 10000.0f;
  float ntc_nominal_ohm = 10000.0f;
  float ntc_beta_k = 3950.0f;
  float ntc_nominal_c = 25.0f;
  float temp_offset_c = 0.0f;
  float temp_gain = 1.0f;
  ManualStep manual_steps[18];
};

struct RuntimeState {
  float ema = 74.0f;
  float v = 0.0f;
  float flux = 0.0f;
  float trend = 0.0f;
  float lastRawTemp = 0.0f;
  float adcRaw = 0.0f;
  float adcSpread = 0.0f;
  float noiseC = 0.0f;
  float predTemp60 = 74.0f;
  float riskScore = 0.0f;
  float siConfidence = 0.0f;
  bool coolantValid = false;
  bool filterReady = false;
  uint8_t sensorGoodStreak = 0;
  uint8_t sensorBadStreak = 0;
  uint8_t spikeRejects = 0;
  uint32_t sensorReadCount = 0;
  uint32_t sensorInvalidCount = 0;
  uint32_t sensorSpikeCount = 0;
  int pump = 0;
  int fan = 0;
  bool ssrActive = false;
  bool ecuSignal = false;
  int ecuHighCount = 0;
  int ecuLowCount = 0;
  uint32_t faults = 0;
  uint32_t sequence = 1;
  SafetyState safetyState = SAFETY_BOOT;
  bool outputsForced = false;
};

struct EventEntry {
  uint32_t sequence = 0;
  uint32_t uptime_ms = 0;
  uint8_t level = 0;
  char message[72] = {0};
};

struct Lockout {
  uint8_t fails = 0;
  uint32_t firstFailMs = 0;
  uint32_t lockedUntilMs = 0;
};

Config cfg;
Config previewCfg;
RuntimeState state;
EventEntry events[32];
uint8_t eventHead = 0;
uint32_t eventSeq = 1;

Preferences prefs;
AsyncWebServer server(80);
AsyncWebSocket ws(ROUTE_WS);
DNSServer dnsServer;

String webPassword = DEFAULT_WEB_PASSWORD;
String sessionToken;
uint32_t sessionExpiryMs = 0;
Lockout loginLockout;
Lockout recoveryLockout;
bool apRunning = false;
uint32_t apLastEmptyMs = 0;
uint32_t lastTickMs = 0;
uint32_t lastWsPushMs = 0;
String otaError;
uint32_t bootCount = 0;
esp_reset_reason_t bootResetReason = ESP_RST_UNKNOWN;
bool configRecovered = false;

void setDefaultConfig(Config &c);
void loadConfig();
void saveConfig();
String configPayload(const Config &source);
uint32_t crc32String(const String &input);
bool loadConfigSlot(const char *key, Config &out, uint32_t &seq);
bool saveConfigSlot(const char *key, const Config &source, uint32_t seq);
const char *resetReasonText(esp_reset_reason_t reason);
const char *safetyStateText(SafetyState safetyState);
void applyConfigFromJson(JsonVariantConst src, Config &target);
void configToJson(JsonDocument &doc, const Config &source);
void statusToJson(JsonDocument &doc);
void sendJson(AsyncWebServerRequest *request, JsonDocument &doc, int code = 200);
void sendError(AsyncWebServerRequest *request, int code, const char *message);
bool hostAllowed(AsyncWebServerRequest *request);
bool requireHost(AsyncWebServerRequest *request);
bool isAuthorized(AsyncWebServerRequest *request);
bool requireAuth(AsyncWebServerRequest *request);
String requestBody(uint8_t *data, size_t len);
void registerJsonPost(const char *route, ArRequestHandlerFunction onRequestBody);
void addEvent(uint8_t level, const char *message);
void setupNetwork();
void startAp();
bool configureApDhcpLeaseRange();
void stopAp();
void maintainAp();
void setupRoutes();
void setupWebSocket();
void engine();
void updateThermalModel();
float clampFloat(float value, float low, float high);
float smoothStep01(float value);
float smoothRange(float edge0, float edge1, float value);
int approachInt(int current, int target, int upStep, int downStep);
float readAdcStable();
float adcToTemp(float raw);
float readTemp();
void handleSSR();
void pushStatus();
String makeToken();
String boardSerial();
bool lockedOut(const Lockout &lockout);
void noteFailure(Lockout &lockout, uint8_t threshold, uint32_t lockMs, uint32_t maxLockMs);
void noteSuccess(Lockout &lockout);
void handleLogin(AsyncWebServerRequest *request, JsonVariantConst body);
void handleRecover(AsyncWebServerRequest *request, JsonVariantConst body);
void handlePassword(AsyncWebServerRequest *request, JsonVariantConst body);
void handlePreview(AsyncWebServerRequest *request, JsonVariantConst body);
void handleSave(AsyncWebServerRequest *request);
void handleConfig(AsyncWebServerRequest *request);
void handleStatus(AsyncWebServerRequest *request);
void handleDevice(AsyncWebServerRequest *request);
void handleEvents(AsyncWebServerRequest *request);
void handleRgbGet(AsyncWebServerRequest *request);
void handleRgbPost(AsyncWebServerRequest *request, JsonVariantConst body);
void handleOtaRequest(AsyncWebServerRequest *request);
void handleOtaUpload(AsyncWebServerRequest *request, const String &filename,
                     size_t index, uint8_t *data, size_t len, bool final);
int interpolateManual(float temp, bool fan);
void applyOutputs(int targetPump, int targetFan);

void setDefaultConfig(Config &c) {
  c = Config();
  for (int i = 0; i < 18; ++i) {
    c.manual_steps[i].temperature_c = i * 8.0f;
    c.manual_steps[i].fan_pct = min(100, i * 6);
    c.manual_steps[i].fan_on_s = i ? 10 : 0;
    c.manual_steps[i].pump_pct = min(100, i * 5);
    c.manual_steps[i].pump_on_s = i ? 10 : 0;
  }
}

void loadConfig() {
  setDefaultConfig(cfg);
  prefs.begin("sc", false);
  bootCount = prefs.getUInt("boot_count", 0) + 1;
  prefs.putUInt("boot_count", bootCount);
  webPassword = prefs.getString("webpass", DEFAULT_WEB_PASSWORD);

  Config slotA;
  Config slotB;
  setDefaultConfig(slotA);
  setDefaultConfig(slotB);
  uint32_t seqA = 0;
  uint32_t seqB = 0;
  bool okA = loadConfigSlot("cfg_a", slotA, seqA);
  bool okB = loadConfigSlot("cfg_b", slotB, seqB);
  if (okA || okB) {
    cfg = (okB && (!okA || seqB >= seqA)) ? slotB : slotA;
  } else {
    String saved = prefs.getString("cfg", "");
    if (saved.length()) {
      DynamicJsonDocument doc(8192);
      if (deserializeJson(doc, saved) == DeserializationError::Ok) {
        applyConfigFromJson(doc.as<JsonVariantConst>(), cfg);
        configRecovered = true;
      } else {
        configRecovered = true;
      }
    }
    saveConfigSlot("cfg_a", cfg, 1);
    prefs.putUInt("cfg_seq", 1);
  }
  if (!okA && !okB) {
    state.faults |= FAULT_CONFIG_RECOVERED;
  }
  previewCfg = cfg;
}

void saveConfig() {
  Config clean = previewCfg;
  DynamicJsonDocument normalized(8192);
  configToJson(normalized, clean);
  applyConfigFromJson(normalized.as<JsonVariantConst>(), clean);

  uint32_t seq = prefs.getUInt("cfg_seq", 1) + 1;
  String active = prefs.getString("cfg_active", "a");
  const char *slot = active == "a" ? "cfg_b" : "cfg_a";
  if (saveConfigSlot(slot, clean, seq)) {
    prefs.putUInt("cfg_seq", seq);
    prefs.putString("cfg_active", active == "a" ? "b" : "a");
    cfg = clean;
    previewCfg = clean;
    state.faults &= ~FAULT_CONFIG_RECOVERED;
  } else {
    state.faults |= FAULT_CONFIG_RECOVERED;
    addEvent(2, "Config save failed");
  }
}

String configPayload(const Config &source) {
  DynamicJsonDocument doc(8192);
  configToJson(doc, source);
  String out;
  serializeJson(doc, out);
  return out;
}

uint32_t crc32String(const String &input) {
  uint32_t crc = 0xFFFFFFFFUL;
  for (size_t i = 0; i < input.length(); ++i) {
    crc ^= (uint8_t)input[i];
    for (uint8_t bit = 0; bit < 8; ++bit) {
      crc = (crc >> 1) ^ (0xEDB88320UL & (0UL - (crc & 1UL)));
    }
  }
  return ~crc;
}

bool loadConfigSlot(const char *key, Config &out, uint32_t &seq) {
  String saved = prefs.getString(key, "");
  if (saved.length() == 0) {
    return false;
  }
  DynamicJsonDocument envelope(10000);
  if (deserializeJson(envelope, saved) != DeserializationError::Ok) {
    configRecovered = true;
    return false;
  }
  seq = envelope["seq"] | 0;
  uint32_t storedCrc = envelope["crc"] | 0;
  JsonVariantConst cfgJson = envelope["cfg"];
  if (cfgJson.isNull() || seq == 0) {
    configRecovered = true;
    return false;
  }
  String payload;
  serializeJson(cfgJson, payload);
  if (crc32String(payload) != storedCrc) {
    configRecovered = true;
    return false;
  }
  applyConfigFromJson(cfgJson, out);
  return true;
}

bool saveConfigSlot(const char *key, const Config &source, uint32_t seq) {
  String payload = configPayload(source);
  DynamicJsonDocument cfgDoc(8192);
  if (deserializeJson(cfgDoc, payload) != DeserializationError::Ok) {
    return false;
  }
  DynamicJsonDocument envelope(10000);
  envelope["seq"] = seq;
  envelope["crc"] = crc32String(payload);
  envelope["cfg"].set(cfgDoc.as<JsonVariantConst>());
  String out;
  serializeJson(envelope, out);
  return prefs.putString(key, out) == out.length();
}

const char *resetReasonText(esp_reset_reason_t reason) {
  switch (reason) {
    case ESP_RST_POWERON:
      return "power_on";
    case ESP_RST_EXT:
      return "external_reset";
    case ESP_RST_SW:
      return "software_reset";
    case ESP_RST_PANIC:
      return "panic";
    case ESP_RST_INT_WDT:
      return "interrupt_watchdog";
    case ESP_RST_TASK_WDT:
      return "task_watchdog";
    case ESP_RST_WDT:
      return "watchdog";
    case ESP_RST_DEEPSLEEP:
      return "deep_sleep";
    case ESP_RST_BROWNOUT:
      return "brownout";
    case ESP_RST_SDIO:
      return "sdio";
    default:
      return "unknown";
  }
}

const char *safetyStateText(SafetyState safetyState) {
  switch (safetyState) {
    case SAFETY_NORMAL:
      return "normal";
    case SAFETY_SENSOR_FAILSAFE:
      return "sensor_failsafe";
    case SAFETY_CRITICAL_FAILSAFE:
      return "critical_failsafe";
    case SAFETY_BOOT:
    default:
      return "boot";
  }
}

void applyConfigFromJson(JsonVariantConst src, Config &target) {
  target.mode = src["mode"] | target.mode;
  target.force_pump = src["force_pump"] | target.force_pump;
  target.force_fan = src["force_fan"] | target.force_fan;
  target.auto_target_c = src["auto_target_c"] | target.auto_target_c;
  target.auto_full_c = src["auto_full_c"] | target.auto_full_c;
  target.min_temp_c = src["min_temp_c"] | target.min_temp_c;
  target.window_s = src["window_s"] | target.window_s;
  target.slew_pct_s = src["slew_pct_s"] | target.slew_pct_s;
  target.sensor_mask = src["sensor_mask"] | target.sensor_mask;
  target.battery_cutoff_v = src["battery_cutoff_v"] | target.battery_cutoff_v;
  target.run_on_s = src["run_on_s"] | target.run_on_s;
  target.run_on_stop_c = src["run_on_stop_c"] | target.run_on_stop_c;
  target.critical_temp_c = src["critical_temp_c"] | target.critical_temp_c;
  target.warning_temp_c = src["warning_temp_c"] | target.warning_temp_c;
  target.low_voltage_delay_ms = src["low_voltage_delay_ms"] | target.low_voltage_delay_ms;
  target.ntc_series_ohm = src["ntc_series_ohm"] | target.ntc_series_ohm;
  target.ntc_nominal_ohm = src["ntc_nominal_ohm"] | target.ntc_nominal_ohm;
  target.ntc_beta_k = src["ntc_beta_k"] | target.ntc_beta_k;
  target.ntc_nominal_c = src["ntc_nominal_c"] | target.ntc_nominal_c;
  target.temp_offset_c = src["temp_offset_c"] | target.temp_offset_c;
  target.temp_gain = src["temp_gain"] | target.temp_gain;

  JsonArrayConst steps = src["manual_steps"].as<JsonArrayConst>();
  if (!steps.isNull() && steps.size() == 18) {
    for (int i = 0; i < 18; ++i) {
      JsonVariantConst s = steps[i];
      target.manual_steps[i].temperature_c = clampFloat(s["temperature_c"] | target.manual_steps[i].temperature_c,
                                                        0.0f, 160.0f);
      target.manual_steps[i].fan_pct = constrain(s["fan_pct"] | target.manual_steps[i].fan_pct, 0, 100);
      target.manual_steps[i].fan_on_s = constrain(s["fan_on_s"] | target.manual_steps[i].fan_on_s, 0, 120);
      target.manual_steps[i].pump_pct = constrain(s["pump_pct"] | target.manual_steps[i].pump_pct, 0, 100);
      target.manual_steps[i].pump_on_s = constrain(s["pump_on_s"] | target.manual_steps[i].pump_on_s, 0, 120);
    }
  }
  target.mode = constrain(target.mode, 0, 4);
  target.force_pump = constrain(target.force_pump, 0, 2);
  target.force_fan = constrain(target.force_fan, 0, 2);
  target.window_s = constrain(target.window_s, 10, 60);
  target.slew_pct_s = constrain(target.slew_pct_s, 1, 20);
  target.battery_cutoff_v = clampFloat(target.battery_cutoff_v, 9.0f, 16.0f);
  target.run_on_s = constrain(target.run_on_s, 0, 600);
  target.run_on_stop_c = constrain(target.run_on_stop_c, 20, 120);
  target.low_voltage_delay_ms = constrain(target.low_voltage_delay_ms, 500, 30000);
  target.ntc_series_ohm = clampFloat(target.ntc_series_ohm, 1000.0f, 100000.0f);
  target.ntc_nominal_ohm = clampFloat(target.ntc_nominal_ohm, 1000.0f, 100000.0f);
  target.ntc_beta_k = clampFloat(target.ntc_beta_k, 2500.0f, 5500.0f);
  target.ntc_nominal_c = clampFloat(target.ntc_nominal_c, 0.0f, 50.0f);
  target.temp_offset_c = clampFloat(target.temp_offset_c, -20.0f, 20.0f);
  target.temp_gain = clampFloat(target.temp_gain, 0.75f, 1.25f);
  target.auto_target_c = constrain(target.auto_target_c, 35, 109);
  target.auto_full_c = constrain(target.auto_full_c, target.auto_target_c + 1, 130);
  target.min_temp_c = constrain(target.min_temp_c, 20, target.auto_target_c);
  target.warning_temp_c = constrain(target.warning_temp_c, 35, 150);
  target.critical_temp_c = constrain(target.critical_temp_c, target.warning_temp_c + 1, 160);
  for (int i = 1; i < 18; ++i) {
    target.manual_steps[i].temperature_c =
        clampFloat(max(target.manual_steps[i].temperature_c,
                       target.manual_steps[i - 1].temperature_c + 0.5f),
                   0.0f, 160.0f);
  }
}

void configToJson(JsonDocument &doc, const Config &source) {
  doc["mode"] = source.mode;
  doc["force_pump"] = source.force_pump;
  doc["force_fan"] = source.force_fan;
  doc["auto_target_c"] = source.auto_target_c;
  doc["auto_full_c"] = source.auto_full_c;
  doc["min_temp_c"] = source.min_temp_c;
  doc["window_s"] = source.window_s;
  doc["slew_pct_s"] = source.slew_pct_s;
  doc["sensor_mask"] = source.sensor_mask;
  doc["battery_cutoff_v"] = source.battery_cutoff_v;
  doc["run_on_s"] = source.run_on_s;
  doc["run_on_stop_c"] = source.run_on_stop_c;
  doc["critical_temp_c"] = source.critical_temp_c;
  doc["warning_temp_c"] = source.warning_temp_c;
  doc["low_voltage_delay_ms"] = source.low_voltage_delay_ms;
  doc["ntc_series_ohm"] = source.ntc_series_ohm;
  doc["ntc_nominal_ohm"] = source.ntc_nominal_ohm;
  doc["ntc_beta_k"] = source.ntc_beta_k;
  doc["ntc_nominal_c"] = source.ntc_nominal_c;
  doc["temp_offset_c"] = source.temp_offset_c;
  doc["temp_gain"] = source.temp_gain;
  JsonArray steps = doc.createNestedArray("manual_steps");
  for (int i = 0; i < 18; ++i) {
    JsonObject s = steps.createNestedObject();
    s["temperature_c"] = source.manual_steps[i].temperature_c;
    s["fan_pct"] = source.manual_steps[i].fan_pct;
    s["fan_on_s"] = source.manual_steps[i].fan_on_s;
    s["pump_pct"] = source.manual_steps[i].pump_pct;
    s["pump_on_s"] = source.manual_steps[i].pump_on_s;
  }
}

void statusToJson(JsonDocument &doc) {
  uint8_t clients = apRunning ? WiFi.softAPgetStationNum() : 0;
  uint32_t apAutoOff = 0;
  if (apRunning && clients == 0) {
    uint32_t elapsed = millis() - apLastEmptyMs;
    apAutoOff = elapsed >= AP_IDLE_OFF_MS ? 0 : (AP_IDLE_OFF_MS - elapsed) / 1000;
  }

  doc["webapp_version"] = APP_VERSION;
  doc["firmware_version"] = APP_VERSION;
  doc["uptime_ms"] = millis();
  doc["coolant_c"] = state.ema;
  doc["coolant_valid"] = state.coolantValid;
  doc["battery_v"] = 0.0;
  doc["battery_valid"] = false;
  doc["pump_pct"] = state.pump;
  doc["fan_pct"] = state.fan;
  doc["pump_current_a"] = 0.0;
  doc["fan_current_a"] = 0.0;
  doc["acc_on"] = true;
  doc["mode"] = previewCfg.mode;
  doc["faults"] = state.faults;
  doc["run_on"] = false;
  doc["preview"] = true;
  doc["outputs_armed"] = state.coolantValid && !state.outputsForced;
  doc["outputs_forced"] = state.outputsForced;
  doc["pump_power_enabled"] = state.pump > 0;
  doc["fan_power_enabled"] = state.fan > 0;
  doc["ssr_active"] = state.ssrActive;
  doc["ecu_request"] = state.ecuSignal;
  doc["si_coolant_rate_c_s"] = state.trend;
  doc["si_pred_temp_60_c"] = state.predTemp60;
  doc["si_risk_score"] = (int)state.riskScore;
  doc["si_confidence"] = (int)state.siConfidence;
  doc["sensor_quality_pct"] = (int)state.siConfidence;
  doc["sensor_degraded"] = state.siConfidence < 80.0f || state.sensorBadStreak > 0 || state.spikeRejects > 0;
  doc["si_noise_c"] = state.noiseC;
  doc["si_adc_raw"] = state.adcRaw;
  doc["si_adc_spread"] = state.adcSpread;
  doc["safety_state"] = safetyStateText(state.safetyState);
  doc["watchdog_s"] = CONTROL_WDT_TIMEOUT_S;
  doc["boot_count"] = bootCount;
  doc["reset_reason"] = resetReasonText(bootResetReason);
  doc["config_recovered"] = configRecovered;
  doc["session_remaining_s"] = sessionExpiryMs > millis() ? (sessionExpiryMs - millis()) / 1000 : 0;
  doc["ssid"] = AP_SSID;
  doc["ap_ip"] = AP_IP.toString();
  doc["wifi_ap_running"] = apRunning;
  doc["wifi_channel"] = SMARTCOOLING_WIFI_AP_CHANNEL;
  doc["wifi_clients"] = clients;
  doc["wifi_dhcp_clients"] = clients;
  doc["wifi_dhcp_start"] = AP_LEASE_START.toString();
  doc["wifi_dhcp_end"] = AP_LEASE_END.toString();
  doc["ap_auto_off_s"] = apAutoOff;
  doc["local_only_network"] = true;
  doc["heap_free"] = ESP.getFreeHeap();
  doc["heap_min_free"] = ESP.getMinFreeHeap();
  doc["internal_free"] = ESP.getFreeHeap();
  doc["security_faults"] = state.faults != 0 || state.outputsForced || configRecovered;
  doc["serial"] = boardSerial();
  doc["route_mode"] = "random";
  JsonObject health = doc.createNestedObject("sensor_health");
  health["ntc_primary_reads"] = state.sensorReadCount;
  health["ntc_primary_failures"] = state.sensorInvalidCount;
  health["ntc_spike_rejects"] = state.sensorSpikeCount;
  health["good_streak"] = state.sensorGoodStreak;
  health["bad_streak"] = state.sensorBadStreak;
  health["filter_ready"] = state.filterReady;
}

void sendJson(AsyncWebServerRequest *request, JsonDocument &doc, int code) {
  String out;
  serializeJson(doc, out);
  AsyncWebServerResponse *response =
      request->beginResponse(code, "application/json; charset=utf-8", out);
  response->addHeader("Cache-Control", "no-store");
  request->send(response);
}

void sendError(AsyncWebServerRequest *request, int code, const char *message) {
  StaticJsonDocument<160> doc;
  doc["ok"] = false;
  doc["error"] = message;
  sendJson(request, doc, code);
}

bool hostAllowed(AsyncWebServerRequest *request) {
  String host = request->host();
  host.toLowerCase();
  int colon = host.indexOf(':');
  if (colon >= 0) {
    host = host.substring(0, colon);
  }
  return host == DOMAIN_HOST || host == MDNS_HOST || host.length() == 0;
}

bool requireHost(AsyncWebServerRequest *request) {
  if (hostAllowed(request)) {
    return true;
  }
  request->send(421, "text/html; charset=utf-8",
                "<!doctype html><meta name=viewport content='width=device-width,initial-scale=1'>"
                "<title>SmartCooling</title><body style='font-family:system-ui;padding:2rem'>"
                "<h1>Gunakan smartcooling.local</h1>"
                "<p>Akses melalui IP tidak dibenarkan untuk WebApp ini.</p></body>");
  return false;
}

bool isAuthorized(AsyncWebServerRequest *request) {
  if (sessionToken.length() == 0 || millis() > sessionExpiryMs) {
    return false;
  }
  String header = request->header("Authorization");
  if (!header.startsWith("Bearer ")) {
    return false;
  }
  return header.substring(7) == sessionToken;
}

bool requireAuth(AsyncWebServerRequest *request) {
  if (!requireHost(request)) {
    return false;
  }
  if (!isAuthorized(request)) {
    sendError(request, 401, "unauthorized");
    return false;
  }
  sessionExpiryMs = millis() + 30UL * 60UL * 1000UL;
  return true;
}

String requestBody(uint8_t *data, size_t len) {
  String body;
  body.reserve(len + 1);
  for (size_t i = 0; i < len; ++i) {
    body += (char)data[i];
  }
  return body;
}

void registerJsonPost(const char *route,
                      std::function<void(AsyncWebServerRequest *, JsonVariantConst)> handler) {
  server.on(route, HTTP_POST, [](AsyncWebServerRequest *request) {},
            nullptr,
            [handler](AsyncWebServerRequest *request, uint8_t *data, size_t len,
                      size_t index, size_t total) {
              if (index != 0 || len != total) {
                sendError(request, 413, "payload");
                return;
              }
              DynamicJsonDocument doc(8192);
              DeserializationError err = deserializeJson(doc, requestBody(data, len));
              if (err) {
                sendError(request, 400, "json");
                return;
              }
              handler(request, doc.as<JsonVariantConst>());
            });
}

void addEvent(uint8_t level, const char *message) {
  EventEntry &e = events[eventHead];
  e.sequence = eventSeq++;
  e.uptime_ms = millis();
  e.level = level;
  strlcpy(e.message, message, sizeof(e.message));
  eventHead = (eventHead + 1) % 32;
}

void setupNetwork() {
  WiFi.persistent(false);
  WiFi.mode(WIFI_AP);
  startAp();
}

void startAp() {
  if (apRunning) {
    return;
  }
  WiFi.softAPdisconnect(true);
  delay(50);
  WiFi.softAPConfig(AP_IP, AP_GW, AP_MASK, AP_LEASE_START);
  configureApDhcpLeaseRange();
  bool ok = WiFi.softAP(AP_SSID, nullptr, SMARTCOOLING_WIFI_AP_CHANNEL, 0, 4);
  apRunning = ok;
  apLastEmptyMs = millis();
  if (ok) {
    dnsServer.setErrorReplyCode(DNSReplyCode::NonExistentDomain);
    dnsServer.setTTL(15);
    dnsServer.start(53, DOMAIN_HOST, AP_IP);
    if (MDNS.begin(MDNS_HOST)) {
      MDNS.addService("http", "tcp", 80);
    }
    addEvent(0, "AP started");
  } else {
    addEvent(2, "AP start failed");
  }
}

bool configureApDhcpLeaseRange() {
  esp_netif_t *netif = get_esp_interface_netif(ESP_IF_WIFI_AP);
  if (!netif) {
    addEvent(1, "AP DHCP netif unavailable");
    return false;
  }

  esp_err_t err = esp_netif_dhcps_stop(netif);
  if (err != ESP_OK && err != ESP_ERR_ESP_NETIF_DHCP_ALREADY_STOPPED) {
    addEvent(1, "AP DHCP stop failed");
    return false;
  }

  dhcps_lease_t lease = {};
  lease.enable = true;
  lease.start_ip.addr = static_cast<uint32_t>(AP_LEASE_START);
  lease.end_ip.addr = static_cast<uint32_t>(AP_LEASE_END);

  err = esp_netif_dhcps_option(netif, ESP_NETIF_OP_SET, ESP_NETIF_REQUESTED_IP_ADDRESS,
                               &lease, sizeof(lease));
  if (err != ESP_OK) {
    addEvent(1, "AP DHCP lease failed");
    esp_netif_dhcps_start(netif);
    return false;
  }

  err = esp_netif_dhcps_start(netif);
  if (err != ESP_OK) {
    addEvent(1, "AP DHCP start failed");
    return false;
  }
  return true;
}

void stopAp() {
  if (!apRunning) {
    return;
  }
  dnsServer.stop();
  MDNS.end();
  WiFi.softAPdisconnect(true);
  apRunning = false;
  addEvent(1, "AP stopped after idle");
}

void maintainAp() {
  if (!apRunning) {
    return;
  }
  dnsServer.processNextRequest();
  uint8_t clients = WiFi.softAPgetStationNum();
  if (clients > 0) {
    apLastEmptyMs = millis();
    return;
  }
  if (millis() - apLastEmptyMs >= AP_IDLE_OFF_MS) {
    stopAp();
  }
}

void setupRoutes() {
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!requireHost(request)) {
      return;
    }
    AsyncWebServerResponse *response = request->beginResponse(
        200, "text/html; charset=utf-8",
        reinterpret_cast<const uint8_t *>(index_html), sizeof(index_html) - 1);
    response->addHeader("Cache-Control", "no-store");
    request->send(response);
  });

  server.on(ROUTE_DEVICE, HTTP_GET, handleDevice);
  server.on(ROUTE_STATUS, HTTP_GET, handleStatus);
  server.on(ROUTE_CONFIG, HTTP_GET, handleConfig);
  server.on(ROUTE_EVENTS, HTTP_GET, handleEvents);
  server.on(ROUTE_RGB, HTTP_GET, handleRgbGet);
  server.on(ROUTE_SAVE, HTTP_POST, handleSave);
  server.on(ROUTE_REBOOT, HTTP_POST, [](AsyncWebServerRequest *request) {
    if (!requireAuth(request)) {
      return;
    }
    request->send(200, "application/json", "{\"ok\":true}");
    delay(300);
    ESP.restart();
  });

  registerJsonPost(ROUTE_LOGIN, handleLogin);
  registerJsonPost(ROUTE_RECOVER, handleRecover);
  registerJsonPost(ROUTE_PASSWORD, handlePassword);
  registerJsonPost(ROUTE_PREVIEW, handlePreview);
  registerJsonPost(ROUTE_RGB, handleRgbPost);

  server.on(ROUTE_OTA, HTTP_POST, handleOtaRequest, handleOtaUpload);

  server.onNotFound([](AsyncWebServerRequest *request) {
    if (!requireHost(request)) {
      return;
    }
    sendError(request, 404, "not_found");
  });
}

void setupWebSocket() {
  ws.handleHandshake([](AsyncWebServerRequest *request) {
    if (!hostAllowed(request)) {
      return false;
    }
    if (!request->hasParam("token")) {
      return false;
    }
    String token = request->getParam("token")->value();
    return sessionToken.length() && token == sessionToken && millis() < sessionExpiryMs;
  });
  ws.onEvent([](AsyncWebSocket *server, AsyncWebSocketClient *client,
                AwsEventType type, void *arg, uint8_t *data, size_t len) {
    (void)server;
    (void)client;
    (void)arg;
    (void)data;
    (void)len;
    if (type == WS_EVT_CONNECT) {
      addEvent(0, "WebSocket connected");
    }
  });
  server.addHandler(&ws);
}

void handleLogin(AsyncWebServerRequest *request, JsonVariantConst body) {
  if (!requireHost(request)) {
    return;
  }
  if (lockedOut(loginLockout)) {
    sendError(request, 423, "locked");
    return;
  }
  String password = body["password"] | "";
  if (password == webPassword) {
    noteSuccess(loginLockout);
    sessionToken = makeToken();
    sessionExpiryMs = millis() + 30UL * 60UL * 1000UL;
    StaticJsonDocument<256> doc;
    doc["token"] = sessionToken;
    doc["session_remaining_s"] = 1800;
    addEvent(0, "WebApp login");
    sendJson(request, doc);
    return;
  }
  noteFailure(loginLockout, 5, 60000UL, 15UL * 60UL * 1000UL);
  addEvent(1, "Login failed");
  sendError(request, 401, "login_failed");
}

void handleRecover(AsyncWebServerRequest *request, JsonVariantConst body) {
  if (!requireHost(request)) {
    return;
  }
  if (lockedOut(recoveryLockout)) {
    sendError(request, 423, "locked");
    return;
  }
  String pin = body["pin"] | "";
  String password = body["password"] | "";
  if (pin == RECOVERY_PIN && password.length() >= 8 && password.length() <= 63) {
    webPassword = password;
    prefs.putString("webpass", webPassword);
    sessionToken = "";
    noteSuccess(recoveryLockout);
    addEvent(1, "Password recovered");
    StaticJsonDocument<80> doc;
    doc["ok"] = true;
    sendJson(request, doc);
    return;
  }
  noteFailure(recoveryLockout, 3, 10UL * 60UL * 1000UL, 10UL * 60UL * 1000UL);
  addEvent(1, "Recovery failed");
  sendError(request, 401, "recovery_failed");
}

void handlePassword(AsyncWebServerRequest *request, JsonVariantConst body) {
  if (!requireAuth(request)) {
    return;
  }
  String password = body["password"] | "";
  if (password.length() < 8 || password.length() > 63) {
    sendError(request, 400, "password_length");
    return;
  }
  webPassword = password;
  prefs.putString("webpass", webPassword);
  sessionToken = "";
  addEvent(1, "Password changed");
  StaticJsonDocument<80> doc;
  doc["ok"] = true;
  sendJson(request, doc);
}

void handlePreview(AsyncWebServerRequest *request, JsonVariantConst body) {
  if (!requireAuth(request)) {
    return;
  }
  Config next = previewCfg;
  applyConfigFromJson(body, next);
  previewCfg = next;
  StaticJsonDocument<80> doc;
  doc["ok"] = true;
  sendJson(request, doc);
}

void handleSave(AsyncWebServerRequest *request) {
  if (!requireAuth(request)) {
    return;
  }
  saveConfig();
  addEvent(0, "Config saved");
  StaticJsonDocument<80> doc;
  doc["ok"] = true;
  sendJson(request, doc);
}

void handleConfig(AsyncWebServerRequest *request) {
  if (!requireAuth(request)) {
    return;
  }
  DynamicJsonDocument doc(8192);
  configToJson(doc, previewCfg);
  sendJson(request, doc);
}

void handleStatus(AsyncWebServerRequest *request) {
  if (!requireAuth(request)) {
    return;
  }
  DynamicJsonDocument doc(4096);
  statusToJson(doc);
  sendJson(request, doc);
}

void handleDevice(AsyncWebServerRequest *request) {
  if (!requireHost(request)) {
    return;
  }
  StaticJsonDocument<512> doc;
  doc["serial"] = boardSerial();
  doc["webapp_version"] = APP_VERSION;
  doc["firmware_version"] = APP_VERSION;
  doc["domain"] = DOMAIN_HOST;
  doc["ssid"] = AP_SSID;
  doc["route_mode"] = "random";
  doc["ap_open"] = true;
  sendJson(request, doc);
}

void handleEvents(AsyncWebServerRequest *request) {
  if (!requireAuth(request)) {
    return;
  }
  DynamicJsonDocument doc(4096);
  JsonArray arr = doc.to<JsonArray>();
  for (int i = 0; i < 32; ++i) {
    int idx = (eventHead + i) % 32;
    if (events[idx].sequence == 0) {
      continue;
    }
    JsonObject e = arr.createNestedObject();
    e["sequence"] = events[idx].sequence;
    e["uptime_ms"] = events[idx].uptime_ms;
    e["level"] = events[idx].level;
    e["message"] = events[idx].message;
  }
  sendJson(request, doc);
}

void handleRgbGet(AsyncWebServerRequest *request) {
  if (!requireAuth(request)) {
    return;
  }
  StaticJsonDocument<256> doc;
  doc["ready"] = false;
  doc["effect"] = "unavailable";
  doc["hex"] = "#38C8FF";
  doc["theme"] = "system";
  doc["brightness12"] = 0;
  sendJson(request, doc);
}

void handleRgbPost(AsyncWebServerRequest *request, JsonVariantConst body) {
  (void)body;
  if (!requireAuth(request)) {
    return;
  }
  handleRgbGet(request);
}

void handleOtaRequest(AsyncWebServerRequest *request) {
  if (!requireAuth(request)) {
    return;
  }
  bool ok = !Update.hasError();
  if (ok) {
    request->send(200, "application/json", "{\"ok\":true,\"reboot\":true}");
    delay(300);
    ESP.restart();
  } else {
    request->send(500, "application/json", "{\"ok\":false,\"error\":\"ota\"}");
  }
}

void handleOtaUpload(AsyncWebServerRequest *request, const String &filename,
                     size_t index, uint8_t *data, size_t len, bool final) {
  (void)filename;
  if (!isAuthorized(request)) {
    return;
  }
  if (index == 0) {
    otaError = "";
    if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
      otaError = "begin";
    }
  }
  if (otaError.length() == 0 && Update.write(data, len) != len) {
    otaError = "write";
  }
  if (final && otaError.length() == 0) {
    if (!Update.end(true)) {
      otaError = "end";
    } else {
      addEvent(1, "OTA received");
    }
  }
}

float clampFloat(float value, float low, float high) {
  if (value < low) {
    return low;
  }
  if (value > high) {
    return high;
  }
  return value;
}

float smoothStep01(float value) {
  float x = clampFloat(value, 0.0f, 1.0f);
  return x * x * (3.0f - 2.0f * x);
}

float smoothRange(float edge0, float edge1, float value) {
  float span = max(0.001f, edge1 - edge0);
  return smoothStep01((value - edge0) / span);
}

int approachInt(int current, int target, int upStep, int downStep) {
  int delta = target - current;
  int limit = delta >= 0 ? upStep : downStep;
  return current + constrain(delta, -limit, limit);
}

float readAdcStable() {
  uint16_t samples[ADC_SAMPLE_COUNT];
  for (uint8_t i = 0; i < ADC_SAMPLE_COUNT; ++i) {
    uint16_t value = analogRead(PIN_NTC);
    uint8_t j = i;
    while (j > 0 && samples[j - 1] > value) {
      samples[j] = samples[j - 1];
      --j;
    }
    samples[j] = value;
    delayMicroseconds(120);
  }

  uint32_t sum = 0;
  for (uint8_t i = ADC_TRIM_COUNT; i < ADC_SAMPLE_COUNT - ADC_TRIM_COUNT; ++i) {
    sum += samples[i];
  }
  uint8_t kept = ADC_SAMPLE_COUNT - (ADC_TRIM_COUNT * 2);
  state.adcRaw = (float)sum / (float)kept;
  state.adcSpread = (float)(samples[ADC_SAMPLE_COUNT - 1] - samples[0]);
  return state.adcRaw;
}

float adcToTemp(float raw) {
  if (raw <= 2.0f || raw >= ADC_MAX_COUNTS - 2.0f) {
    return NAN;
  }
  float res = previewCfg.ntc_series_ohm / ((ADC_MAX_COUNTS / raw) - 1.0f);
  if (!isfinite(res) || res <= 0.0f) {
    return NAN;
  }
  float nominalK = previewCfg.ntc_nominal_c + 273.15f;
  float invK = (1.0f / nominalK) + (log(res / previewCfg.ntc_nominal_ohm) / previewCfg.ntc_beta_k);
  if (!isfinite(invK) || invK <= 0.0f) {
    return NAN;
  }
  float tempC = (1.0f / invK) - 273.15f;
  return tempC * previewCfg.temp_gain + previewCfg.temp_offset_c;
}

float readTemp() {
  return adcToTemp(readAdcStable());
}

void updateThermalModel() {
  float dt = TICK_MS / 1000.0f;
  float meas = readTemp();
  state.sensorReadCount++;
  state.lastRawTemp = meas;
  bool validElectrical = isfinite(meas) && meas > NTC_MIN_VALID_C && meas < NTC_MAX_VALID_C;
  if (!validElectrical) {
    state.sensorInvalidCount++;
    if (state.sensorBadStreak < 255) {
      state.sensorBadStreak++;
    }
    state.sensorGoodStreak = 0;
    state.siConfidence = max(0.0f, state.siConfidence - 12.0f);
    if (!state.filterReady || state.sensorBadStreak >= SENSOR_BAD_LIMIT) {
      state.coolantValid = false;
      state.faults |= FAULT_COOLANT_SENSOR;
    }
    return;
  }

  if (!state.filterReady) {
    state.ema = meas;
    state.v = 0.0f;
    state.trend = 0.0f;
    state.flux = 0.0f;
    state.noiseC = 0.0f;
    state.predTemp60 = meas;
    state.filterReady = true;
  }

  float predicted = state.ema + state.v * dt;
  float residual = meas - predicted;
  float residualAbs = fabs(residual);
  float maxJump = 5.0f + fabs(state.trend) * 4.0f + state.noiseC * 3.0f;
  bool safetyHigh = meas >= (float)previewCfg.warning_temp_c - 2.0f;
  if (residualAbs > maxJump && !safetyHigh) {
    state.spikeRejects++;
    state.sensorSpikeCount++;
    state.noiseC = clampFloat(state.noiseC * 0.85f + residualAbs * 0.15f, 0.0f, 12.0f);
    state.siConfidence = clampFloat(state.siConfidence - 8.0f, 5.0f, 100.0f);
    if (residual < 0.0f) {
      state.sensorInvalidCount++;
      if (state.sensorBadStreak < 255) {
        state.sensorBadStreak++;
      }
      state.sensorGoodStreak = 0;
      if (state.sensorBadStreak >= SENSOR_BAD_LIMIT) {
        state.coolantValid = false;
        state.faults |= FAULT_COOLANT_SENSOR;
      }
      return;
    }
    if (state.spikeRejects < SENSOR_SPIKE_LIMIT) {
      return;
    }
  }

  state.spikeRejects = 0;
  if (state.sensorGoodStreak < 255) {
    state.sensorGoodStreak++;
  }
  state.sensorBadStreak = 0;
  if (state.sensorGoodStreak >= SENSOR_GOOD_LIMIT) {
    state.coolantValid = true;
    state.faults &= ~FAULT_COOLANT_SENSOR;
  }

  float spreadPenalty = clampFloat(state.adcSpread / 120.0f, 0.0f, 1.0f);
  float alpha = 0.07f + smoothStep01(residualAbs / 7.0f) * 0.16f;
  alpha *= 1.0f - spreadPenalty * 0.45f;
  alpha = clampFloat(alpha, 0.045f, 0.24f);
  float beta = clampFloat(alpha * 0.22f, 0.012f, 0.055f);

  float previousTrend = state.trend;
  state.ema = predicted + alpha * residual;
  state.v = clampFloat(state.v + beta * residual / dt, -1.8f, 2.8f);
  state.trend = state.trend * 0.82f + state.v * 0.18f;
  state.flux = (state.trend - previousTrend) / dt;
  state.noiseC = clampFloat(state.noiseC * 0.88f + residualAbs * 0.12f, 0.0f, 12.0f);
  state.predTemp60 = clampFloat(state.ema + state.trend * 60.0f, NTC_MIN_VALID_C, NTC_MAX_VALID_C);

  float riskTemp = max(state.ema, state.predTemp60);
  state.riskScore = clampFloat(smoothRange(previewCfg.warning_temp_c - 10.0f,
                                           previewCfg.critical_temp_c,
                                           riskTemp) * 100.0f,
                               0.0f, 100.0f);
  state.siConfidence = clampFloat(96.0f - spreadPenalty * 22.0f - state.noiseC * 4.5f,
                                  10.0f, 99.0f);
}

int interpolateManual(float temp, bool fan) {
  ManualStep *steps = previewCfg.manual_steps;
  if (temp <= steps[0].temperature_c) {
    return fan ? steps[0].fan_pct : steps[0].pump_pct;
  }
  for (int i = 1; i < 18; ++i) {
    if (temp <= steps[i].temperature_c) {
      float span = max(1.0f, steps[i].temperature_c - steps[i - 1].temperature_c);
      float k = (temp - steps[i - 1].temperature_c) / span;
      int a = fan ? steps[i - 1].fan_pct : steps[i - 1].pump_pct;
      int b = fan ? steps[i].fan_pct : steps[i].pump_pct;
      return constrain((int)(a + (b - a) * k), 0, 100);
    }
  }
  return fan ? steps[17].fan_pct : steps[17].pump_pct;
}

void applyOutputs(int targetPump, int targetFan) {
  bool safetyOverride = !state.coolantValid || state.ema >= previewCfg.critical_temp_c ||
                        (state.faults & FAULT_CONTROL_STALE);
  if (!safetyOverride) {
    state.outputsForced = false;
    if (previewCfg.force_pump == 1) {
      targetPump = 0;
    } else if (previewCfg.force_pump == 2) {
      targetPump = 100;
    }
    if (previewCfg.force_fan == 1) {
      targetFan = 0;
    } else if (previewCfg.force_fan == 2) {
      targetFan = 100;
    }
  } else {
    state.outputsForced = true;
    state.pump = 100;
    state.fan = 100;
    ledcWrite(PWM_CHAN, 255);
    return;
  }
  targetPump = constrain(targetPump, 0, 100);
  targetFan = constrain(targetFan, 0, 100);
  int upStep = max(1, (int)roundf(previewCfg.slew_pct_s * (TICK_MS / 1000.0f)));
  int downStep = max(1, (int)roundf(previewCfg.slew_pct_s * 0.55f * (TICK_MS / 1000.0f)));
  state.pump = approachInt(state.pump, targetPump, upStep, downStep);
  state.fan = approachInt(state.fan, targetFan, upStep, downStep);
  ledcWrite(PWM_CHAN, map(state.fan, 0, 100, 0, 255));
}

void engine() {
  updateThermalModel();
  bool curEcu = digitalRead(PIN_ECU);
  if (curEcu != state.ecuSignal) {
    if (curEcu) {
      state.ecuHighCount++;
    } else {
      state.ecuLowCount++;
    }
    state.ecuSignal = curEcu;
  }

  int targetP = 0;
  int targetF = 0;
  if (!state.coolantValid) {
    state.safetyState = SAFETY_SENSOR_FAILSAFE;
    targetP = 100;
    targetF = 100;
  } else if (state.ema >= previewCfg.critical_temp_c) {
    state.safetyState = SAFETY_CRITICAL_FAILSAFE;
    state.faults |= FAULT_CONTROL_STALE;
    targetP = 100;
    targetF = 100;
  } else if (state.ema < previewCfg.warning_temp_c - 4) {
    state.safetyState = SAFETY_NORMAL;
    state.faults &= ~FAULT_CONTROL_STALE;
  } else {
    state.safetyState = SAFETY_NORMAL;
  }

  if (targetP == 0 && targetF == 0 && state.coolantValid) {
    if (previewCfg.mode == 1) {
      targetP = interpolateManual(state.ema, false);
      targetF = interpolateManual(state.ema, true);
    } else if (previewCfg.mode == 2) {
      targetP = 100;
      targetF = 100;
    } else if (previewCfg.mode == 3 || previewCfg.mode == 4) {
      targetP = 100;
      targetF = previewCfg.mode == 4 ? 100 : 40;
    } else {
      float warmTrend = max(0.0f, state.trend);
      float coolTrend = max(0.0f, -state.trend);
      float pumpTemp = max(state.ema, state.ema + warmTrend * 24.0f);
      float fanTemp = max(state.ema, state.ema + warmTrend * 18.0f);
      float pumpBase = smoothRange(previewCfg.min_temp_c, previewCfg.auto_full_c, pumpTemp) * 100.0f;
      float fanBase = smoothRange(previewCfg.auto_target_c - 3.0f,
                                  previewCfg.auto_full_c,
                                  fanTemp) *
                      100.0f;
      float trendBoost = clampFloat(warmTrend * 18.0f, 0.0f, 24.0f);
      float ecuBoost = state.ecuSignal ? 18.0f : 0.0f;
      float coolingPull = clampFloat(coolTrend * 10.0f, 0.0f, 14.0f);
      targetP = (int)roundf(clampFloat(pumpBase + trendBoost + ecuBoost - coolingPull, 0.0f, 100.0f));
      targetF = (int)roundf(clampFloat(fanBase + trendBoost + (state.ecuSignal ? 8.0f : 0.0f) - coolingPull,
                                       0.0f, 100.0f));

      if (state.pump > 0 && state.ema > previewCfg.min_temp_c - 2.0f) {
        targetP = max(targetP, min(state.pump, 18));
      }
      if (state.fan > 0 && state.ema > previewCfg.auto_target_c - 4.0f) {
        targetF = max(targetF, min(state.fan, 15));
      }
    }
  }
  applyOutputs(targetP, targetF);
  state.sequence++;
}

void handleSSR() {
  if (state.pump <= 0) {
    state.ssrActive = false;
    digitalWrite(PIN_SSR, LOW);
    return;
  }
  if (state.pump >= 99) {
    state.ssrActive = true;
    digitalWrite(PIN_SSR, HIGH);
    return;
  }
  unsigned long now = millis();
  unsigned long window = (unsigned long)max(10, previewCfg.window_s) * 1000UL;
  unsigned long elapsed = now % window;
  unsigned long onTime = (unsigned long)((state.pump / 100.0f) * window);
  state.ssrActive = state.pump > 0 && elapsed < onTime;
  digitalWrite(PIN_SSR, state.ssrActive ? HIGH : LOW);
}

void pushStatus() {
  if (ws.count() == 0 || millis() - lastWsPushMs < STATUS_PUSH_MS) {
    return;
  }
  lastWsPushMs = millis();
  DynamicJsonDocument doc(4096);
  statusToJson(doc);
  String out;
  serializeJson(doc, out);
  ws.textAll(out);
}

String makeToken() {
  char buf[33];
  uint32_t a = esp_random();
  uint32_t b = esp_random();
  uint32_t c = esp_random();
  uint32_t d = esp_random();
  snprintf(buf, sizeof(buf), "%08lx%08lx%08lx%08lx",
           (unsigned long)a, (unsigned long)b, (unsigned long)c, (unsigned long)d);
  return String(buf);
}

String boardSerial() {
  uint64_t mac = ESP.getEfuseMac();
  char buf[24];
  snprintf(buf, sizeof(buf), "SC-%04X%08X", (uint16_t)(mac >> 32), (uint32_t)mac);
  return String(buf);
}

bool lockedOut(const Lockout &lockout) {
  return lockout.lockedUntilMs != 0 && millis() < lockout.lockedUntilMs;
}

void noteFailure(Lockout &lockout, uint8_t threshold, uint32_t lockMs, uint32_t maxLockMs) {
  uint32_t now = millis();
  if (lockout.firstFailMs == 0 || now - lockout.firstFailMs > 5UL * 60UL * 1000UL) {
    lockout.firstFailMs = now;
    lockout.fails = 0;
  }
  lockout.fails++;
  if (lockout.fails >= threshold) {
    uint32_t multiplier = min<uint8_t>(lockout.fails - threshold + 1, 15);
    lockout.lockedUntilMs = now + min(lockMs * multiplier, maxLockMs);
  }
}

void noteSuccess(Lockout &lockout) {
  lockout = Lockout();
}

void setup() {
  Serial.begin(115200);
  bootResetReason = esp_reset_reason();
  esp_task_wdt_init(CONTROL_WDT_TIMEOUT_S, true);
  esp_task_wdt_add(NULL);
  analogReadResolution(12);
  analogSetPinAttenuation(PIN_NTC, ADC_11db);
  pinMode(PIN_SSR, OUTPUT);
  digitalWrite(PIN_SSR, LOW);
  pinMode(PIN_ECU, INPUT_PULLDOWN);
  ledcSetup(PWM_CHAN, PWM_FREQ, PWM_RES);
  ledcAttachPin(PIN_PWM, PWM_CHAN);
  loadConfig();
  setupNetwork();
  setupWebSocket();
  setupRoutes();
  server.begin();
  char bootMsg[72];
  snprintf(bootMsg, sizeof(bootMsg), "Boot reset=%s count=%lu",
           resetReasonText(bootResetReason), (unsigned long)bootCount);
  addEvent(0, bootMsg);
  if (configRecovered) {
    addEvent(1, "Config recovered from protected storage");
  }
}

void loop() {
  esp_task_wdt_reset();
  uint32_t now = millis();
  if (now - lastTickMs >= TICK_MS) {
    lastTickMs = now;
    engine();
  }
  handleSSR();
  maintainAp();
  ws.cleanupClients();
  pushStatus();
}

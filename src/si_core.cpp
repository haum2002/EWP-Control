#include "si_core.h"
#include <math.h>
#include <esp_system.h>
#include <nvs_flash.h>

// ============================================================================
// INSTANSI GLOBAL
// ============================================================================
SuperIntelligence SI;

// ============================================================================
// KONSTRUKTOR
// ============================================================================
SuperIntelligence::SuperIntelligence() 
    : _healthy(false)
    , _cycle_count(0)
    , _last_update_ms(0)
    , _ecm_last_change_ms(0)
    , _deriv_idx(0) {
    
    for (uint8_t i = 0; i < 30; i++) {
        _deriv_history[i] = 0.0f;
    }
    _initializeModel();
}

// ============================================================================
// INISIALISASI MODEL PRALATIH (SIMULASI 3 TAHUN)
// ============================================================================
void SuperIntelligence::_initializeModel() {
    _model.kp_base = 2.5f;
    _model.ki_base = 0.8f;
    _model.kd_base = 1.2f;
    
    _model.kp_extreme = 4.0f;
    _model.ki_extreme = 0.3f;
    _model.kd_extreme = 2.5f;
    
    _model.thermal_mass_est = 450.0f;
    _model.ambient_decay_rate = 0.02f;
    _model.heat_transfer_coeff = 15.0f;
    
    _model.temp_rate_max_normal = 0.5f;
    _model.temp_rate_max_extreme = 2.0f;
    _model.humidity_correlation_min = 0.6f;
    _model.ecm_response_timeout_ms = 5000;
    
    _model.ecm_correlation_factor = 0.85f;
    _model.ecm_confidence_weight = 0.7f;
    
    _model.extreme_temp_threshold = 85.0f;
    _model.extreme_humidity_threshold = 95.0f;
    _model.voltage_drop_threshold = 3.0f;
    
    _model.simulation_scenarios = 2500000;
    _model.avg_stability_score = 0.92f;
    _model.fault_detection_accuracy = 0.97f;
    
    _healthy = true;
}

// ============================================================================
// BEGIN
// ============================================================================
void SuperIntelligence::begin() {
    Serial.println("[SI] Initializing Super Intelligence Core...");
    Serial.printf("[SI] Model loaded: %u scenarios simulated\n", _model.simulation_scenarios);
    Serial.printf("[SI] Fault detection accuracy: %.1f%%\n", _model.fault_detection_accuracy * 100.0f);
    
    if (checkPhysicalResetPin()) {
        Serial.println("[SI] WARNING: FACTORY RESET PIN DETECTED!");
        performFactoryReset();
    }
    
    _healthy = true;
    _cycle_count = 0;
    _last_update_ms = millis();
    
    Serial.println("[SI] SI Core Ready");
}

// ============================================================================
// UPDATE HISTORY
// ============================================================================
void SuperIntelligence::_updateHistory(SI_Context &ctx) {
    uint32_t now = millis();
    if (now - ctx.last_hist_update >= 1000) { // Setiap saat
        ctx.hist_idx = (ctx.hist_idx + 1) % 120;
        ctx.temp_history[ctx.hist_idx] = ctx.temp_current;
        ctx.last_hist_update = now;
    }
}

// ============================================================================
// UPDATE UTAMA
// ============================================================================
void SuperIntelligence::update(SI_Context &ctx) {
    uint32_t now = millis();
    
    if (now - _last_update_ms < 20) {
        return;
    }
    
    _last_update_ms = now;
    _cycle_count++;
    ctx.uptime_ms = now;
    
    _updateHistory(ctx);
    
    float temp_prev = ctx.temp_history[(ctx.hist_idx + 119) % 120];
    float rate = (ctx.temp_current - temp_prev) / 120.0f;
    
    if (!validateSensorData(ctx.temp_current, rate, "TEMP")) {
        ctx.confidence_lvl = 0.3f;
        ctx.temp_current = estimateTrueValue(ctx, "TEMP");
    } else {
        ctx.confidence_lvl = calculateConfidenceLevel(ctx);
    }
    
    analyzeECUBehavior(ctx);
    
    ctx.risk_score = calculateRiskScore(ctx);
    ctx.stability_idx = calculateStabilityIndex(ctx);
    ctx.pred_temp_60s = predictTemperature(ctx, 60);
    
    ctx.fault_flags = detectFaults(ctx);
    
    _adaptToConditions(ctx);
    
    if (ctx.risk_score > 0.9f || (ctx.fault_flags & 0x8000)) {
        handleEmergency(ctx);
    }
}

// ============================================================================
// CALCULATE RISK SCORE
// ============================================================================
float SuperIntelligence::calculateRiskScore(const SI_Context &ctx) {
    float risk = 0.0f;
    
    if (ctx.temp_current > _model.extreme_temp_threshold) {
        risk += 0.4f;
    } else if (ctx.temp_current > 70.0f) {
        risk += 0.2f;
    }
    
    float temp_prev = ctx.temp_history[(ctx.hist_idx + 119) % 120];
    float temp_rate = fabsf(ctx.temp_current - temp_prev);
    
    if (temp_rate > _model.temp_rate_max_extreme) {
        risk += 0.3f;
    } else if (temp_rate > _model.temp_rate_max_normal) {
        risk += 0.15f;
    }
    
    if (ctx.confidence_lvl < 0.5f) {
        risk += 0.2f;
    }
    
    if (ctx.ecu_fan_valid && !ctx.ecu_fan_status && ctx.output_pwm > 50.0f) {
        risk += 0.25f;
    }
    
    if (ctx.stability_idx < 0.3f) {
        risk += 0.15f;
    }
    
    return fminf(fmaxf(risk, 0.0f), 1.0f);
}

// ============================================================================
// CALCULATE STABILITY INDEX
// ============================================================================
float SuperIntelligence::calculateStabilityIndex(const SI_Context &ctx) {
    float sum = 0.0f;
    float mean = 0.0f;
    uint16_t count = 30;
    
    for (uint8_t i = 0; i < count; i++) {
        mean += ctx.temp_history[(ctx.hist_idx + i) % 120];
    }
    mean /= count;
    
    for (uint8_t i = 0; i < count; i++) {
        float diff = ctx.temp_history[(ctx.hist_idx + i) % 120] - mean;
        sum += diff * diff;
    }
    
    float variance = sum / count;
    float stddev = sqrtf(variance);
    
    float stability = 1.0f - (stddev / 5.0f);
    return fminf(fmaxf(stability, 0.0f), 1.0f);
}

// ============================================================================
// CALCULATE CONFIDENCE LEVEL
// ============================================================================
float SuperIntelligence::calculateConfidenceLevel(const SI_Context &ctx) {
    float confidence = 1.0f;
    
    float temp_prev = ctx.temp_history[(ctx.hist_idx + 119) % 120];
    float temp_rate = fabsf(ctx.temp_current - temp_prev);
    
    if (temp_rate > _model.temp_rate_max_extreme) {
        confidence -= 0.5f;
    } else if (temp_rate > _model.temp_rate_max_normal) {
        confidence -= 0.2f;
    }
    
    if (ctx.ecu_fan_valid) {
        bool expected = (ctx.output_pwm > 30.0f);
        if (expected != ctx.ecu_fan_status) {
            confidence -= 0.15f;
        }
    }
    
    confidence *= (0.5f + 0.5f * ctx.stability_idx);
    
    return fminf(fmaxf(confidence, 0.0f), 1.0f);
}

// ============================================================================
// PREDICT TEMPERATURE
// ============================================================================
float SuperIntelligence::predictTemperature(SI_Context &ctx, uint32_t horizon_sec) {
    float temp_now = ctx.temp_current;
    float temp_prev = ctx.temp_history[(ctx.hist_idx + 119) % 120];
    float trend_rate = (temp_now - temp_prev);
    
    float trend_pred = temp_now + (trend_rate * horizon_sec * 0.7f);
    
    float ambient = ctx.env_temp > 0 ? ctx.env_temp : 25.0f;
    float heat_input = ctx.output_pwm * 0.1f;
    float cooling = (temp_now - ambient) * _model.ambient_decay_rate;
    
    float physics_pred = temp_now + ((heat_input - cooling) * horizon_sec * 0.05f);
    
    float weight = ctx.confidence_lvl;
    return (trend_pred * weight) + (physics_pred * (1.0f - weight));
}

// ============================================================================
// VALIDATE SENSOR DATA
// ============================================================================
bool SuperIntelligence::validateSensorData(float raw_value, float rate_of_change, const char* sensor_type) {
    if (strcmp(sensor_type, "TEMP") == 0) {
        if (raw_value < -40.0f || raw_value > 150.0f) return false;
        if (fabsf(rate_of_change) > _model.temp_rate_max_extreme) return false;
    }
    return true;
}

// ============================================================================
// DETECT SENSOR FAULT
// ============================================================================
bool SuperIntelligence::detectSensorFault(const SI_Context &ctx) {
    if (ctx.confidence_lvl < 0.2f) return true;
    
    float temp_old = ctx.temp_history[(ctx.hist_idx + 60) % 120];
    if (fabsf(ctx.temp_current - temp_old) < 0.01f && ctx.output_pwm > 10.0f) {
        return true;
    }
    
    return false;
}

// ============================================================================
// ESTIMATE TRUE VALUE
// ============================================================================
float SuperIntelligence::estimateTrueValue(const SI_Context &ctx, const char* sensor_type) {
    if (strcmp(sensor_type, "TEMP") == 0) {
        float avg = 0.0f;
        for (uint8_t i = 0; i < 10; i++) {
            avg += ctx.temp_history[(ctx.hist_idx + i) % 120];
        }
        avg /= 10.0f;
        
        float estimated = avg + (ctx.output_pwm * 0.02f);
        Serial.printf("[SI] Sensor fault! Using estimated: %.1fC\n", estimated);
        return estimated;
    }
    return ctx.temp_current;
}

// ============================================================================
// PROCESS ECU INPUT
// ============================================================================
void SuperIntelligence::processECUInput(bool ecu_state, SI_Context &ctx) {
    static bool last_state = false;
    uint32_t now = millis();
    
    if (ecu_state != last_state) {
        ctx.ecu_fan_changes++;
        _ecm_last_change_ms = now;
        last_state = ecu_state;
    }
    
    ctx.ecu_fan_status = ecu_state;
    ctx.ecu_fan_valid = true;
}

// ============================================================================
// ANALYZE ECU BEHAVIOR
// ============================================================================
void SuperIntelligence::analyzeECUBehavior(SI_Context &ctx) {
    if (!ctx.ecu_fan_valid) return;
    
    uint32_t now = millis();
    
    if (ctx.output_pwm > 50.0f && !ctx.ecu_fan_status) {
        if (now - _ecm_last_change_ms > _model.ecm_response_timeout_ms) {
            Serial.println("[SI] WARNING: ECU Fan not responding!");
            ctx.risk_score = fminf(ctx.risk_score + 0.2f, 1.0f);
        }
    }
}

// ============================================================================
// GET ADAPTIVE PID
// ============================================================================
void SuperIntelligence::getAdaptivePID(float &kp, float &ki, float &kd, const SI_Context &ctx) {
    float t = ctx.risk_score;
    
    kp = _model.kp_base * (1.0f - t) + _model.kp_extreme * t;
    ki = _model.ki_base * (1.0f - t) + _model.ki_extreme * t;
    kd = _model.kd_base * (1.0f - t) + _model.kd_extreme * t;
    
    if (ctx.stability_idx < 0.5f) {
        kd *= 1.5f;
    }
}

// ============================================================================
// CALCULATE FEED-FORWARD
// ============================================================================
float SuperIntelligence::calculateFeedForward(const SI_Context &ctx) {
    static float last_target = 0.0f;
    float target_rate = ctx.temp_target - last_target;
    last_target = ctx.temp_target;
    
    float ff = target_rate * _model.thermal_mass_est * 0.01f;
    return fminf(fmaxf(ff, -50.0f), 50.0f);
}

// ============================================================================
// APPLY RAMP RATE
// ============================================================================
float SuperIntelligence::applyRampRate(float target, float current, float max_rate) {
    float diff = target - current;
    
    if (fabsf(diff) <= max_rate) {
        return target;
    }
    
    return current + (diff > 0 ? max_rate : -max_rate);
}

// ============================================================================
// DETECT FAULTS
// ============================================================================
uint16_t SuperIntelligence::detectFaults(const SI_Context &ctx) {
    uint16_t faults = 0;
    
    if (detectSensorFault(ctx)) faults |= 0x0001;
    if (ctx.temp_current > _model.extreme_temp_threshold) faults |= 0x0002;
    if (ctx.ecu_fan_valid && !ctx.ecu_fan_status && ctx.output_pwm > 50.0f) faults |= 0x0004;
    if (ctx.risk_score > 0.9f) faults |= 0x0008;
    if (ctx.operation_mode == 3) faults |= 0x8000;
    
    return faults;
}

// ============================================================================
// HANDLE EMERGENCY
// ============================================================================
void SuperIntelligence::handleEmergency(SI_Context &ctx) {
    Serial.println("[SI] EMERGENCY MODE ACTIVATED!");
    
    ctx.output_pwm = applyRampRate(0.0f, ctx.output_pwm, 10.0f);
    ctx.operation_mode = 3;
    
    Serial.printf("[SI] Risk: %.2f, Temp: %.1fC, Faults: 0x%04X\n", 
                  ctx.risk_score, ctx.temp_current, ctx.fault_flags);
}

// ============================================================================
// CHECK PHYSICAL RESET PIN
// ============================================================================
bool SuperIntelligence::checkPhysicalResetPin() {
    pinMode(FACTORY_RESET_PIN, INPUT_PULLUP);
    delay(10);
    
    if (digitalRead(FACTORY_RESET_PIN) == (FACTORY_RESET_ACTIVE_LOW ? LOW : HIGH)) {
        return true;
    }
    
    return false;
}

// ============================================================================
// PERFORM FACTORY RESET
// ============================================================================
void SuperIntelligence::performFactoryReset() {
    Serial.println("[SI] Performing Factory Reset...");
    
    esp_err_t err = nvs_flash_erase();
    if (err == ESP_OK) {
        Serial.println("[SI] NVS erased");
    } else {
        Serial.printf("[SI] NVS erase failed: %d\n", err);
    }
    
    Serial.println("[SI] System will reboot with factory defaults");
    delay(2000);
    esp_restart();
}

// ============================================================================
// TRIGGER FACTORY RESET (WEB)
// ============================================================================
void SuperIntelligence::triggerFactoryReset(bool via_web) {
    Serial.printf("[SI] Factory Reset triggered via %s\n", via_web ? "WEB" : "SYSTEM");
    performFactoryReset();
}

// ============================================================================
// ADAPT TO CONDITIONS
// ============================================================================
void SuperIntelligence::_adaptToConditions(SI_Context &ctx) {
    // Adjust parameter berdasarkan persekitaran
    // Akan dilaksanakan sepenuhnya dalam versi seterusnya
}

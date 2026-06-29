#include "si_sentinel.h"
#include <esp_system.h>

SISentinel Sentinel;

SISentinel::SISentinel() : _canary(SENTINEL_CANARY_VALUE), _prev_output(0.0f), _hist_idx(0) {
    _status.is_safe = true;
    _status.active_threat = THREAT_NONE;
    _status.threat_count = 0;
    _status.sanity_score = 1.0f;
    _status.last_request_ms = 0;
    _status.request_count = 0;
    
    for(int i=0; i<SENTINEL_ANOMALY_WINDOW; i++) _history_vals[i] = 0.0f;
}

void SISentinel::begin() {
    Serial.println("[SENTINEL] System Shield Active. Monitoring logic & memory integrity.");
    verifyMemoryIntegrity(); // Check awal
}

bool SISentinel::validateAction(float proposed_output, const SI_Context &ctx) {
    // 1. Semak cadangan output yang tidak selamat
    if (_checkUnsafeAction(proposed_output, ctx)) {
        _raiseThreat(THREAT_UNSAFE_ACTION);
        return false;
    }

    // 2. Cek Kadar Perubahan Output (Cegah lonjakan mendadak)
    float delta = abs(proposed_output - _prev_output);
    if (delta > SENTINEL_MAX_OUTPUT_DELTA) {
        // Ramp rate limiter: Jangan benarkan perubahan terlalu pantas
        // Ini adalah perlindungan fizikal & logik
        return false; 
    }

    // 3. Cek Risk Score Konteks
    if (ctx.risk_score > SENTINEL_RISK_THRESHOLD) {
        // Jika risiko sudah tinggi, hanya benarkan tindakan yang mengurangkan risiko
        if (proposed_output < _prev_output && ctx.temp_current > ctx.temp_target) {
             // Mengurangkan output ketika suhu tinggi dan risiko tinggi adalah berbahaya.
             _raiseThreat(THREAT_UNSAFE_ACTION);
             return false;
        }
    }

    _prev_output = proposed_output;
    
    // Update history untuk deteksi anomali
    _history_vals[_hist_idx] = proposed_output;
    _hist_idx = (_hist_idx + 1) % SENTINEL_ANOMALY_WINDOW;

    return true;
}

bool SISentinel::sanitizeSensorInput(float &value, float min_limit, float max_limit, float max_rate_change) {
    // 1. Cek Had Fizikal
    if (value < min_limit || value > max_limit) {
        _raiseThreat(THREAT_SENSOR_SPOOFING);
        return false;
    }

    // 2. Cek Kadar Perubahan (Rate of Change)
    float last_val = _history_vals[(_hist_idx + SENTINEL_ANOMALY_WINDOW - 1) % SENTINEL_ANOMALY_WINDOW];
    if (last_val != 0.0f) {
        float rate = abs(value - last_val);
        if (rate > max_rate_change) {
            // Perubahan terlalu pantas secara fizikal mustahil = Spoofing/Fault
            _raiseThreat(THREAT_SENSOR_SPOOFING);
            return false;
        }
    }

    return true;
}

bool SISentinel::detectLogicInjection(const char* input, size_t len) {
    // Sanitasi Input String (Cegah buffer overflow & injection)
    if (len == 0 || len > 256) return false; // Had panjang
    
    // Cek karakter berbahaya (null terminator di tengah, kontrol karakter, dll)
    for (size_t i = 0; i < len; i++) {
        char c = input[i];
        if (c == '\0' && i != len-1) return false; // Null tengah string
        if (c < 32 && c != '\n' && c != '\r') return false; // Kontrol karakter aneh
        if (c == '<' || c == '>' || c == ';' || c == '|') {
            // Potensi script injection atau command injection
            _raiseThreat(THREAT_LOGIC_INJECTION);
            return false;
        }
    }
    return true;
}

bool SISentinel::checkRateLimit() {
    uint32_t now = millis();
    if (now - _status.last_request_ms < SENTINEL_RATE_LIMIT_MS) {
        _status.request_count++;
        if (_status.request_count > 10) { // Lebih dari 10 request dalam jendela singkat
            _raiseThreat(THREAT_RATE_OVERFLOW);
            return false;
        }
    } else {
        _status.request_count = 1;
        _status.last_request_ms = now;
    }
    return true;
}

bool SISentinel::verifyMemoryIntegrity() {
    // 1. Cek Canary Value (Deteksi korupsi memori statis)
    if (_canary != SENTINEL_CANARY_VALUE) {
        _raiseThreat(THREAT_MEMORY_CORRUPTION);
        _canary = SENTINEL_CANARY_VALUE; // Reset tapi catat ancaman
        return false;
    }

    // 2. Cek Heap & Stack Free (ESP32 Specific)
    uint32_t free_heap = esp_get_free_heap_size();
    if (free_heap < 10000) { // Kurang dari 10KB bebas = Bahaya
        _status.sanity_score = 0.5f;
        // Tidak raise threat langsung, tapi turunkan skor kesihatan
        // Sistem akan kurangkan beban secara automatik di main loop
        return false;
    }
    
    // 3. Cek Minimum Stack Size (Task Watchdog implicit)
    // Nota: Ini perlu dijalankan dalam task jika menggunakan FreeRTOS tasks
    
    _status.sanity_score = 1.0f;
    return true;
}

void SISentinel::_raiseThreat(ThreatType type) {
    _status.active_threat = type;
    _status.threat_count++;
    _status.is_safe = false;
    _status.sanity_score -= 0.2f;
    if (_status.sanity_score < 0.0f) _status.sanity_score = 0.0f;

    Serial.printf("[SENTINEL ALERT] Threat Detected: %d | Count: %lu | Sanity: %.2f\n",
                  type, (unsigned long)_status.threat_count, _status.sanity_score);
    
    // Log ancaman ke Data Logger jika tersedia
    // DataLogger.logThreat(type); 
}

void SISentinel::clearThreat() {
    if (_status.threat_count > 0) {
        Serial.println("[SENTINEL] Threat cleared manually. Resuming normal operation.");
        _status.active_threat = THREAT_NONE;
        _status.is_safe = true;
        _status.sanity_score = 1.0f;
        _status.request_count = 0;
    }
}

bool SISentinel::_checkUnsafeAction(float proposed, const SI_Context &ctx) {
    // Logik heuristik untuk tindakan output yang tidak selamat.

    if (ctx.temp_current > 150.0f && proposed < 80.0f) {
        // Suhu sangat tinggi mesti memihak kepada penyejukan maksimum.
        return true;
    }
    
    if (ctx.confidence_lvl < 0.3f && ctx.temp_current > ctx.temp_target && proposed < 80.0f) {
        // Bila data tidak meyakinkan dan suhu tinggi, output rendah tidak selamat.
        return true;
    }

    return false;
}

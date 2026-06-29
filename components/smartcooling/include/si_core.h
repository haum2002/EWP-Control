#ifndef SI_CORE_H
#define SI_CORE_H

#include <Arduino.h>
#include "factory_config.h"

// ============================================================================
// STRUKTUR DATA SUPER INTELLIGENCE (SI)
// ============================================================================

struct SI_Context {
    // Input Sensor
    float temp_current;
    float temp_target;
    float env_temp;
    float env_humidity;
    float env_pressure;
    
    // Input ECU/ECM
    bool ecu_fan_status;
    bool ecu_fan_valid;
    uint32_t ecu_fan_changes;
    
    // Output Kawalan
    float output_pwm;
    float output_prev;
    
    // Metrik SI
    float risk_score;        // 0.0 (Selamat) - 1.0 (Bahaya Kritikal)
    float stability_idx;     // 0.0 (Tidak Stabil) - 1.0 (Sangat Stabil)
    float confidence_lvl;    // Keyakinan SI terhadap data (0.0-1.0)
    float pred_temp_60s;     // Prediksi suhu 60 saat hadapan
    
    // Status Sistem
    uint32_t uptime_ms;
    uint16_t fault_flags;
    uint8_t operation_mode;  // 0=Manual, 1=Auto, 2=Safe, 3=Emergency
    
    // Sejarah untuk analisis trend
    float temp_history[120]; // 120 saat sejarah
    uint8_t hist_idx;
    uint32_t last_hist_update;
};

// ============================================================================
// MODEL KAWALAN SI
// ============================================================================

struct SI_Control_Model {
    // Parameter PID Adaptif
    float kp_base, ki_base, kd_base;
    float kp_extreme, ki_extreme, kd_extreme;
    
    // Model Termodinamik
    float thermal_mass_est;
    float ambient_decay_rate;
    float heat_transfer_coeff;
    
    // Threshold Fault (dari simulasi)
    float temp_rate_max_normal;      // °C/s
    float temp_rate_max_extreme;     // °C/s
    float humidity_correlation_min;
    float ecm_response_timeout_ms;
    
    // Faktor Korelasi ECU
    float ecm_correlation_factor;
    float ecm_confidence_weight;
    
    // Parameter Stres Ekstrem
    float extreme_temp_threshold;
    float extreme_humidity_threshold;
    float voltage_drop_threshold;
    
    // Metadata kalibrasi dalaman
    uint32_t calibration_revision;
    float stability_weight;
    float fault_weight;
};

// ============================================================================
// KELAS SUPER INTELLIGENCE
// ============================================================================

class SuperIntelligence {
public:
    SuperIntelligence();
    
    // Inisialisasi
    void begin();
    
    // Kitaran Utama SI (dipanggil setiap 10-50ms)
    void update(SI_Context &ctx);
    
    // Pengiraan Metrik SI
    float calculateRiskScore(const SI_Context &ctx);
    float calculateStabilityIndex(const SI_Context &ctx);
    float calculateConfidenceLevel(const SI_Context &ctx);
    float predictTemperature(SI_Context &ctx, uint32_t horizon_sec);
    
    // Validasi Sensor & Data
    bool validateSensorData(float raw_value, float rate_of_change, const char* sensor_type);
    bool detectSensorFault(const SI_Context &ctx);
    float estimateTrueValue(const SI_Context &ctx, const char* sensor_type);
    
    // Integrasi ECU/ECM
    void processECUInput(bool ecu_state, SI_Context &ctx);
    void analyzeECUBehavior(SI_Context &ctx);
    
    // Kawalan Adaptif
    void getAdaptivePID(float &kp, float &ki, float &kd, const SI_Context &ctx);
    float calculateFeedForward(const SI_Context &ctx);
    float applyRampRate(float target, float current, float max_rate);
    
    // Pengurusan Fault
    uint16_t detectFaults(const SI_Context &ctx);
    void handleEmergency(SI_Context &ctx);
    
    // Factory Reset
    void triggerFactoryReset(bool via_web);
    bool checkPhysicalResetPin();
    void performFactoryReset();
    
    // Akses Model
    const SI_Control_Model& getModel() const { return _model; }
    
    // Status
    bool isHealthy() const { return _healthy; }
    uint32_t getCycleCount() const { return _cycle_count; }

private:
    SI_Control_Model _model;
    bool _healthy;
    uint32_t _cycle_count;
    uint32_t _last_update_ms;
    uint32_t _ecm_last_change_ms;
    
    // Ring Buffer untuk Analisis
    float _deriv_history[30];
    uint8_t _deriv_idx;
    
    // Kaedah Dalaman
    void _initializeModel();
    void _updateHistory(SI_Context &ctx);
    void _simulateEnvironment(SI_Context &ctx);
    void _adaptToConditions(SI_Context &ctx);
    float _fixedPointMultiply(int32_t a, int32_t b, uint8_t shift);
    int32_t _fixedPointDivide(int32_t a, int32_t b, uint8_t shift);
};

// Instance Global
extern SuperIntelligence SI;

#endif // SI_CORE_H

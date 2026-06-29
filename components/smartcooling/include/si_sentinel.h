#ifndef SI_SENTINEL_H
#define SI_SENTINEL_H

#include <Arduino.h>
#include "si_core.h"

// --- KONFIGURASI PERTAHANAN ---
#define SENTINEL_MAX_OUTPUT_DELTA 15.0f      // % maks perubahan output per saat (cegah lonjakan)
#define SENTINEL_RISK_THRESHOLD 0.85f        // Ambang risiko untuk campur tangan
#define SENTINEL_ANOMALY_WINDOW 5            // Bilangan bacaan untuk deteksi anomali
#define SENTINEL_RATE_LIMIT_MS 100           // Had laju permintaan web/API
#define SENTINEL_CANARY_VALUE 0xDEADBEEF     // Nilai canary untuk deteksi korupsi memori

// --- JENIS ANCAMAN NON-FIZIK ---
enum ThreatType {
    THREAT_NONE = 0,
    THREAT_SENSOR_SPOOFING,      // Data sensor tidak masuk akal
    THREAT_LOGIC_INJECTION,      // Cubaan manipulasi logik melalui input
    THREAT_RATE_OVERFLOW,        // Permintaan terlalu laju (DoS)
    THREAT_MEMORY_CORRUPTION,    // Deteksi korupsi RAM/Stack
    THREAT_UNSAFE_ACTION         // Cadangan output tidak selamat
};

// --- STRUKTUR STATUS SENTINEL ---
struct SentinelStatus {
    bool is_safe;
    ThreatType active_threat;
    uint32_t threat_count;
    float sanity_score;          // 1.0 (Sihat) - 0.0 (Terancam)
    uint32_t last_request_ms;
    uint16_t request_count;
};

class SISentinel {
public:
    SISentinel();
    
    void begin();
    
    // Fungsi Utama: Validasi sebelum SI bertindak
    bool validateAction(float proposed_output, const SI_Context &ctx);
    
    // Fungsi Pertahanan: Sanitasi Input & Deteksi Anomali
    bool sanitizeSensorInput(float &value, float min_limit, float max_limit, float max_rate_change);
    bool detectLogicInjection(const char* input, size_t len);
    
    // Fungsi Pemantauan: Rate Limiting & Integriti Memori
    bool checkRateLimit();
    bool verifyMemoryIntegrity();
    
    // Dapatkan Status Terkini
    SentinelStatus getStatus() { return _status; }
    
    // Reset Ancaman (Hanya jika sah)
    void clearThreat();

private:
    SentinelStatus _status;
    uint32_t _canary;
    float _prev_output;
    float _history_vals[SENTINEL_ANOMALY_WINDOW];
    uint8_t _hist_idx;
    
    void _raiseThreat(ThreatType type);
    bool _checkUnsafeAction(float proposed, const SI_Context &ctx);
};

extern SISentinel Sentinel;

#endif

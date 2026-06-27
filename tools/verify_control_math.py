import json
import math
import random


ADC_MAX = 4095.0
TICK_S = 0.5
SENSOR_BAD_LIMIT = 3
SENSOR_SPIKE_LIMIT = 3


def clamp(value, low, high):
    return max(low, min(high, value))


def smooth_step01(value):
    x = clamp(value, 0.0, 1.0)
    return x * x * (3.0 - 2.0 * x)


def smooth_range(edge0, edge1, value):
    return smooth_step01((value - edge0) / max(0.001, edge1 - edge0))


def temp_to_adc(temp_c, cfg):
    nominal_k = cfg["ntc_nominal_c"] + 273.15
    temp_k = (temp_c - cfg["temp_offset_c"]) / cfg["temp_gain"] + 273.15
    resistance = cfg["ntc_nominal_ohm"] * math.exp(cfg["ntc_beta_k"] * (1.0 / temp_k - 1.0 / nominal_k))
    return ADC_MAX / (1.0 + cfg["ntc_series_ohm"] / resistance)


def adc_to_temp(raw, cfg):
    if raw <= 2.0 or raw >= ADC_MAX - 2.0:
        return math.nan
    resistance = cfg["ntc_series_ohm"] / ((ADC_MAX / raw) - 1.0)
    inv_k = (1.0 / (cfg["ntc_nominal_c"] + 273.15)) + math.log(resistance / cfg["ntc_nominal_ohm"]) / cfg["ntc_beta_k"]
    temp_c = (1.0 / inv_k) - 273.15
    return temp_c * cfg["temp_gain"] + cfg["temp_offset_c"]


class Model:
    def __init__(self, cfg):
        self.cfg = cfg
        self.ema = 74.0
        self.v = 0.0
        self.trend = 0.0
        self.noise_c = 0.0
        self.pred60 = 74.0
        self.conf = 0.0
        self.forced = False
        self.filter_ready = False
        self.good = 0
        self.bad = 0
        self.spikes = 0
        self.valid = False
        self.pump = 0
        self.fan = 0
        self.invalid_count = 0
        self.spike_count = 0

    def update_temp(self, meas, adc_spread=3.0):
        if not math.isfinite(meas) or meas <= -30.0 or meas >= 170.0:
            self.invalid_count += 1
            self.bad = min(255, self.bad + 1)
            self.good = 0
            if self.bad >= SENSOR_BAD_LIMIT:
                self.valid = False
            return

        if not self.filter_ready:
            self.ema = meas
            self.v = 0.0
            self.trend = 0.0
            self.pred60 = meas
            self.filter_ready = True

        predicted = self.ema + self.v * TICK_S
        residual = meas - predicted
        residual_abs = abs(residual)
        max_jump = 5.0 + abs(self.trend) * 4.0 + self.noise_c * 3.0
        safety_high = meas >= self.cfg["warning_temp_c"] - 2.0
        if residual_abs > max_jump and not safety_high:
            self.spikes += 1
            self.spike_count += 1
            self.noise_c = clamp(self.noise_c * 0.85 + residual_abs * 0.15, 0.0, 12.0)
            if residual < 0.0:
                self.invalid_count += 1
                self.bad = min(255, self.bad + 1)
                self.good = 0
                if self.bad >= SENSOR_BAD_LIMIT:
                    self.valid = False
                return
            if self.spikes < SENSOR_SPIKE_LIMIT:
                return

        self.spikes = 0
        self.good = min(255, self.good + 1)
        self.bad = 0
        self.valid = self.good >= 2

        spread_penalty = clamp(adc_spread / 120.0, 0.0, 1.0)
        alpha = (0.07 + smooth_step01(residual_abs / 7.0) * 0.16) * (1.0 - spread_penalty * 0.45)
        alpha = clamp(alpha, 0.045, 0.24)
        beta = clamp(alpha * 0.22, 0.012, 0.055)
        self.ema = predicted + alpha * residual
        self.v = clamp(self.v + beta * residual / TICK_S, -1.8, 2.8)
        self.trend = self.trend * 0.82 + self.v * 0.18
        self.noise_c = clamp(self.noise_c * 0.88 + residual_abs * 0.12, 0.0, 12.0)
        self.pred60 = clamp(self.ema + self.trend * 60.0, -30.0, 170.0)
        self.conf = clamp(96.0 - spread_penalty * 22.0 - self.noise_c * 4.5, 10.0, 99.0)

    def control(self, ecu=False):
        if not self.valid or self.ema >= self.cfg["critical_temp_c"]:
            self.pump = 100
            self.fan = 100
            self.forced = True
            return
        else:
            self.forced = False
            warm = max(0.0, self.trend)
            cool = max(0.0, -self.trend)
            pump_temp = max(self.ema, self.ema + warm * 24.0)
            fan_temp = max(self.ema, self.ema + warm * 18.0)
            pump_base = smooth_range(self.cfg["min_temp_c"], self.cfg["auto_full_c"], pump_temp) * 100.0
            fan_base = smooth_range(self.cfg["auto_target_c"] - 3.0, self.cfg["auto_full_c"], fan_temp) * 100.0
            trend_boost = clamp(warm * 18.0, 0.0, 24.0)
            cooling_pull = clamp(cool * 10.0, 0.0, 14.0)
            target_p = round(clamp(pump_base + trend_boost + (18.0 if ecu else 0.0) - cooling_pull, 0.0, 100.0))
            target_f = round(clamp(fan_base + trend_boost + (8.0 if ecu else 0.0) - cooling_pull, 0.0, 100.0))

        step_up = max(1, round(self.cfg["slew_pct_s"] * TICK_S))
        step_down = max(1, round(self.cfg["slew_pct_s"] * 0.55 * TICK_S))
        self.pump += clamp(target_p - self.pump, -step_down, step_up)
        self.fan += clamp(target_f - self.fan, -step_down, step_up)


def run():
    cfg = {
        "ntc_series_ohm": 10000.0,
        "ntc_nominal_ohm": 10000.0,
        "ntc_beta_k": 3950.0,
        "ntc_nominal_c": 25.0,
        "temp_offset_c": 0.0,
        "temp_gain": 1.0,
        "min_temp_c": 70.0,
        "auto_target_c": 80.0,
        "auto_full_c": 110.0,
        "warning_temp_c": 108.0,
        "critical_temp_c": 120.0,
        "slew_pct_s": 5.0,
    }

    for temp in (20.0, 40.0, 80.0, 110.0):
        raw = temp_to_adc(temp, cfg)
        back = adc_to_temp(raw, cfg)
        assert abs(back - temp) < 0.05, (temp, raw, back)

    random.seed(747)
    steady = Model(cfg)
    pump_values = []
    for _ in range(160):
        steady.update_temp(82.0 + random.uniform(-0.18, 0.18), adc_spread=5.0)
        steady.control()
        pump_values.append(steady.pump)
    assert steady.valid
    assert max(pump_values[-60:]) - min(pump_values[-60:]) <= 6
    assert steady.conf >= 80

    spike = Model(cfg)
    for _ in range(30):
        spike.update_temp(86.0 + random.uniform(-0.1, 0.1), adc_spread=4.0)
        spike.control()
    before = spike.ema
    for _ in range(5):
        spike.update_temp(20.0, adc_spread=4.0)
        spike.control()
    assert spike.ema > before - 3.0
    assert spike.spike_count >= 5

    warmup = Model(cfg)
    fan_values = []
    for i in range(180):
        temp = 68.0 + i * 0.28
        warmup.update_temp(temp, adc_spread=4.0)
        warmup.control(ecu=(i > 80))
        fan_values.append(warmup.fan)
    assert warmup.pump > 70
    assert warmup.fan > 40
    assert all(b >= a - 3 for a, b in zip(fan_values[-80:], fan_values[-79:]))

    fail_safe = Model(cfg)
    for _ in range(8):
        fail_safe.update_temp(84.0 + random.uniform(-0.08, 0.08), adc_spread=4.0)
        fail_safe.control()
    assert fail_safe.valid
    fail_safe.pump = 12
    fail_safe.fan = 8
    for _ in range(SENSOR_BAD_LIMIT):
        fail_safe.update_temp(math.nan, adc_spread=ADC_MAX)
        fail_safe.control()
    assert not fail_safe.valid
    assert fail_safe.forced
    assert fail_safe.pump == 100 and fail_safe.fan == 100

    critical = Model(cfg)
    critical.valid = True
    critical.ema = cfg["critical_temp_c"] + 1.0
    critical.pump = 15
    critical.fan = 10
    critical.control()
    assert critical.forced
    assert critical.pump == 100 and critical.fan == 100

    long_run = Model(cfg)
    long_min = 100
    long_max = 0
    for i in range(21600):
        thermal = 84.0 + math.sin(i / 700.0) * 9.0 + math.sin(i / 57.0) * 0.7
        long_run.update_temp(thermal + random.uniform(-0.22, 0.22), adc_spread=6.0)
        long_run.control(ecu=(i % 900) > 720)
        assert math.isfinite(long_run.ema)
        assert math.isfinite(long_run.trend)
        assert 0 <= long_run.pump <= 100
        assert 0 <= long_run.fan <= 100
        long_min = min(long_min, long_run.pump, long_run.fan)
        long_max = max(long_max, long_run.pump, long_run.fan)
    assert long_run.valid
    assert long_max <= 100 and long_min >= 0

    print(json.dumps({
        "calibration_roundtrip": "ok",
        "steady_jitter_pct": max(pump_values[-60:]) - min(pump_values[-60:]),
        "steady_quality_pct": round(steady.conf, 1),
        "spike_rejects": spike.spike_count,
        "sensor_fail_safe": "immediate_100_after_invalid_limit",
        "critical_fail_safe": "immediate_100",
        "long_run_ticks": 21600,
        "warmup_pump_pct": warmup.pump,
        "warmup_fan_pct": warmup.fan,
    }, separators=(",", ":")))


if __name__ == "__main__":
    run()

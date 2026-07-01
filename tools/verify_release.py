import json
import re
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
ACTIVE_SUFFIXES = {".cpp", ".h", ".html", ".json", ".md", ".py", ".ini", ".csv"}
COMPONENT_INCLUDE = Path("components/smartcooling/include")
FIRMWARE_MAIN = Path("main/main.cpp")
REQUIRED_ROUTES = {
    "login", "recover", "password", "device", "status", "config", "preview",
    "save", "rgb", "events", "ota", "ws", "reboot",
}


def fail(message):
    raise SystemExit(message)


def active_files():
    for path in ROOT.rglob("*"):
        if not path.is_file() or path.suffix.lower() not in ACTIVE_SUFFIXES:
            continue
        rel = path.relative_to(ROOT)
        parts = set(rel.parts)
        if ".pio" in parts or ".vscode" in parts or "archive" in parts:
            continue
        if rel.as_posix() in {"components/smartcooling/include/web_assets.h", "tools/verify_release.py"}:
            continue
        yield path


def verify_routes():
    routes = json.loads((ROOT / "config/routes.json").read_text(encoding="utf-8"))
    missing = sorted(REQUIRED_ROUTES - set(routes))
    if missing:
        fail("missing routes: " + ",".join(missing))
    values = list(routes.values())
    if len(set(values)) != len(values):
        fail("duplicate route path")
    bad = [value for value in values if not re.fullmatch(r"/[A-Za-z0-9]{6}", value)]
    if bad:
        fail("non-random route path: " + ",".join(bad))

    header = (ROOT / COMPONENT_INCLUDE / "routes.h").read_text(encoding="utf-8")
    for name, value in routes.items():
        needle = f'ROUTE_{name.upper()} = "{value}"'
        if needle not in header:
            fail("route header mismatch: " + name)

    assets = (ROOT / COMPONENT_INCLUDE / "web_assets.h").read_text(encoding="utf-8")
    if "__SMARTCOOLING_ROUTES__" in assets:
        fail("web asset route token still present")
    for value in values:
        if value not in assets:
            fail("route missing from web asset: " + value)


def verify_i18n():
    html = (ROOT / "web/index.html").read_text(encoding="utf-8")
    match = re.search(r"const I18N = \{\s*ms:\{(?P<ms>.*?)\},\s*en:\{(?P<en>.*?)\}\s*\};", html, re.S)
    if not match:
        fail("I18N block not found")
    ms = set(re.findall(r'"([^"]+)":', match.group("ms")))
    en = set(re.findall(r'"([^"]+)":', match.group("en")))
    if ms != en:
        fail("I18N mismatch")
    used = set(re.findall(r'data-i18n="([^"]+)"', html))
    missing = sorted(used - ms)
    if missing:
        fail("I18N missing keys: " + ",".join(missing))


def verify_text_clean():
    blocked = [
        "con" + "toh",
        "exa" + "mple",
        "de" + "mo",
        "place" + "holder",
        "TO" + "DO",
        "FIX" + "ME",
        "Aero" + "space",
        "Automotive HMI",
        "SSID: " + "SmartCooling",
        "rec" + "Serial",
        "/" + "api/",
        "ewp-" + "control",
        "SmartCooling " + "V" + "3",
        "Console " + "V" + "3",
        "-" + "v" + "3",
        "smartcooling-" + "v" + "3",
    ]
    hits = []
    for path in active_files():
        text = path.read_text(encoding="utf-8", errors="ignore")
        for term in blocked:
            if term in text:
                hits.append(f"{path.relative_to(ROOT)}: {term}")
    if hits:
        fail("blocked release text:\n" + "\n".join(hits))


def verify_browser_storage():
    html = (ROOT / "web/index.html").read_text(encoding="utf-8")
    if "sessionStorage" in html:
        fail("sessionStorage is not allowed for WebApp release")
    if "sc_token" in html or "localStorage.token" in html or "localStorage.sc_token" in html:
        fail("session token must not be persisted in browser storage")
    allowed = {"sc_lang", "sc_theme"}
    used = set(re.findall(r"localStorage\.([A-Za-z0-9_]+)", html))
    bad = sorted(used - allowed)
    if bad:
        fail("unexpected localStorage keys: " + ",".join(bad))


def verify_required_files():
    required = [
        "CMakeLists.txt",
        "main/CMakeLists.txt",
        "main/main.cpp",
        "components/smartcooling/CMakeLists.txt",
        "components/smartcooling/library.json",
        "web/index.html",
        "components/smartcooling/include/factory_config.h",
        "components/smartcooling/include/web_assets.h",
        "components/smartcooling/include/routes.h",
        "components/smartcooling/include/si_core.h",
        "components/smartcooling/include/si_sentinel.h",
        "components/smartcooling/src/si_core.cpp",
        "components/smartcooling/src/si_sentinel.cpp",
        "config/routes.json",
        "docs/MANUAL_PENGGUNAAN.md",
        "docs/PANDUAN_PEMBANGUN.md",
        "tools/verify_control_math.py",
        "sdkconfig.defaults",
    ]
    missing = [item for item in required if not (ROOT / item).is_file()]
    if missing:
        fail("missing files: " + ",".join(missing))


def verify_gitignore():
    text = (ROOT / ".gitignore").read_text(encoding="utf-8")
    if "```" in text:
        fail(".gitignore contains markdown fences")
    required = [".pio/", ".vscode/", "docs/archive/", "*.bin", "*.elf", "*.map", "sdkconfig", "managed_components/"]
    missing = [item for item in required if item not in text]
    if missing:
        fail(".gitignore missing required entries: " + ",".join(missing))


def verify_factory_config():
    factory = (ROOT / COMPONENT_INCLUDE / "factory_config.h").read_text(encoding="utf-8")
    src = (ROOT / FIRMWARE_MAIN).read_text(encoding="utf-8")
    required = [
        '#define SC_FACTORY_PROFILE "super_mini_esp32_s3_hw747"',
        '#define SC_FACTORY_HARDWARE_REV "hw-747"',
        '#define SC_FACTORY_AP_SSID "EWP-SYSTEM-PRO"',
        "#define SC_FACTORY_AP_OPEN 1",
        "#define SC_FACTORY_AP_START_RETRY_COUNT 3",
        "#define SC_FACTORY_AP_RETRY_DELAY_MS 150UL",
        '#define SC_FACTORY_DEFAULT_WEB_PASSWORD "12345678"',
        '#define SC_FACTORY_RECOVERY_PIN "747747"',
        "#define SC_FACTORY_RGB_ENABLED 1",
        "#define SC_FACTORY_PIN_RGB 48",
        "#define SC_FACTORY_RGB_BRIGHTNESS12 4",
        "#define SC_FACTORY_PUMP_TYPE_NONE 0",
        "#define SC_FACTORY_PUMP_TYPE_SSR 1",
        "#define SC_FACTORY_PUMP_TYPE_PWM 2",
        "#define SC_FACTORY_PUMP_TYPE SC_FACTORY_PUMP_TYPE_NONE",
        "#define SC_FACTORY_FAN_TYPE_NONE 0",
        "#define SC_FACTORY_FAN_TYPE_SSR 1",
        "#define SC_FACTORY_FAN_TYPE_PWM 2",
        "#define SC_FACTORY_FAN_TYPE SC_FACTORY_FAN_TYPE_NONE",
        "#define SC_FACTORY_ENV_SENSOR_TYPE SC_FACTORY_SENSOR_NONE",
        "#define SC_FACTORY_SD_CARD_ENABLED 0",
        "#define SC_FACTORY_PIN_NTC 1",
        "#define SC_FACTORY_PIN_ECU 6",
        "#define SC_FACTORY_PIN_PUMP_PWM -1",
        "#define SC_FACTORY_PIN_PUMP_SSR -1",
        "#define SC_FACTORY_PIN_FAN_PWM -1",
        "#define SC_FACTORY_PIN_FAN_SSR -1",
        "#define SC_FACTORY_PIN_SSR SC_FACTORY_PIN_PUMP_SSR",
        "#define SC_FACTORY_PIN_PWM SC_FACTORY_PIN_FAN_PWM",
        "#define SC_FACTORY_AP_IDLE_OFF_MS 300000UL",
        "#define SC_FACTORY_RESET_PIN GPIO_NUM_0",
        "#define SC_FACTORY_RESET_ACTIVE_LOW true",
    ]
    missing = [item for item in required if item not in factory]
    if missing:
        fail("factory config mismatch: " + ",".join(missing))
    for needle in [
        '#include "factory_config.h"',
        "static constexpr const char *AP_SSID = SC_FACTORY_AP_SSID;",
        "static constexpr const char *RECOVERY_PIN = SC_FACTORY_RECOVERY_PIN;",
        "WiFi.setSleep(false);",
        "WiFi.setTxPower(WIFI_POWER_19_5dBm);",
        "neopixelWrite(PIN_RGB",
        "#elif SC_FACTORY_PUMP_TYPE == SC_FACTORY_PUMP_TYPE_SSR",
        "#elif SC_FACTORY_FAN_TYPE == SC_FACTORY_FAN_TYPE_SSR",
        "#define PIN_PUMP_PWM -1",
        "#define PIN_PUMP_SSR -1",
        "#define PIN_FAN_PWM -1",
        "#define PIN_FAN_SSR -1",
        "ledcAttachPin(PIN_FAN_PWM, PWM_CHAN_FAN);",
    ]:
        if needle not in src:
            fail("firmware not linked to factory config: " + needle)
    rejected = ["PIN_NTC_COOLANT", "PIN_SSR_PUMP", "PIN_SSR_FAN", "SYSTEM_RECOVERY_PIN", "#define PIN_SSR", "#define PIN_PWM"]
    hits = [needle for needle in rejected if needle in src]
    if hits:
        fail("unsafe factory branch symbols in firmware: " + ",".join(hits))


def main():
    verify_required_files()
    verify_gitignore()
    verify_factory_config()
    verify_routes()
    verify_i18n()
    verify_text_clean()
    verify_browser_storage()
    print(json.dumps({"release_static_gate": "ok"}, separators=(",", ":")))


if __name__ == "__main__":
    main()

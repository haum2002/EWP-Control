import json
import re
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
ACTIVE_SUFFIXES = {".cpp", ".h", ".html", ".json", ".md", ".py", ".ini", ".csv"}
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
        if rel.as_posix() in {"include/web_assets.h", "tools/verify_release.py"}:
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

    header = (ROOT / "include/routes.h").read_text(encoding="utf-8")
    for name, value in routes.items():
        needle = f'ROUTE_{name.upper()} = "{value}"'
        if needle not in header:
            fail("route header mismatch: " + name)

    assets = (ROOT / "include/web_assets.h").read_text(encoding="utf-8")
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


def verify_required_files():
    required = [
        "web/index.html",
        "src/main.cpp",
        "include/web_assets.h",
        "include/routes.h",
        "config/routes.json",
        "docs/MANUAL_PENGGUNAAN.md",
        "docs/PANDUAN_PEMBANGUN.md",
        "tools/verify_control_math.py",
    ]
    missing = [item for item in required if not (ROOT / item).is_file()]
    if missing:
        fail("missing files: " + ",".join(missing))


def main():
    verify_required_files()
    verify_routes()
    verify_i18n()
    verify_text_clean()
    print(json.dumps({"release_static_gate": "ok"}, separators=(",", ":")))


if __name__ == "__main__":
    main()

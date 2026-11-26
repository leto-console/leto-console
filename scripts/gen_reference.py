#!/usr/bin/env python3
"""Генерирует docs/reference/presets.md из CMakePresets.json.

Список пресетов — факт, который переименовывают чаще, чем читают документацию.
Поэтому он не пишется руками: файл собирается из манифеста, а CI проверяет, что
закоммиченная версия совпадает с результатом генерации.

    python3 scripts/gen_reference.py           # записать
    python3 scripts/gen_reference.py --check   # только сравнить (код возврата 1 при расхождении)
"""
from __future__ import annotations

import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
MANIFEST = ROOT / "CMakePresets.json"
TARGET = ROOT / "docs" / "reference" / "presets.md"
def collect() -> dict[str, dict]:
    data = json.loads(MANIFEST.read_text(encoding="utf-8"))
    presets: dict[str, dict] = {}
    for entry in data.get("configurePresets", []):
        name = entry.get("name")
        if not name:
            continue
        presets[name] = {
            "inherits": entry.get("inherits", ""),
            "hidden": bool(entry.get("hidden")),
            "cache": entry.get("cacheVariables", {}),
        }
    return presets


def render() -> str:
    presets = collect()
    lines = [
        "<!-- Сгенерировано scripts/gen_reference.py из CMakePresets.json. Не редактировать вручную. -->",
        "",
        "# Пресеты leto-console",
        "",
        "| Пресет | Наследует | Сборка | Плата | Экран | Линкер | Макро |",
        "| --- | --- | --- | --- | --- | --- | --- |",
    ]
    for name, info in presets.items():
        if info["hidden"]:
            continue
        cache = info["cache"]
        lines.append(
            "| `{name}` | {inherits} | {build} | {chip} | {screen} | {linker} | {macro} |".format(
                name=name,
                inherits=f"`{info['inherits']}`" if info["inherits"] else "—",
                build=cache.get("CMAKE_BUILD_TYPE", "—"),
                chip=cache.get("MCU_CHIP", "—"),
                screen=cache.get("USE_SCREEN", "—"),
                linker=cache.get("LINKER_SCRIPT", "—"),
                macro=cache.get("PRESET_DEFINE_MACRO", "—"),
            )
        )
    lines += ["", "Скрытые (`hidden`) пресеты — базовые, они не собираются напрямую.", ""]
    return "\n".join(lines)


def main() -> int:
    rendered = render()
    if "--check" in sys.argv:
        current = TARGET.read_text(encoding="utf-8") if TARGET.is_file() else ""
        if current != rendered:
            print(f"ERROR {TARGET.relative_to(ROOT)} устарел: запустить scripts/gen_reference.py")
            return 1
        print(f"OK {TARGET.relative_to(ROOT)} актуален")
        return 0
    TARGET.parent.mkdir(parents=True, exist_ok=True)
    TARGET.write_text(rendered, encoding="utf-8")
    print(f"written {TARGET.relative_to(ROOT)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())

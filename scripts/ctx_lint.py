#!/usr/bin/env python3
"""Линтер базы знаний агента: пути, пресеты, скиллы, бюджеты размера, пометка AI.

Проверяет только репозиторий, в котором лежит. Результат не зависит от окружения:
ни `LETO_PATH`, ни клонированных соседей, ни платформы. Поэтому локальный прогон
и CI дают одно и то же, и «зелено в CI, но не локально» не бывает. Ссылки на файлы
других репозиториев (`LetoCore/...`) — ориентиры для агента, а не утверждения:
их проверка требует чужих клонов на нужной ветке, то есть заведомо нестабильна.

Пресеты читаются из `CMakePresets.json`, а не из `cmake --list-presets`:
тот скрывает пресеты с `condition` (на Linux не видно `win-*`), и проверка
снова стала бы платформо-зависимой.

    python3 scripts/ctx_lint.py .            # обычная проверка
    python3 scripts/ctx_lint.py . --issues   # плюс проверка ссылок на issue через gh
"""
from __future__ import annotations

import json
import re
import subprocess
import sys
from pathlib import Path

REPO_NAME = "leto-console"
MAX_AGENTS_LINES = 120
MAX_SKILL_LINES = 200
MAX_SKILL_BODY = 80
MAX_SKILLS = 12
MIN_DESCRIPTION = 40
AI_MARKER = "Написано AI-агентом"

PATH_RE = re.compile(r"`([A-Za-z][\w./@{},-]*/[\w./@{},-]+)`")
PRESET_RE = re.compile(r"--preset[\"'=\s]+([\w.-]+)")
ISSUE_RE = re.compile(r"(?<![\w`/])#(\d{2,4})(?![\w`])")
PRESET_SECTIONS = ("configurePresets", "buildPresets", "testPresets")

errors: list[str] = []
warnings: list[str] = []


def err(msg: str) -> None:
    errors.append(msg)


def warn(msg: str) -> None:
    warnings.append(msg)


def repo_files(root: Path) -> set[str] | None:
    """Файлы под контролем версий: git знает, что существует в репозитории."""
    result = subprocess.run(
        ["git", "-C", str(root), "ls-files", "-z"], capture_output=True, text=True
    )
    if result.returncode != 0:
        return None
    return {path for path in result.stdout.split("\0") if path}


def preset_names(root: Path) -> set[str]:
    manifest = root / "CMakePresets.json"
    if not manifest.is_file():
        err("CMakePresets.json отсутствует")
        return set()
    try:
        data = json.loads(manifest.read_text(encoding="utf-8"))
    except json.JSONDecodeError as exc:
        err(f"CMakePresets.json: невалидный JSON ({exc})")
        return set()
    return {
        preset["name"]
        for section in PRESET_SECTIONS
        for preset in data.get(section) or []
        if isinstance(preset, dict) and preset.get("name")
    }


def local_path(token: str) -> str:
    for prefix in (f"{REPO_NAME}/", f"{Path.cwd().name}/"):
        if token.startswith(prefix):
            return token[len(prefix):]
    return token


def is_claim(token: str, files: set[str]) -> bool:
    """Утверждение о пути внутри этого репозитория, а не ссылка на чужой."""
    if any(ch in token for ch in "<>{}$*?|"):
        return False
    head = token.split("/", 1)[0]
    if head == REPO_NAME:
        # `owner/repo` — ссылка на GitHub, а не путь внутри клона.
        return token != f"{REPO_NAME}/{REPO_NAME}"
    return any(path == head or path.startswith(f"{head}/") for path in files)


def exists(token: str, files: set[str]) -> bool:
    if token in files:
        return True
    return any(path.startswith(token.rstrip("/") + "/") for path in files)


def check_doc(path: Path, files: set[str] | None, presets: set[str], budget: int) -> None:
    text = path.read_text(encoding="utf-8")

    if len(text.splitlines()) > budget:
        err(f"{path}: {len(text.splitlines())} строк при бюджете {budget} — хвост не читается")

    if files is not None:
        for raw in PATH_RE.findall(text):
            token = raw.rstrip(".,;:)")
            if not is_claim(token, files):
                continue
            if not exists(local_path(token), files):
                err(f"{path}: путь `{token}` не существует")

    for preset in PRESET_RE.findall(text):
        if preset not in presets:
            err(f"{path}: пресета `{preset}` нет в CMakePresets.json")

    for stale in ("Текущее состояние", "Current state"):
        if stale in text:
            warn(f"{path}: раздел «{stale}» описывает состояние текстом — заменить проверкой")


def check_skill(path: Path, files: set[str] | None, presets: set[str]) -> None:
    text = path.read_text(encoding="utf-8")
    if not text.startswith("---\n"):
        err(f"{path}: нет frontmatter")
        return
    try:
        _, frontmatter, body = text.split("---\n", 2)
    except ValueError:
        err(f"{path}: незакрытый frontmatter")
        return

    meta = {}
    for line in frontmatter.splitlines():
        if ":" in line and not line.startswith((" ", "\t")):
            key, value = line.split(":", 1)
            meta[key.strip()] = value.strip()

    if meta.get("name") != path.parent.name:
        err(f"{path}: name={meta.get('name')!r} не совпадает с каталогом {path.parent.name!r}")

    description = meta.get("description", "")
    if len(description) < MIN_DESCRIPTION:
        err(f"{path}: description короче {MIN_DESCRIPTION} символов — по нему скилл не вызовется")
    if "Использовать" not in description and "Use when" not in description:
        warn(f"{path}: в description нет ситуации-триггера («Использовать, когда…»)")

    check_doc(path, files, presets, MAX_SKILL_LINES)

    if len(body.splitlines()) > MAX_SKILL_BODY:
        warn(f"{path}: тело больше {MAX_SKILL_BODY} строк — вынести детали в файлы рядом")


def check_router_and_skills(root: Path, skills: list[Path]) -> None:
    """Таблица скиллов в AGENTS.md и каталог скиллов не расходятся."""
    agents = root / "AGENTS.md"
    if not agents.is_file():
        err("AGENTS.md отсутствует")
        return
    text = agents.read_text(encoding="utf-8")
    section = text.split("## Скиллы", 1)[-1].split("\n## ", 1)[0]
    listed = set(re.findall(r"`(leto-[a-z0-9-]+)`", section))
    dirs = {path.parent.name for path in skills}
    if "## Скиллы" not in text:
        err("AGENTS.md: нет раздела «Скиллы» с таблицей маршрутизации")
    for name in sorted(dirs - listed):
        err(f"AGENTS.md: скилл `{name}` есть, но не указан в таблице")
    for name in sorted(listed - dirs):
        err(f"AGENTS.md: указан скилл `{name}`, но каталога .agents/skills/{name} нет")


def check_ai_marker(root: Path) -> None:
    variants = set()
    for path in list(root.glob("*.md")) + list(root.glob(".agents/skills/*/SKILL.md")):
        for line in path.read_text(encoding="utf-8").splitlines():
            if AI_MARKER in line:
                variants.add(line.strip().lstrip(">").strip().strip("`\"«»").strip())
    if len(variants) > 1:
        err(f"пометка AI встречается в {len(variants)} вариантах — нужна одна формулировка")


def check_issues(root: Path) -> None:
    refs = set()
    for path in list(root.glob("*.md")) + list(root.glob("docs/**/*.md")):
        refs.update(ISSUE_RE.findall(path.read_text(encoding="utf-8")))
    for number in sorted(refs):
        result = subprocess.run(
            ["gh", "api", f"repos/{REPO_NAME}/{REPO_NAME}/issues/{number}", "--jq", ".number"],
            capture_output=True, text=True,
        )
        if result.returncode != 0:
            err(f"issue #{number} не открывается: {result.stderr.strip()[:80]}")


def main() -> int:
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    root = Path(args[0] if args else ".").resolve()

    files = repo_files(root)
    if files is None:
        warn("git ls-files недоступен — пути не проверяются")
    presets = preset_names(root)

    agents = root / "AGENTS.md"
    if agents.is_file():
        check_doc(agents, files, presets, MAX_AGENTS_LINES)

    skills = sorted(root.glob(".agents/skills/*/SKILL.md"))
    if len(skills) > MAX_SKILLS:
        err(f"{len(skills)} скиллов при потолке {MAX_SKILLS} — выбор перестанет быть однозначным")
    for skill in skills:
        check_skill(skill, files, presets)

    check_router_and_skills(root, skills)
    check_ai_marker(root)
    if "--issues" in sys.argv:
        check_issues(root)

    for msg in warnings:
        print(f"WARN {msg}")
    for msg in errors:
        print(f"ERROR {msg}")
    print(f"{len(skills)} skills, {len(presets)} presets, {len(errors)} errors, {len(warnings)} warnings")
    return 1 if errors else 0


if __name__ == "__main__":
    sys.exit(main())

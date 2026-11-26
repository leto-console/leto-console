#!/usr/bin/env python3
"""Проверка самой базы знаний агента: пути, пресеты, скиллы, бюджеты размера.

База знаний, которую никто не проверяет, расходится с кодом: команда переименована,
путь удалён, пресет переименован — а текст утверждает обратное. Этот линтер ловит
такое на PR, а не через месяц при чтении устаревшей инструкции.

    python3 scripts/ctx_lint.py .            # обычная проверка
    python3 scripts/ctx_lint.py . --issues   # плюс проверка ссылок на issue через gh
"""
from __future__ import annotations

import json
import os
import re
import subprocess
import sys
from pathlib import Path

REPOS = ["LetoAPI", "LetoCore", "LetoOS", "leto-console", "game-template"]
MAX_AGENTS_LINES = 120
MAX_SKILL_LINES = 200
MAX_SKILL_BODY = 80
MIN_DESCRIPTION = 40
MAX_SKILLS = 12
AI_MARKER = "Написано AI-агентом"

# Путь проверяется, только если он обещает быть файлом/каталогом репозитория:
# `origin/main`, `leto-games/*` и подобное — не пути, а упоминания.
PATH_RE = re.compile(r"`([A-Za-z][\w./@{},-]*/[\w./@{},-]+)`")
PRESET_RE = re.compile(r"--preset[\"'=\s]+([\w.-]+)")
ISSUE_RE = re.compile(r"(?<![\w`/])#(\d{2,4})(?![\w`])")

PREFIXES = (
    "LetoAPI/", "LetoCore/", "LetoOS/", "leto-console/", "game-template/",
    "Core/", "Tests/", "scripts/", "cmake/", "docs/", "include/", "source/",
)
SUFFIXES = (
    ".hpp", ".h", ".cpp", ".c", ".sh", ".py", ".json", ".yml", ".yaml", ".md",
    ".txt", ".cmake", ".ps1", ".bat", ".ioc", ".ld",
)

errors: list[str] = []
warnings: list[str] = []


def err(msg: str) -> None:
    errors.append(msg)


def warn(msg: str) -> None:
    warnings.append(msg)


def repo_roots(root: Path) -> list[Path]:
    leto = Path(os.environ.get("LETO_PATH", str(Path.home() / "leto"))).resolve()
    roots = [root.resolve()]
    for base in (leto, leto / "leto-console"):
        for repo in REPOS:
            if (base / repo).is_dir():
                roots.append(base / repo)
        if base.is_dir():
            roots.append(base)
    return roots


def preset_sources() -> list[str]:
    """Репозитории, чьи пресеты имеет смысл проверять в тексте."""
    return REPOS[:4]


def load_presets(roots: list[Path]) -> set[str]:
    presets: set[str] = set()
    for root in roots:
        manifest = root / "CMakePresets.json"
        if not manifest.is_file():
            continue
        try:
            data = json.loads(manifest.read_text(encoding="utf-8"))
        except json.JSONDecodeError as exc:
            err(f"{manifest}: невалидный JSON ({exc})")
            continue
        for key in ("configurePresets", "buildPresets", "testPresets"):
            for preset in data.get(key, []):
                if preset.get("name") and not preset.get("hidden"):
                    presets.add(preset["name"])
    return presets


def repo_cloned(repo: str, roots: list[Path]) -> bool:
    return any(root.name == repo for root in roots)


def is_path_claim(token: str) -> bool:
    if any(ch in token for ch in "<>{}$*?|"):
        return False
    if not token.startswith(PREFIXES):
        return False
    return token.endswith("/") or token.endswith(SUFFIXES)


def check_doc(path: Path, roots: list[Path], presets: set[str], budget: int) -> None:
    text = path.read_text(encoding="utf-8")
    lines = text.splitlines()

    if len(lines) > budget:
        err(f"{path}: {len(lines)} строк при бюджете {budget} — хвост файла не читается")

    for raw in PATH_RE.findall(text):
        token = raw.rstrip(".,;:)")
        if not is_path_claim(token):
            continue
        candidates = [token]
        for repo in REPOS:
            if token.startswith(f"{repo}/"):
                candidates.append(token[len(repo) + 1:])
        if any((root / candidate).exists() for root in roots for candidate in candidates):
            continue
        owner = token.split("/", 1)[0]
        if owner in REPOS and not repo_cloned(owner, roots):
            warn(f"{path}: `{token}` не проверен — репозиторий {owner} не клонирован рядом")
            continue
        err(f"{path}: путь `{token}` не существует")

    missing_repos = [r for r in preset_sources() if not repo_cloned(r, roots)]
    for preset in PRESET_RE.findall(text):
        if not presets or preset in presets:
            continue
        if missing_repos:
            warn(f"{path}: пресет `{preset}` не проверен — не клонированы: {', '.join(missing_repos)}")
            continue
        err(f"{path}: пресет `{preset}` отсутствует в CMakePresets.json")

    for stale in ("Текущее состояние", "Current state"):
        if stale in text:
            warn(f"{path}: раздел «{stale}» описывает состояние кодом текстом — заменить проверкой")


def check_skill(path: Path, roots: list[Path], presets: set[str]) -> None:
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

    check_doc(path, roots, presets, MAX_SKILL_LINES)

    if len(body.splitlines()) > MAX_SKILL_BODY:
        warn(f"{path}: тело больше {MAX_SKILL_BODY} строк — вынести детали в файлы рядом")


def normalize_marker(line: str) -> str:
    return line.strip().lstrip(">").strip().strip("`\"«»").strip()


def check_ai_marker(root: Path) -> None:
    variants = set()
    for path in list(root.glob("*.md")) + list(root.glob(".agents/skills/*/SKILL.md")):
        for line in path.read_text(encoding="utf-8").splitlines():
            if "AI-агентом" in line:
                variants.add(normalize_marker(line))
    if len(variants) > 1:
        err(f"пометка AI встречается в {len(variants)} вариантах — нужна одна формулировка")


def check_issues(root: Path) -> None:
    refs = set()
    for path in list(root.glob("*.md")) + list(root.glob("docs/**/*.md")):
        refs.update(ISSUE_RE.findall(path.read_text(encoding="utf-8")))
    for number in sorted(refs):
        result = subprocess.run(
            ["gh", "api", f"repos/leto-console/leto-console/issues/{number}", "--jq", ".number"],
            capture_output=True, text=True,
        )
        if result.returncode != 0:
            err(f"issue #{number} не открывается: {result.stderr.strip()[:80]}")


def main() -> int:
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    root = Path(args[0] if args else ".")
    roots = repo_roots(root)
    presets = load_presets(roots)

    agents = root / "AGENTS.md"
    if not agents.is_file():
        err("AGENTS.md отсутствует")
    else:
        check_doc(agents, roots, presets, MAX_AGENTS_LINES)

    skills = sorted(root.glob(".agents/skills/*/SKILL.md"))
    if not skills:
        warn("скиллов нет: все процедуры остаются в AGENTS.md и читаются целиком")
    if len(skills) > MAX_SKILLS:
        err(f"{len(skills)} скиллов при потолке {MAX_SKILLS} — выбор перестанет быть однозначным")
    for skill in skills:
        check_skill(skill, roots, presets)

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

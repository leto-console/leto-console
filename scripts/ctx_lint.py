#!/usr/bin/env python3
"""Проверка базы знаний агента: пути, пресеты, скиллы, бюджеты размера.

База знаний, которую никто не проверяет, расходится с кодом: команда
переименована, путь удалён, пресет удалён — а текст утверждает обратное. Этот
линтер ловит такое на PR, а не через месяц при чтении устаревшей инструкции.

Список пресетов не хранится и не парсится: его отдаёт `cmake --list-presets`.
Файлов, которые надо поддерживать вручную, у линтера нет.

    python3 scripts/ctx_lint.py .            # обычная проверка
    python3 scripts/ctx_lint.py . --issues   # плюс проверка ссылок на issue через gh
"""
from __future__ import annotations

import os
import re
import subprocess
import sys
from pathlib import Path

REPOS = ["LetoAPI", "LetoCore", "LetoOS", "leto-console"]
ALL_REPOS = REPOS + ["game-template", "Battleships"]
MAX_AGENTS_LINES = 120
MAX_SKILL_LINES = 200
MAX_SKILL_BODY = 80
MIN_DESCRIPTION = 40
MAX_SKILLS = 12
AI_MARKER = "Написано AI-агентом"

PATH_RE = re.compile(r"`([A-Za-z][\w./@{},-]*/[\w./@{},-]+)`")
PRESET_RE = re.compile(r"--preset[\"'=\s]+([\w.-]+)")
ISSUE_RE = re.compile(r"(?<![\w`/])#(\d{2,4})(?![\w`])")

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


def cloned(repo: str, roots: list[Path]) -> Path | None:
    for root in roots:
        if root.name == repo:
            return root
    return None


def cmake_presets(repo_dir: Path) -> set[str] | None:
    """Пресеты так, как их видит CMake; None — спросить не удалось."""
    try:
        result = subprocess.run(
            ["cmake", "--list-presets"], cwd=repo_dir,
            capture_output=True, text=True, timeout=120,
        )
    except (OSError, subprocess.SubprocessError):
        return None
    if result.returncode != 0:
        return None
    return set(re.findall(r'^\s+"([^"]+)"', result.stdout, re.M))


def collect_presets(roots: list[Path]) -> tuple[set[str], list[str]]:
    presets: set[str] = set()
    unqueried: list[str] = []
    for repo in REPOS:
        path = cloned(repo, roots)
        if path is None:
            unqueried.append(repo)
            continue
        listed = cmake_presets(path)
        if listed is None:
            unqueried.append(repo)
            continue
        presets.update(listed)
    return presets, unqueried


def tracked_paths(roots: list[Path]) -> dict[Path, set[str]]:
    """Файлы под контролем версий: git знает, что существует в проекте."""
    tracked: dict[Path, set[str]] = {}
    for root in roots:
        if not (root / ".git").exists():
            continue
        result = subprocess.run(
            ["git", "-C", str(root), "ls-files", "-z"],
            capture_output=True, text=True,
        )
        if result.returncode == 0:
            tracked[root] = {p for p in result.stdout.split("\0") if p}
    return tracked


def resolves(token: str, roots: list[Path], tracked: dict[Path, set[str]]) -> bool:
    for root in roots:
        paths = tracked.get(root)
        if paths is None:
            if (root / token).exists():
                return True
            continue
        if token in paths or any(p.startswith(token.rstrip("/") + "/") for p in paths):
            return True
    return False


def is_path_claim(token: str, roots: list[Path], tracked: dict[Path, set[str]]) -> bool:
    if any(ch in token for ch in "<>{}$*?|"):
        return False
    head = token.split("/", 1)[0]
    if head in ALL_REPOS:
        # `owner/repo` в тексте — ссылка на GitHub, а не путь внутри клона.
        return token != f"{head}/{head}"
    # Первый сегмент — каталог верхнего уровня, который реально есть в репозитории.
    return any(head == p.split("/", 1)[0] for paths in tracked.values() for p in paths)


def check_doc(path: Path, roots: list[Path], presets: set[str], unqueried: list[str],
              budget: int, tracked: dict[Path, set[str]]) -> None:
    text = path.read_text(encoding="utf-8")

    if len(text.splitlines()) > budget:
        err(f"{path}: {len(text.splitlines())} строк при бюджете {budget} — хвост файла не читается")

    for raw in PATH_RE.findall(text):
        token = raw.rstrip(".,;:)")
        if not is_path_claim(token, roots, tracked):
            continue
        candidates = [token]
        for repo in REPOS:
            if token.startswith(f"{repo}/"):
                candidates.append(token[len(repo) + 1:])
        if any(resolves(candidate, roots, tracked) for candidate in candidates):
            continue
        owner = token.split("/", 1)[0]
        if owner in ALL_REPOS and cloned(owner, roots) is None:
            warn(f"{path}: `{token}` не проверен — репозиторий {owner} не клонирован рядом")
            continue
        err(f"{path}: путь `{token}` не существует")

    for preset in PRESET_RE.findall(text):
        if preset in presets:
            continue
        if unqueried:
            warn(f"{path}: пресет `{preset}` не проверен — нет пресетов: {', '.join(unqueried)}")
            continue
        err(f"{path}: пресета `{preset}` нет в `cmake --list-presets`")

    for stale in ("Текущее состояние", "Current state"):
        if stale in text:
            warn(f"{path}: раздел «{stale}» описывает состояние текстом — заменить проверкой")


def check_skill(path: Path, roots: list[Path], presets: set[str], unqueried: list[str],
                tracked: dict[Path, set[str]]) -> None:
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

    check_doc(path, roots, presets, unqueried, MAX_SKILL_LINES, tracked)

    if len(body.splitlines()) > MAX_SKILL_BODY:
        warn(f"{path}: тело больше {MAX_SKILL_BODY} строк — вынести детали в файлы рядом")


def check_router_lists_skills(root: Path, skills: list[Path]) -> None:
    """Таблица скиллов в AGENTS.md и каталог скиллов не должны расходиться."""
    agents = root / "AGENTS.md"
    if not agents.is_file():
        return
    text = agents.read_text(encoding="utf-8")
    section = text.split("## Скиллы", 1)[-1].split("\n## ", 1)[0]
    listed = set(re.findall(r"`(leto-[a-z0-9-]+)`", section))
    dirs = {path.parent.name for path in skills}
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
            ["gh", "api", f"repos/leto-console/leto-console/issues/{number}", "--jq", ".number"],
            capture_output=True, text=True,
        )
        if result.returncode != 0:
            err(f"issue #{number} не открывается: {result.stderr.strip()[:80]}")


def main() -> int:
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    root = Path(args[0] if args else ".")
    roots = repo_roots(root)
    presets, unqueried = collect_presets(roots)
    tracked = tracked_paths(roots)

    agents = root / "AGENTS.md"
    if not agents.is_file():
        err("AGENTS.md отсутствует")
    else:
        check_doc(agents, roots, presets, unqueried, MAX_AGENTS_LINES, tracked)

    skills = sorted(root.glob(".agents/skills/*/SKILL.md"))
    if not skills:
        warn("скиллов нет: все процедуры остаются в AGENTS.md и читаются целиком")
    if len(skills) > MAX_SKILLS:
        err(f"{len(skills)} скиллов при потолке {MAX_SKILLS} — выбор перестанет быть однозначным")
    for skill in skills:
        check_skill(skill, roots, presets, unqueried, tracked)

    check_router_lists_skills(root, skills)
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

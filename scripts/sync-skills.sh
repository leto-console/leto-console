#!/usr/bin/env bash
# Доставляет .agents/skills из этого репозитория в соседние клоны LETO.
#
# Подмодулей нет: агент, открывший LetoOS или LetoCore, иначе не увидит скиллов.
# Источник один — leto-console; копии в остальных репозиториях не редактируются.
#
#   ./scripts/sync-skills.sh          # записать
#   ./scripts/sync-skills.sh --check  # только сравнить (код 1 при расхождении)
set -u

LETO_PATH="${LETO_PATH:-$HOME/leto/}"
SRC="$(cd "$(dirname "$0")/.." && pwd)/.agents/skills"
TARGETS=(LetoAPI LetoCore LetoOS)
MODE=write
[ "${1:-}" = "--check" ] && MODE=check

[ -d "$SRC" ] || { echo "нет источника: $SRC" >&2; exit 1; }

status=0
for repo in "${TARGETS[@]}"; do
    dst="$LETO_PATH/$repo/.agents/skills"
    if [ ! -d "$LETO_PATH/$repo" ]; then
        echo "skip $repo (нет клона в $LETO_PATH)"
        continue
    fi
    if [ "$MODE" = check ]; then
        if diff -r -q "$SRC" "$dst" >/dev/null 2>&1; then
            echo "ok   $repo"
        else
            echo "drift $repo: запустить ./scripts/sync-skills.sh" >&2
            status=1
        fi
    else
        rm -rf "$dst"
        mkdir -p "$dst"
        cp -R "$SRC"/. "$dst"/
        echo "synced $repo"
    fi
done
exit "$status"

#!/usr/bin/env bash
# Единая точка проверки LETO: сборка затронутых репозиториев по цепочке + ctest.
#
#   ./scripts/leto-check.sh                # репозитории с изменениями и всё, что ниже по цепочке
#   ./scripts/leto-check.sh --full         # все четыре + проверка выгрузки .so
#   ./scripts/leto-check.sh --repo LetoOS  # явно указанный репозиторий (можно несколько)
#   ./scripts/leto-check.sh --no-tests     # только сборка
set -u

LETO_PATH="${LETO_PATH:-$HOME/leto/}"
ORDER=(LetoAPI LetoCore LetoOS leto-console)
declare -A PRESET=(
    [LetoAPI]=linux-debug
    [LetoCore]=linux-debug
    [LetoOS]=linux-debug
    [leto-console]=linux-st7735-debug
)
declare -A HAS_TESTS=([LetoCore]=1 [LetoOS]=1)

MODE=changed
EXPLICIT=()
while [ $# -gt 0 ]; do
    case "$1" in
        --full) MODE=full; shift ;;
        --no-tests) HAS_TESTS=(); shift ;;
        --repo) EXPLICIT+=("$2"); shift 2 ;;
        -h|--help) sed -n '2,8p' "$0"; exit 0 ;;
        *) echo "usage: leto-check.sh [--full] [--no-tests] [--repo <name>]" >&2; exit 2 ;;
    esac
done

die() { printf 'FAIL %s\n' "$*" >&2; exit 1; }
log=$(mktemp)
trap 'rm -f "$log"' EXIT

run_logged() {   # run_logged <описание> <команда...>
    local desc=$1
    shift
    if ! "$@" >"$log" 2>&1; then
        tail -40 "$log" >&2
        die "$desc: см. лог выше"
    fi
}

chain_index() {
    local target=$1 i
    for i in "${!ORDER[@]}"; do
        [ "${ORDER[$i]}" = "$target" ] && { echo "$i"; return 0; }
    done
    return 1
}

if [ ${#EXPLICIT[@]} -gt 0 ]; then
    SELECTED=("${EXPLICIT[@]}")
elif [ "$MODE" = full ]; then
    SELECTED=("${ORDER[@]}")
else
    # Изменился LetoCore — пересобирать нужно и всё, что от него зависит.
    first=$((${#ORDER[@]} - 1))
    for repo in "${ORDER[@]}"; do
        [ -d "$LETO_PATH/$repo/.git" ] || continue
        [ -n "$(git -C "$LETO_PATH/$repo" status --porcelain)" ] || continue
        idx=$(chain_index "$repo") || continue
        [ "$idx" -lt "$first" ] && first=$idx
    done
    if [ "$first" -eq $((${#ORDER[@]} - 1)) ]; then
        last_dirty=$(git -C "$LETO_PATH/${ORDER[-1]}" status --porcelain 2>/dev/null)
        [ -n "$last_dirty" ] || { echo "OK nothing to check (clean tree)"; exit 0; }
    fi
    SELECTED=("${ORDER[@]:$first}")
fi

started=$(date +%s)
for repo in "${ORDER[@]}"; do
    printf '%s\n' "${SELECTED[@]}" | grep -qx "$repo" || continue
    dir="$LETO_PATH/$repo"
    preset="${PRESET[$repo]}"
    build="$dir/build/$preset"

    [ -d "$dir" ] || die "$repo: нет клона в $LETO_PATH"

    if [ ! -d "$build" ]; then
        echo "[$repo] configure $preset"
        run_logged "$repo" bash -c "cd '$dir' && cmake --preset '$preset'"
    fi

    echo "[$repo] build"
    run_logged "$repo" cmake --build "$build" -j

    if [ -n "${HAS_TESTS[$repo]:-}" ]; then
        echo "[$repo] ctest"
        run_logged "$repo" ctest --test-dir "$build" --output-on-failure -Q
    fi
done

if [ "$MODE" = full ]; then
    for so in "$LETO_PATH"/Apps/linux-debug/*/*.so; do
        [ -e "$so" ] || continue
        uniques=$(nm -C "$so" | grep -c ' u ')
        [ "$uniques" -eq 0 ] || die "$(basename "$so"): $uniques STB_GNU_UNIQUE (нет -fno-gnu-unique)"
        echo "[dso] $(basename "$so") unloadable"
    done
fi

echo "OK $(( $(date +%s) - started ))s"

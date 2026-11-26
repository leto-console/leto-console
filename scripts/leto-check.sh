#!/usr/bin/env bash
# Единая точка проверки LETO: сборка затронутых репозиториев по цепочке + ctest.
#
# Пресеты не перечислены списком: их отдаёт `cmake --list-presets`, linux-пресет
# в каждом репозитории один. Тесты определяются по наличию CTestTestfile.cmake.
set -u

usage() {
    sed -n '2,5p' "$0" | sed 's/^# \{0,1\}//'
    cat <<'TXT'

    ./scripts/leto-check.sh                 изменённые репозитории и всё, что ниже по цепочке
    ./scripts/leto-check.sh --full          все четыре + проверка выгрузки игры из .so
    ./scripts/leto-check.sh --repo LetoOS   только указанные репозитории (можно несколько)
    ./scripts/leto-check.sh --no-tests      собрать без ctest

LETO_PRESET=<имя> переопределяет пресет для всех репозиториев.
TXT
}

LETO_PATH="${LETO_PATH:-$HOME/leto/}"
ORDER=(LetoAPI LetoCore LetoOS leto-console)
MODE=changed
RUN_TESTS=1
EXPLICIT=()

while [ $# -gt 0 ]; do
    case "$1" in
        --full) MODE=full; shift ;;
        --no-tests) RUN_TESTS=0; shift ;;
        --repo) EXPLICIT+=("$2"); shift 2 ;;
        -h|--help) usage; exit 0 ;;
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

preset_for() {   # $1 = каталог репозитория
    if [ -n "${LETO_PRESET:-}" ]; then
        printf '%s' "$LETO_PRESET"
        return 0
    fi
    ( cd "$1" && cmake --list-presets 2>/dev/null ) \
        | sed -n 's/^ *"\([^"]*\)".*/\1/p' | grep -m1 '^linux'
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
        for i in "${!ORDER[@]}"; do
            [ "${ORDER[$i]}" = "$repo" ] && [ "$i" -lt "$first" ] && first=$i
        done
    done
    if [ "$first" -eq $((${#ORDER[@]} - 1)) ]; then
        [ -n "$(git -C "$LETO_PATH/${ORDER[-1]}" status --porcelain 2>/dev/null)" ] \
            || { echo "OK nothing to check (clean tree)"; exit 0; }
    fi
    SELECTED=("${ORDER[@]:$first}")
fi

started=$(date +%s)
for repo in "${ORDER[@]}"; do
    printf '%s\n' "${SELECTED[@]}" | grep -qx "$repo" || continue
    dir="$LETO_PATH/$repo"
    [ -d "$dir" ] || die "$repo: нет клона в $LETO_PATH"

    preset=$(preset_for "$dir")
    [ -n "$preset" ] || die "$repo: нет linux-пресета (cmake --list-presets)"
    build="$dir/build/$preset"

    if [ ! -d "$build" ]; then
        echo "[$repo] configure $preset"
        run_logged "$repo" bash -c "cd '$dir' && cmake --preset '$preset'"
    fi

    echo "[$repo] build $preset"
    run_logged "$repo" cmake --build "$build" -j

    if [ "$RUN_TESTS" = 1 ] && [ -f "$build/CTestTestfile.cmake" ]; then
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

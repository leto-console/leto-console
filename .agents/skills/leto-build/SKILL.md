---
name: leto-build
description: Собрать LETO — всю цепочку LetoAPI → LetoCore → LetoOS → leto-console или один репозиторий нужным пресетом. Использовать при сборке, при ошибке конфигурации/линковки, при добавлении зависимости, при вопросах про пресеты, LETO_PATH, SDL2, toolchain.
---

# Сборка

Порядок обязателен: `LetoAPI` → `LetoCore` → `LetoOS` → `leto-console`.
Каждый следующий шаг собирается против артефактов предыдущего, поэтому
последовательность задаётся скриптом, а не набором команд в этом файле.

## Локальная сборка

Порядок цепочки и имена пресетов держать в голове не нужно — их знает
`scripts/leto-check.sh`:

```sh
export LETO_PATH="$HOME/leto/"
./scripts/leto-check.sh --full    # вся цепочка, конфигурация при первом запуске
```

Один репозиторий: пресет берётся из `cmake --list-presets` (linux-пресет
в каждом ровно один).

```sh
cd "$LETO_PATH/LetoCore"
cmake --list-presets                      # linux-пресет в репозитории один
cmake --preset <linux-пресет>
```

Зависимости: `cmake`, `ninja-build`, `libsdl2-dev`.
`LETO_PATH` читается как `$ENV{LETO_PATH}`: задан — берутся локальные include/lib
соседних репозиториев, не задан — зависимости тянутся через `FetchContent`
(так собирает CI, порядок там не нужен).

## Пресеты

Источник правды — `CMakePresets.json` в каждом репозитории, не память и не этот файл.
Список спрашивать у CMake, а не читать манифест самому:

```sh
for repo in LetoAPI LetoCore LetoOS leto-console; do
    echo "== $repo"; ( cd "$LETO_PATH/$repo" && cmake --list-presets )
done
```

Ориентиры: прошивка — `stm32f401xc-*`/`stm32f411xe-*`, ПК — `win-*`, `linux-*`,
`macos-*` (macOS есть в `LetoCore`, в `LetoAPI`/`LetoOS` пресетов нет).

## Инкрементально

Уже сконфигурированный репозиторий пересобирается без `--preset`:

```sh
cmake --build build/linux-debug -j
```

`--preset` вызывает переконфигурацию: для одной правки это лишние секунды и шум.

## Проверки после сборки

Не считать сборку доказательством поведения: `leto-check`, `leto-run-game`.

---
name: leto-build
description: Собрать LETO — всю цепочку LetoAPI → LetoCore → LetoOS → leto-console или один репозиторий нужным пресетом. Использовать при сборке, при ошибке конфигурации/линковки, при добавлении зависимости, при вопросах про пресеты, LETO_PATH, SDL2, toolchain.
---

# Сборка

Порядок обязателен: `LetoAPI` → `LetoCore` → `LetoOS` → `leto-console`.
Каждый следующий шаг собирается против артефактов предыдущего.

## Локальные пресеты (Linux)

```sh
export LETO_PATH="$HOME/leto/"
cd "$LETO_PATH/LetoAPI"      && cmake --preset linux-debug
cd "$LETO_PATH/LetoCore"     && cmake --preset linux-debug
cd "$LETO_PATH/LetoOS"       && cmake --preset linux-debug
cd "$LETO_PATH/leto-console" && cmake --preset linux-st7735-debug
```

Зависимости: `cmake`, `ninja-build`, `libsdl2-dev`.
`LETO_PATH` читается как `$ENV{LETO_PATH}`: задан — берутся локальные include/lib
соседних репозиториев, не задан — зависимости тянутся через `FetchContent`
(так собирает CI, порядок там не нужен).

## Пресеты

Источник правды — `CMakePresets.json` в каждом репозитории, не память и не этот файл.
Перечислить актуальные:

```sh
python3 -c "import json,sys;d=json.load(open('CMakePresets.json'));print([p['name'] for p in d['configurePresets'] if not p.get('hidden')])"
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

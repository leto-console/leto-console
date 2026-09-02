<p align="center">
  <a href="./README.md" title="English version">English</a> •
  <b>Русский</b>
</p>

# Развёртывание Leto SDK

Всё, что нужно для сборки SDK из исходников: инструментарий, расположение каталогов, два
bootstrap-скрипта и что делать, когда конфигурация отказывается проходить.

> [!TIP]
> Инструментарий из [шага 1](#шаг-1--устанавливаем-инструменты) уже стоит? Тогда на Windows развёртывание —
> две команды, между которыми нужно перезапустить терминал: [шаг 2](#шаг-2--клонируем-и-собираем).

## Что нужно знать заранее

SDK состоит из трёх репозиториев — [`LetoAPI`](https://github.com/leto-console/LetoAPI) (контракты API),
[`LetoCore`](https://github.com/leto-console/LetoCore) (драйверы и сервисы) и
[`leto-console`](https://github.com/leto-console/leto-console) (сама консоль), — которые рассчитаны на
соседство в одной папке; bootstrap-скрипт из шага 2 склонирует все три сразу. Пресеты общие для всех трёх
проектов, и артефакты раскладываются по одному общему дереву вывода, поэтому имя пресета всегда означает
одну и ту же папку — в любом репозитории.

## Шаг 1 — Устанавливаем инструменты

| Инструмент | Для чего | Скачать | Проверка |
| --- | --- | --- | --- |
| Visual Studio Build Tools | эмулятор | [visualstudio.microsoft.com/ru/downloads](https://visualstudio.microsoft.com/ru/downloads/) | `cl` внутри *x64 Native Tools* |
| GNU Arm Embedded Toolchain | прошивка | [загрузки Arm GNU toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads) | `arm-none-eabi-gcc --version` |
| CMake ≥ 3.20 | всё остальное | [cmake.org/download](https://cmake.org/download/) | `cmake --version` |
| Ninja | всё остальное | [релизы ninja-build](https://github.com/ninja-build/ninja/releases) | `ninja --version` |
| Git | всё остальное | [git-scm.com](https://git-scm.com/downloads) | `git --version` |

Два момента, на которых спотыкаются чаще всего:

- На Windows установщик GNU Arm toolchain **не** добавляет свою папку `bin` в `PATH` — это делается
  вручную (нужна та, где лежит `arm-none-eabi-gcc.exe`).
- MSVC виден только из *Developer*-приглашения. Все команды ниже рассчитаны на
  **x64 Native Tools Command Prompt for VS**; в обычном `cmd` конфигурация пресета `win-*` упадёт.

## Шаг 2 — Клонируем и собираем

**1. Создайте папку под SDK** — она станет `LETO_PATH`. Путь может быть любым, дальше везде
фигурирует `C:\LETO`.

**2. Скопируйте в неё два скрипта из [`guide/deploy/`](.) :**

```
C:\LETO\clone_env.bat
C:\LETO\setup.bat
```

**3. Запустите бустрап из x64 Native Tools приглашения:**

```bat
cd /d C:\LETO
clone_env.bat
```

`clone_env.bat` клонирует три репозитория рядом, создаёт папки вывода `Common\` и `Apps\` и
прописывает `LETO_PATH` через `setx`.

**4. Закройте приглашение и откройте новое.** `setx` не трогает уже запущенные оболочки, а CMake
читает `LETO_PATH` в момент конфигурации — именно этот шаг чаще всего пропускают.

**5. Соберите всё:**

```bat
setup.bat
```

`scripts/setup.bat` каждого репозитория конфигурирует, собирает и устанавливает свои пресеты:
библиотеки уходят в `%LETO_PATH%Common\<preset>`, консоль — в `%LETO_PATH%Console\<preset>`.
Компилируются все пресеты всех трёх репозиториев, поэтому на первой сборке закладывайте десятки минут,
а не несколько; упавший шаг печатает имя пресета, поэтому смотреть нужно на строку выше последней
ошибки, а не на неё саму.

## Шаг 3 — Выставляем библиотеки в PATH

Хостовая сборка линкует `LetoAPI` и `LetoCore` как **shared**-библиотеки, поэтому эмулятору при
запуске нужна их папка в `PATH`:

```
%LETO_PATH%Common\win-debug\bin
```

Добавьте её через *Система → Переменные среды* (достаточно пользовательского `PATH`) и снова откройте
терминал, чтобы изменение подхватилось.

## Шаг 4 — Проверяем

```bat
"%LETO_PATH%Console\win-st7735-debug\bin\leto-console.exe"
```

Окно с интерфейсом консоли означает, что SDK собран здоровым. Два запущенных экземпляра — один
обычный, второй с `--client` — это штатный способ разрабатывать многопользовательские фичи.

## Что получается в итоге

```
C:\LETO\                      <- LETO_PATH
├── LetoAPI\                  <- склонирован
├── LetoCore\                 <- склонирован
├── leto-console\             <- склонирован
├── Common\                   <- собранные библиотеки, по папке на пресет
│   ├── win-debug\bin|lib     <-   LetoAPI.dll, LetoCore.dll  (должны быть в PATH)
│   └── stm32f411xe-debug\lib <-   static-библиотеки для прошивки
├── Console\                  <- собранная консоль, по папке на пресет
│   └── win-st7735-debug\bin\leto-console.exe
└── Apps\                     <- игры и приложения, по папке на пресет
```

`clone_env.bat` завершает `setx LETO_PATH %~dp0` обратным слэшом, поэтому пути пишутся как
`%LETO_PATH%Common`, а не `%LETO_PATH%\Common`. Обе записи раскрываются в одно место; держитесь того
написания, которое используют скрипты.

С заданной `LETO_PATH` сборка потребляет те библиотеки, которые этот гайд только что собрал. Без неё те же
пресеты подтянут зависимости с GitHub и соберут их статически через `FetchContent` — так ходит CI. Обе
дороги разобраны в [`CONTRIBUTING.md`](../../CONTRIBUTING.md).

## Ручная сборка (любая ОС, без скриптов)

```sh
export LETO_PATH="$HOME/LETO"       # или не задавайте её вовсе — тогда FetchContent
cd "$LETO_PATH/leto-console"

cmake --preset win-st7735-debug             # конфигурация
cmake --build --preset win-st7735-debug -j  # сборка
cmake --install build/win-st7735-debug --prefix "$LETO_PATH/Console/win-st7735-debug" --component app
```

Бинарники попадают в `bin/<preset>/`, библиотеки в `lib/<preset>/`, деревья сборки в `build/<preset>/` —
все три пути закреплены пресетом `base`, так что рядом с исходниками ничего не появляется.
`scripts/preset_setup.bat <репозиторий> <пресет>` оборачивает эти три вызова для одного пресета, а
`scripts/build.sh` ведёт ARM-цепочку на Linux; конвенция `LETO_PATH` описана в
[`scripts/README.md`](../../scripts/README.md).

## Прошивка

STM32-сборка даёт `leto-console.elf`, а рядом `.hex` / `.bin`; прошиваются они в `0x08000000` через
STM32CubeProgrammer, OpenOCD или `st-flash`. Сессии отладки для STM32CubeIDE готовы в `*.launch`-файлах
в корне репозитория.

## Linux и Termux

🚧 Раздел ещё пишется (в [английской версии](README.md) — тоже).

- `ubuntu-debug` — единственный пресет, которому нужны Qt 6 Widgets.
- `termux-*` показывают вывод дисплея в браузере по HTTP и предполагают соседскую раскладку из этого
  гайда, а не `FetchContent`; `termux-win-debug` только конфигурирует проект — он нужен для отладки
  веб-вывода на Windows-машине без телефона.

## Устранение неполадок

| Симптом | Причина / решение |
| --- | --- |
| Предупреждение про `LETO_PATH`, заголовки API не найдены | `setx LETO_PATH "C:/LETO"` (или `export`), затем **перезапустить терминал и IDE**. |
| Эмулятор стартует с ошибкой «не найден DLL» | `%LETO_PATH%Common\win-debug\bin` не в `PATH` — см. шаг 3, после чего открыть терминал заново. |
| `Qt6Config.cmake not found` на `ubuntu-debug` | Поставьте Qt 6 Widgets или воспользуйтесь `termux-*` / `win-*` пресетом. |
| `setup.bat` упал где-то в середине | Перезапустите `scripts/setup.bat` того репозитория, на котором всё встало: он печатает имя пресета. |

<p align="center">
  <sub><a href="../../README_ru.md">← К README проекта</a></sub>
</p>

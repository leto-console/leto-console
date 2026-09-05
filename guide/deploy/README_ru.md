<p align="center">
  <sub><a href="../../README_ru.md">← К README проекта</a></sub>
</p>

<p align="center">
  🌐 <a href="./README.md" title="English version">English</a> •
  <b>Русский</b>
</p>

# Развёртывание Leto SDK на Windows

## Что нужно знать заранее

SDK состоит из трёх репозиториев, которые должны находиться одной папке, — [`LetoAPI`](https://github.com/leto-console/LetoAPI) (контракты API),
[`LetoCore`](https://github.com/leto-console/LetoCore) (драйверы и сервисы) и
[`leto-console`](https://github.com/leto-console/leto-console) (сама консоль). Развёртывание проектов производится в несколько шагов: сначала они клонируются из репозиториев, затем создается переменная окружения с их местоположением, и производится поэтапная сборка каждого репозитория для двух пресетов - `win-debug` и `stm32f411xe-debug`. 

Результаты сборки будут размещены в папках `Common` (библиотеки) и `Console` (исполняемые файлы).

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

**1. Создайте папку под SDK** — она станет `LETO_PATH`. Путь может быть любым, дальше везде фигурирует `C:\LETO`.

**2. Скопируйте в неё два скрипта из [`guide/deploy/`](.) :**

```
C:\LETO\clone_env.bat
C:\LETO\setup.bat
```

**3. Запустите `x64 Native Tools` и выполните в нем команды:**

```bat
cd /d C:\LETO
clone_env.bat
```

**4. Перезапустите терминал (закройте) и откройте снова** 

Это необходимо, чтобы подгрузилась вновь созданная (или обновленная) переменная окружения `LETO_PATH`.

**5. Соберите всё:**

```bat
setup.bat
```

Сборка может занять некоторое время (длительность зависит от вашего компьютера).

## Шаг 3 — Выставляем библиотеки в PATH

Эмулятор игровой консоли для быстроты отладки линкуется к библиотекам `LetoAPI` и `LetoCore` динамически. Для того, чтобы система могла корректно найти собранные `.dll` файлы, необходито в переменную `PATH` добавить путь к бинарным файлам библиотек:

```
%LETO_PATH%Common\win-debug\bin
```

Сделать это можно в настройках через *Система → Переменные среды* (достаточно пользовательского `PATH`).

## Шаг 4 — Проверяем

```bat
"%LETO_PATH%Console\win-st7735-debug\bin\leto-console.exe"
```

Окно с интерфейсом консоли означает, что SDK собран здоровым. Два запущенных экземпляра — один
обычный, второй с `--client` — это штатный способ разрабатывать многопользовательские фичи.

## Linux и Termux

🚧 Раздел ещё пишется (в [английской версии](README.md) — тоже).

- `ubuntu-debug` — единственный пресет, которому нужен Qt6.
- `termux-*` показывают вывод дисплея в браузере по HTTP
- `termux-win-debug` нужен для отладки веб-вывода на Windows без телефона.

<p align="center">
  <sub><a href="../../README_ru.md">← К README проекта</a></sub>
</p>

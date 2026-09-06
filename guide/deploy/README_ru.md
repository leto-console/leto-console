<p align="center">
  <sub><a href="../../README_ru.md">← К README проекта</a></sub>
</p>

<p align="center">
  🌐 <a href="./README.md" title="English version">English</a> •
  <b>Русский</b>
</p>

# Развёртывание Leto SDK на Windows

## Что нужно знать заранее

SDK состоит из трёх репозиториев, которые должны находиться в одной папке: [`LetoAPI`](https://github.com/leto-console/LetoAPI) (контракты API),
[`LetoCore`](https://github.com/leto-console/LetoCore) (драйверы и сервисы) и
[`leto-console`](https://github.com/leto-console/leto-console) (сама консоль). Развёртывание проходит в несколько шагов:
репозитории клонируются, для них создаётся переменная окружения `LETO_PATH` с путём к этой папке, после чего каждый
репозиторий поэтапно собирается под два пресета — `win-debug` и `stm32f411xe-debug` у библиотек, `win-st7735-debug` и
`stm32f411xe-st7735-debug` у консоли.

Результаты сборки помещаются в папки `Common` (библиотеки) и `Console` (исполняемые файлы).

## Шаг 1 — Устанавливаем инструменты

| Инструмент | Скачать | Проверка |
| --- | --- | --- |
| Visual Studio Build Tools | https://visualstudio.microsoft.com/ru/downloads/ | `cl` внутри *x64 Native Tools* |
| GNU Arm Embedded Toolchain | https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads | `arm-none-eabi-gcc --version` |
| CMake | https://cmake.org/download/ | `cmake --version` |
| Ninja | https://github.com/ninja-build/ninja/releases | `ninja --version` |
| Git | https://git-scm.com/downloads | `git --version` |

После установки проверьте две вещи:

- Установщик GNU Arm toolchain по умолчанию **не** добавляет свою папку `bin` в `PATH` — её нужно
  добавить вручную (ту, где лежит `arm-none-eabi-gcc.exe`).
- Инструменты MSVC видны только из `x64 Native Tools Command Prompt for VS` — в обычном `cmd` конфигурация пресета `win-*` упадёт.

## Шаг 2 — Клонируем и собираем

**1. Создайте папку под SDK** — она станет `LETO_PATH`. Путь может быть любым, дальше везде используется `C:\LETO`.

**2. Скопируйте в неё два скрипта из [`guide/deploy/`](.):**

```
C:\LETO\clone_env.bat
C:\LETO\setup.bat
```

**3. Запустите `x64 Native Tools` и выполните в нём команды:**

```bat
cd /d C:\LETO
clone_env.bat
```

**4. Перезапустите терминал: закройте и откройте снова.**

Это нужно, чтобы новая (или обновлённая) переменная окружения `LETO_PATH` применилась.

**5. Соберите всё:**

```bat
setup.bat
```

Сборка может занять некоторое время — всё зависит от вашего компьютера.

## Шаг 3 — Выставляем библиотеки в PATH

Эмулятор консоли динамически линкуется к библиотекам `LetoAPI` и `LetoCore` — так отладка идёт быстрее. Чтобы система
находила собранные `.dll` файлы, добавьте путь к библиотекам LETO в переменную окружения `PATH` (достаточно пользовательского):

```
%LETO_PATH%Common\win-debug\bin
```

Сделать это можно через *Система → Переменные среды*.

## Шаг 4 — Проверяем

Запустите эмулятор:

```bat
"%LETO_PATH%Console\win-st7735-debug\bin\leto-console.exe"
```

Программа должна запуститься без ошибок и показать окно игровой консоли.

## Linux и Termux

🚧 Раздел ещё пишется (в [английской версии](README.md) — тоже).

- `ubuntu-debug` — единственный пресет, которому нужен Qt6.
- `termux-*` показывают вывод дисплея в браузере по HTTP.
- `termux-win-debug` нужен для отладки веб-вывода на Windows без телефона.

<p align="center">
  <sub><a href="../../README_ru.md">← К README проекта</a></sub>
</p>

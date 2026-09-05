<div align="center">

# LETO Console

<img src="assets/leto-cover.jpg" alt="LETO" width="120" align="right" />

**Портативная игровая консоль на STM32**

*Одна кодовая база на C++, которая стартует и на плате STM32F4, *и* на рабочем столе —
те же сцены, то же меню, те же игры.*

[![Hardware](https://img.shields.io/badge/STM32F401%20%C2%B7%20F411-SSD1306%20%C2%B7%20ST7735-orange)](leto-console.ioc)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue?logo=cplusplus&logoColor=white)](CMakeLists.txt)
[![CMake](https://img.shields.io/badge/CMake-3.20%2B-064F8C?logo=cmake&logoColor=white)](CMakePresets.json)
[![License: MIT](https://img.shields.io/badge/License-MIT-green)](LICENSE)
[![CI](https://github.com/leto-console/leto-console/actions/workflows/build.yml/badge.svg)](https://github.com/leto-console/leto-console/actions/workflows/build.yml)

[Быстрый старт](#-быстрый-старт) ·
[Железо](#-железо) ·
[Сборка](#-сборка) ·
[Документация](#-документация) ·
[Участие](#-участие) ·
[Лицензия](#-лицензия)

🌐 <a href="./README.md" title="English version">English</a> • <b>Русский</b>

</div>

---

<div align="center">

## Что это?

</div>

<div align="center">
  <img src="assets/1.jpg" alt="LETO Console" width="450px"/><br/>
  <sub>Внешний вид отладочного макета игровой консоли</sub>
</div>
<br>

**LETO** — это любительский проект портативной игровой консоли, который вдохновлен:
- *ретро-эстетикой* второй половины прошлого века (иногда и первой)
- *духом энтузиастов*, которые в это время создавали нечто действительно гениальное
- желанием *глубже понимать язык `C++`*
- экзистенциальной потребностью *прикоснуться к созданию* чего-либо

Проект развивается по принципам открытого и свободного ПО: его можно изучать, запускать, модифицировать, ломать, чинить и т.д. 

<div align="center">

## Возможности

<div align="center">
  <img src="assets/6_1.jpg" alt="LETO Console" width="450px"/><br/>
  <sub>Запущенная с SD-карты игра "Морской бой"</sub>
</div>
<br>

</div>

- **Системная оболочка** — аккаунты с аватарами, главное меню, игровой центр,
  файловый менеджер, системные / отладочные сцены и страницы настроек.
- **Игры как внешние модули** — приложения подгружаются во время исполнения на лету через ABI и версионируемое API из `LetoAPI`.
- **Беспроводной мультиплеер** —  радиомодуль с настраиваемыми каналами,
а также реализованная высокоуровневая маршрутизация данных.
- **Сборка CI** — GitHub Actions компилирует ПО как для целевой платформы STM32, так и
для отладки под Windows, и выкладывает готовые к использованию артефакты.

<div align="center">

## 🚀 Быстрый старт 🚀

</div>

Первую игру для LETO Console можно написать и протестировать прямо на компьютере, без необходимости иметь физическое устройство.

> На текущий момент реализована среда разработки для Windows, включающая эмулятор консоли. Поддержка других платформ планируется в будущих версиях.

Инструкция по настройке Leto SDK и запуску проекта описана в [руководстве по развёртыванию](guide/deploy/README_ru.md). 

<div align="center">
  <img src="assets/win-debug.png" alt="LETO Console"/><br/>
  <sub>Процесс отладки кода в VS Code — <i>breakpoint на открытии сцены</i></sub>
</div>
<br>

<div align="center">
  <img src="assets/win-debug2.png" alt="LETO Console"/><br/>
  <sub>Процесс отладки кода в VS Code — <i>breakpoint на выборе элемента меню</i></sub>
</div>
<br>

<div align="center">
  <img src="assets/win-debug3.png" alt="LETO Console"/><br/>
  <sub>Процесс отладки кода в VS Code — <i>Step Into в функцию из библиотеки LetoCore</i></sub>
</div>
<br>


<div align="center">

## Железо

</div>

</div>

🚧 Схема платы будет добавлена позже. 

Игровая консоль состоит из следующих компонентов:

| | |
| --- | --- |
| **MCU** | • `STM32F411CE` (512 КБ flash / 128 КБ RAM) <br> • `STM32F401CC` (256 КБ / 64 КБ) - урезанная версия |
| **Дисплей** | • `ST7735` 160×128 цветной по SPI <br> • `SSD1306` 128×64 монохромный по I²C  |
| **Ввод** | • 4 кнопки направления  <br> • 2 кнопки действия (A и B) <br> • кнопка "Меню" <br> • энкодер |
| **Память** | • 32 КБ EEPROM по I²C (настройки, аккаунты, сохранения) <br> • micro-SD по SPI через FatFs |
| **Радио** | • `nRF24L01` по SPI |
| **Прочее** | • отладочная консоль по UART <br> • UART канал между консолями <br> • RTC от LSE <br> • аппаратный CRC, TIM, DMA |

<div align="center">

## Сборка

</div>

Сборка ПО под различные конфигурации реализуется через **пресеты CMake**.

Для 
```

```

Для проекта `leto-console` доступны следующие пресеты:

| Семейство пресетов | Цель | Примечания |
| --- | --- | --- |
| `win-ssd1306-debug` · `win-st7735-debug` · `win-st7735-release` | эмулятор на Windows (MSVC) | `win-st7735-debug` — рабочий дефолт, все три собираются в CI |
| `stm32f401xc-ssd1306-debug` · `stm32f401xc-st7735-debug` · `stm32f411xe-st7735-debug` | прошивка STM32F4 | кросс-компиляция `gcc-arm-none-eabi`, последние два собираются в CI |
| `ubuntu-debug` | эмулятор на Linux (GCC) | единственный пресет, которому нужны Qt6 Widgets |
| `termux-debug` · `termux-win-debug` | вывод по HTTP | отдают интерфейс консоли в браузер; второй только конфигурирует — для проверки без телефона |

[`scripts/`](scripts/) оборачивает те же вызовы: `setup.bat` собирает два поставляемых пресета и ставит
их в `%LETO_PATH%Console/`, `preset_setup.bat <repo> <preset>` делает то же для одного, `build.sh`
ведёт ARM-цепочку на Linux.

### Запуск эмулятора

```bat
leto-console.exe
:: обычный запуск — это «серверный» узел: server.eeprom / server.img
leto-console.exe --client --user 0 --game
:: клиентский узел со своими образами памяти, авто-входом и авто-запуском
```

| Флаг | Действие |
| --- | --- |
| `--client` / `-c` | клиентский узел: использует `client.eeprom` / `client.img` вместо файлов `server.*` |
| `--user <n>` | автоматически аутентифицировать аккаунт *n* при загрузке (по умолчанию `0`) |
| `--game <name>` | сразу загрузить и запустить указанное приложение после входа |

### Артефакты

[`build.yml`](.github/workflows/build.yml) запускается на каждый push и pull request в `main`
(`stm32f401xc-st7735-debug`, `stm32f411xe-st7735-debug` на Ubuntu, `win-st7735-debug`,
`win-st7735-release` на Windows). Каждая задача выгружает `dist/<preset>/` в артефакт
`leto-console-<preset>` — бери готовую сборку там вместо локальной, а про прошивку на плату читай
[руководство по развёртыванию](guide/deploy/README_ru.md).

## 📚 Документация

| Документ | О чём |
| --- | --- |
| [`guide/deploy/README.md`](guide/deploy/README.md) <br> [`guide/deploy/README_ru.md`](guide/deploy/README_ru.md) | развёртывание, сборка, прошивка, устранение неполадок (EN / RU) |
| [`CONTRIBUTING.md`](CONTRIBUTING.md) | конвенции веток и коммитов, правила дома, как сборка находит `LetoAPI` / `LetoCore`, рецепты сцен и настроек |
| [`scripts/README.md`](scripts/README.md) | локальные скрипты сборки и конвенция `LETO_PATH` (RU) |
| [`LetoAPI`](https://github.com/leto-console/LetoAPI) · [`LetoCore`](https://github.com/leto-console/LetoCore) | две остальные половины SDK |

## 🗺️ Цели

До 1.0 и в движении: `main` — рабочая ветка, релизы отмечены вплоть до `v0.0.5`, обсуждения дизайна идут
в [трекере задач](https://github.com/leto-console/leto-console/issues). Текущий фокус — половина
руководства про Linux/Termux, протокол мультиплеера nRF24L01 и растущий каталог игр, собранных под ABI
`LetoAPI` v1.

## 🤝 Участие

Форкни, ответвись от `main` в ветку `NN-short-slug` (номер задачи), держи префиксы коммитов
(`feature:`, `fix:`, `docs:`) и заставь матрицу CI пройти локально до открытия pull request. В
[`CONTRIBUTING.md`](CONTRIBUTING.md) — конвенции, два пути разрешения зависимостей и рецепты для новой
сцены или постоянного параметра. Хорошие первые задачи помечены лейблами в трекере.

## 📄 Лицензия

Опубликовано под **MIT License** — см. [`LICENSE`](LICENSE). Vendored-исходники STMicroelectronics
CMSIS и HAL в `Drivers/` остаются под собственными условиями лицензии STM32 (`Drivers/*/LICENSE.txt`).

<div align="center">

Собрано сообществом LETO · *железо, которое можно подержать в руках, код, который можно прочитать.*

</div>
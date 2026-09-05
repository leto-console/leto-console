<div align="center">

# LETO Console

<img src="assets/leto-cover.jpg" alt="LETO" width="120" align="right" />

**A portable game console built on STM32**

[![Hardware](https://img.shields.io/badge/STM32F411%20%C2%B7%20F401-ST7735%20%C2%B7%20SSD1306-orange)](leto-console.ioc)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue?logo=cplusplus&logoColor=white)](CMakeLists.txt)
[![CMake](https://img.shields.io/badge/CMake-3.20%2B-064F8C?logo=cmake&logoColor=white)](CMakePresets.json)
[![License: MIT](https://img.shields.io/badge/License-MIT-green)](LICENSE)
[![CI](https://github.com/leto-console/leto-console/actions/workflows/build.yml/badge.svg)](https://github.com/leto-console/leto-console/actions/workflows/build.yml)

[Quick start](#-1-quick-start-) ·
[Hardware](#2-hardware) ·
[Building](#3-building) ·
[Goals](#4-goals) ·
[Documentation](#5-documentation) ·
[Contributing](#6-contributing) ·
[License](#7-license)

🌐 <b>English</b> • <a href="./README_ru.md" title="Версия на русском">Русский</a>

</div>

---

<div align="center">

## What is this?

</div>

<div align="center">
  <img src="assets/1.jpg" alt="LETO Console" width="450px"/><br/>
  <sub>The way the debug prototype of the console looks</sub>
</div>
<br>

**LETO** is a hobby project of a portable game console, inspired by:
- the *retro aesthetics* of the second half of the last century (sometimes of the first one as well)
- the *spirit of the enthusiasts* who, at that time, were creating genuinely brilliant things
- the wish *to understand the `C++` language more deeply*
- an existential need *to take part in creating* something

The project grows following the principles of free and open source software: it can be studied, run,
modified, broken, fixed, and so on.

<div align="center">

## Features

<div align="center">
  <img src="assets/6_1.jpg" alt="LETO Console" width="450px"/><br/>
  <sub>"Battleship", launched from an SD card</sub>
</div>
<br>

</div>

- **System shell** — accounts with avatars, a main menu, a game center, a file manager,
  system / debug scenes and settings pages.
- **Games as external modules** — applications are loaded on the fly at runtime, through the ABI
  and the versioned API of `LetoAPI`.
- **Wireless multiplayer** — a radio module with configurable channels, plus a ready high-level
  data routing implementation.
- **CI builds** — GitHub Actions compiles the software both for the STM32 target platform and for
  debugging under Windows, and publishes ready-to-use artifacts.

<div align="right">

## ⚡ 1. Quick start ⚡

</div>

Your first game for the LETO Console can be written and tested right on a computer, without the need to
own a physical device.

> At the moment the development environment is available for Windows only, and it includes the console
> emulator. Support for other platforms is planned for future versions.

Setting up the Leto SDK and running the project is described in the [setup guide](guide/deploy/README.md).

<div align="center">
  <img src="assets/win-debug.png" alt="LETO Console"/><br/>
  <sub>Debugging the code in VS Code — <i>a breakpoint on opening a scene</i></sub>
</div>
<br>

<div align="center">
  <img src="assets/win-debug2.png" alt="LETO Console"/><br/>
  <sub>Debugging the code in VS Code — <i>a breakpoint on selecting a menu item</i></sub>
</div>
<br>

<div align="center">
  <img src="assets/win-debug3.png" alt="LETO Console"/><br/>
  <sub>Debugging the code in VS Code — <i>stepping into a function of the LetoCore library</i></sub>
</div>
<br>


<div align="right">

## 2. Hardware

</div>

🚧 The board schematic will be added later.

The game console consists of the following components:

| | |
| --- | --- |
| **MCU** | • `STM32F411CE` (512 KB flash / 128 KB RAM) <br> • `STM32F401CC` (256 KB / 64 KB) — a cut-down version |
| **Display** | • `ST7735` 160×128 color over SPI <br> • `SSD1306` 128×64 monochrome over I²C |
| **Input** | • 4 direction buttons <br> • 2 action buttons (A and B) <br> • a "Menu" button <br> • an encoder |
| **Memory** | • 32 KB EEPROM over I²C (settings, accounts, saves) <br> • micro-SD over SPI through FatFs |
| **Radio** | • `nRF24L01` over SPI |
| **Other** | • a debug console over UART <br> • a UART channel between the consoles <br> • RTC fed from LSE <br> • hardware CRC, TIM, DMA |

<div align="right">

## 3. Building

</div>

Building the software for the various configurations is done through **CMake presets**:

```
cmake --preset win-st7735-debug
cmake --build --preset win-st7735-debug -j
```

The `leto-console` project provides the following presets:

| Preset family | Purpose |
| --- | --- |
| • `win-st7735-debug` <br>• `win-st7735-release` <br>• `win-ssd1306-debug` | the emulator on Windows (MSVC) |
| • `stm32f411xe-st7735-debug` <br>• `stm32f401xc-ssd1306-debug` <br>• `stm32f401xc-st7735-debug` | STM32F4 firmware |
| • `ubuntu-debug` | the emulator on Linux (GCC) |
| • `termux-debug` <br>• `termux-win-debug` | the emulator over HTTP |

### Running the emulator

```bat
leto-console.exe
:: a plain run
leto-console.exe --user 0 --game %LETO_PATH%Apps\win-debug\Battleship\Battleship.dll
:: a run with auto-login and auto-launch of an application
```

`LETO_PATH` is provided by the SDK setup and already ends with a separator, which is why the path above has
no extra backslash after it.

| Flag | Effect |
| --- | --- |
| `--client` / `-c` | a client node: uses the `client.eeprom` / `client.img` files instead of `server.*` |
| `--user <n>` | automatically authenticate account *n* at start-up (default `0`) |
| `--game <path>` | load and start the given application right after login (handled inside the `--user` branch, so `--user` has to be present as well) |

<div align="right">

## 4. Goals

</div>

Before version 1.0 it is planned to:

- Make it fully possible to develop applications on `Linux`
- Add an API for working with fonts on the application side
- Set up a mechanism for adding new fonts to the system
- Move to `C++20`
- Add a `Dirty rectangles` rendering algorithm
- Add a pleasant retro-style system UI
- Design a new power system for the device, based on a rechargeable battery
- Extend the game collection

<div align="right">

## 5. Documentation

</div>

🚧 This section is in progress. A detailed description of the system will be available later.

<div align="right">

## 6. Contributing

</div>

To contribute a change you have to:
- create an issue describing the change (or pick up an existing one)
- branch off `main` into a `NN-task-name` branch (the issue number and a title)
- commit the changes with a `feature:`, `fix:`, `docs:` or other prefix
- open a pull request and make sure that all CI operations pass
- go through code review and, after the approval, merge the changes into the main branch

🚧 Detailed contribution conventions will be described later in `CONTRIBUTING.md`.

<div align="right">

## 7. License

</div>

Published under the **MIT License** — see [`LICENSE`](LICENSE). The vendored STMicroelectronics CMSIS and
HAL sources in `Drivers/` stay under their own STM32 license terms (`Drivers/*/LICENSE.txt`).

<div align="center">
<br>

<i>Created by the LETO community</i>

</div>

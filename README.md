<div align="center">

# LETO Console
<img src="assets/leto-cover.jpg" alt="LETO" width="120" align="right" />

**A compact hobbyist game console built on the STM32.**

One C++ codebase that boots on an STM32F4 board *and* on your desktop —
the same scenes, the same menus, the same games.

[![CI](https://github.com/leto-console/leto-console/actions/workflows/build.yml/badge.svg)](https://github.com/leto-console/leto-console/actions/workflows/build.yml)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue?logo=cplusplus&logoColor=white)](CMakeLists.txt)
[![CMake](https://img.shields.io/badge/CMake-3.20%2B-064F8C?logo=cmake&logoColor=white)](CMakePresets.json)
[![Hardware](https://img.shields.io/badge/STM32F401%20%C2%B7%20F411-SSD1306%20%C2%B7%20ST7735-orange)](leto-console.ioc)
[![License: MIT](https://img.shields.io/badge/License-MIT-green)](LICENSE)

[Quick start](#-quick-start) ·
[Hardware](#-hardware) ·
[Building](#-building) ·
[Documentation](#-documentation) ·
[Contributing](#-contributing) ·
[License](#-license)

🌐 <b>EN</b> • <a href="guide/deploy/README_ru.md" title="Setup guide in Russian">Русский</a>

</div>

---

## What is this?

**LETO** is a hobby project in the "retro handheld + SDK" genre. This repository is the **system layer
of the console**: it boots the device, draws the UI, manages user accounts and saved data, launches
games and exposes the system API to them. It is one of three repositories — `LetoAPI` holds the public
contracts, `LetoCore` the drivers and services, `leto-console` the product itself.

The trick that makes development possible is that the exact same logic is compiled twice:

| | **Firmware** | **Emulator** |
| --- | --- | --- |
| Entry point | `Core/Src/main.cpp` | `Core/Main/App.cpp` |
| Screen | • SSD1306 128×64 mono <br>• ST7735 160×128 color | • native Win32 window <br>• Qt6 window <br>• browser over HTTP |
| Why | the real handheld | development, CI, demos, extra console nodes |

Prototype a scene on the laptop in seconds, flash the board, see it byte-for-byte identical — no hardware required to start contributing.

## ✨ Highlights

- 🎮 **Full console shell** — welcome screen, avatar-based accounts, main menu, game center, file
  manager, system / debug / EEPROM scenes and settings pages.
- 🧩 **Games as external modules** — apps are loaded at runtime through a versioned ABI from `LetoAPI`,
  so a game survives a firmware update.
- 💾 **Persistent typed settings** — declarative `StoredDataCell<T>` values in EEPROM-backed storage,
  applied on the fly.
- 📡 **Wireless multiplayer** — nRF24L01 2.4 GHz radio with configurable pipes and channel; two emulator
  instances on one laptop already talk to each other.
- 🧊 **No STL, no exceptions on target** — `-fno-exceptions -fno-rtti` on ARM, fixed-size containers
  (`StaticList`, `StaticText`, `BitmapView`) from `LetoCore`.
- 🤖 **CI builds both halves** — a GitHub Actions matrix compiles firmware *and* Windows binaries and
  uploads ready-to-flash artifacts.

<div align="center">
  <img src="assets/leto-games.jpg" alt="LETO Games" width="232" /><br/>
  <sub>Games are compiled as external modules against the <code>LetoAPI</code> ABI.</sub>
</div>

## 🔌 Hardware

The board is described by [`leto-console.ioc`](leto-console.ioc); `Core/Src/main.cpp` hands the
peripheral handles to `Application::Periphery`. Out of the box the console drives:

| | |
| --- | --- |
| **MCU** | STM32F401CC (256 KB flash / 64 KB RAM) or STM32F411CE (512 KB / 128 KB), Cortex-M4 with hard float |
| **Display** | SSD1306 128×64 mono over I²C *or* ST7735 160×128 color over SPI1, chosen by a preset |
| **Input** | 7 buttons plus a rotary encoder, abstracted by `UserInputDevice` from `LetoCore` |
| **Storage** | 32 KB EEPROM on I²C1 (settings, accounts, saves) and micro-SD on SPI1 through FatFs |
| **Radio** | nRF24L01 on SPI3 for console-to-console play |
| **Comms** | USART2 debug console @19200, USART6 web bridge, LSE-driven RTC, hardware CRC, TIM1, DMA, status LED |

## 🚀 Quick start

Install [VS Build Tools](https://visualstudio.microsoft.com/downloads/), the
[Arm GNU toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads), CMake, Ninja
and Git, then copy [`clone_env.bat`](guide/deploy/clone_env.bat) and [`setup.bat`](guide/deploy/setup.bat)
into an empty folder — that folder becomes `LETO_PATH`. From an **x64 Native Tools Command Prompt**:

```bat
cd /d C:\LETO
:: clones LetoAPI, LetoCore and leto-console, creates Common\ and Apps\, registers LETO_PATH
clone_env.bat
:: close the prompt, open a fresh x64 Native Tools one, then:
setup.bat
:: builds and installs every preset of all three repositories
"%LETO_PATH%Console\win-st7735-debug\bin\leto-console.exe"
```

A window showing the console UI means the SDK is healthy. The
[setup guide](guide/deploy/README.md) covers the manual route, `PATH` for the shared libraries,
flashing the firmware, Linux/Termux and troubleshooting — and there is a
[Russian version](guide/deploy/README_ru.md).

## 🏗️ Building

Everything goes through **CMake presets** — no manual `-D` flags:

```sh
cmake --preset win-st7735-debug        # configure
cmake --build --preset win-st7735-debug -j
```

The `base` preset pins the output layout: binaries in `bin/<preset>/`, libraries in `lib/<preset>/`,
build trees in `build/<preset>/`.

| Preset family | Target | Notes |
| --- | --- | --- |
| `win-ssd1306-debug` · `win-st7735-debug` · `win-st7735-release` | Windows emulator (MSVC) | `win-st7735-debug` is the default dev target, all three built in CI |
| `stm32f401xc-ssd1306-debug` · `stm32f401xc-st7735-debug` · `stm32f411xe-st7735-debug` | STM32F4 firmware | cross-compiled with `gcc-arm-none-eabi`, the last two built in CI |
| `ubuntu-debug` | Linux emulator (GCC) | the only preset that needs Qt6 Widgets |
| `termux-debug` · `termux-win-debug` | HTTP display | serve the console UI to a browser; the second is configure-only, for testing without a phone |

[`scripts/`](scripts/) wraps the same calls: `setup.bat` builds the two shipping presets and installs
them under `%LETO_PATH%Console/`, `preset_setup.bat <repo> <preset>` handles one, `build.sh` drives the
ARM chain on Linux.

### Running the emulator

```bat
leto-console.exe
:: a plain run acts as the "server" node — server.eeprom / server.img
leto-console.exe --client --user 0 --game
:: a client node with its own storage images, auto-login and auto-launch
```

| Flag | Effect |
| --- | --- |
| `--client` / `-c` | client node: uses `client.eeprom` / `client.img` instead of the `server.*` files |
| `--user <n>` | auto-authenticate account *n* at boot (default `0`) |
| `--game <name>` | auto-load and start the given app right after login |

### Artifacts

[`build.yml`](.github/workflows/build.yml) runs on every push and pull request to `main`
(`stm32f401xc-st7735-debug`, `stm32f411xe-st7735-debug` on Ubuntu, `win-st7735-debug`,
`win-st7735-release` on Windows). Each job uploads `dist/<preset>/` as the artifact
`leto-console-<preset>` — grab a build there instead of compiling locally, and see the
[setup guide](guide/deploy/README.md) for flashing it onto a board.

## 📚 Documentation

| Document | Contents |
| --- | --- |
| [`guide/deploy/README.md`](guide/deploy/README.md) <br> [`guide/deploy/README_ru.md`](guide/deploy/README_ru.md) | setup, build, flash, troubleshooting (EN / RU) |
| [`CONTRIBUTING.md`](CONTRIBUTING.md) | branch and commit conventions, house rules, how the build resolves `LetoAPI` / `LetoCore`, recipes for scenes and settings |
| [`scripts/README.md`](scripts/README.md) | the local build scripts and the `LETO_PATH` convention (RU) |
| [`LetoAPI`](https://github.com/leto-console/LetoAPI) · [`LetoCore`](https://github.com/leto-console/LetoCore) | the other two halves of the SDK |

## 🗺️ Status

Pre-1.0 and moving: `main` is the working branch, releases are tagged up to `v0.0.5`, and design
discussions happen in the [issue tracker](https://github.com/leto-console/leto-console/issues). Current
focus — the Linux/Termux half of the setup guide, the nRF24L01 multiplayer protocol and a growing game
catalog built against the `LetoAPI` v1 ABI.

## 🤝 Contributing

Fork, branch from `main` as `NN-short-slug` (the issue number), keep the commit prefixes
(`feature:`, `fix:`, `docs:`) and make the CI matrix pass locally before opening a pull request.
[`CONTRIBUTING.md`](CONTRIBUTING.md) has the conventions, the two dependency-resolution paths and the
recipes for adding a scene or a persistent setting. Good first issues are labeled in the tracker.

## 📄 License

Released under the **MIT License** — see [`LICENSE`](LICENSE). Vendored STMicroelectronics CMSIS and HAL
sources under `Drivers/` stay governed by their own STM32 license terms (`Drivers/*/LICENSE.txt`).

<div align="center">

Built by the LETO community · *hardware you can hold, code you can read.*

</div>

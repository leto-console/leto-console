<p align="center">
  <b>English</b> •
  <a href="./README_ru.md" title="Версия на русском">Русский</a>
</p>

# Setting up the LETO SDK

Everything needed to build the SDK from source: the toolchain, the folder layout, the two bootstrap
scripts and what to do when something refuses to configure.

> [!TIP]
> Tools from [step 1](#step-1--install-the-tools) already installed? Then on Windows the rest is two
> commands, with one terminal restart in between — [jump to step 2](#step-2--clone-and-build).

## What you need to know first

The SDK is three repositories — [`LetoAPI`](https://github.com/leto-console/LetoAPI) (the API contracts),
[`LetoCore`](https://github.com/leto-console/LetoCore) (drivers and services) and
[`leto-console`](https://github.com/leto-console/leto-console) (this product) — that expect to sit side by
side in one folder; the bootstrap script in step 2 clones all three for you. Preset names are shared
across the projects, and every artifact lands in one common output tree, so a preset name always means the
same folder in any repository.

## Step 1 — Install the tools

| Tool | Needed for | Download | Verify |
| --- | --- | --- | --- |
| Visual Studio Build Tools | the emulator | [visualstudio.microsoft.com/downloads](https://visualstudio.microsoft.com/downloads/) | `cl` in an *x64 Native Tools* prompt |
| GNU Arm Embedded Toolchain | the firmware | [Arm GNU toolchain downloads](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads) | `arm-none-eabi-gcc --version` |
| CMake ≥ 3.20 | everything | [cmake.org/download](https://cmake.org/download/) | `cmake --version` |
| Ninja | everything | [ninja-build releases](https://github.com/ninja-build/ninja/releases) | `ninja --version` |
| Git | everything | [git-scm.com](https://git-scm.com/downloads) | `git --version` |

Two gotchas worth knowing up front:

- On Windows, add the toolchain's `bin` folder (the one containing `arm-none-eabi-gcc.exe`) to `PATH`
  yourself — the installer does not do it.
- MSVC is only visible inside a *Developer* prompt. Everything below assumes the
  **x64 Native Tools Command Prompt for VS**; a plain `cmd` will fail to configure a `win-*` preset.

## Step 2 — Clone and build

**1. Create a folder for the SDK** — it becomes `LETO_PATH`. Any path works, `C:\LETO` is used here.

**2. Copy the two scripts from [`guide/deploy/`](.) into that folder:**

```
C:\LETO\clone_env.bat
C:\LETO\setup.bat
```

**3. Run the bootstrap from an x64 Native Tools prompt:**

```bat
cd /d C:\LETO
clone_env.bat
```

`clone_env.bat` clones the three repositories side by side, creates the `Common\` and `Apps\` output
folders and registers `LETO_PATH` with `setx`.

**4. Close the prompt and open a fresh one.** `setx` does not touch already-running shells, and CMake
reads `LETO_PATH` at configure time — this is the step people miss.

**5. Build everything:**

```bat
setup.bat
```

Each repository's `scripts/setup.bat` configures, builds and installs its presets: libraries go to
`%LETO_PATH%Common\<preset>`, the console to `%LETO_PATH%Console\<preset>`. Every preset of all three
repositories gets compiled, so budget tens of minutes on a first run rather than a few; a failing step
prints the preset it was building, so scroll up, not down.

## Step 3 — Make the DLLs reachable

The host build links `LetoAPI` and `LetoCore` as **shared** libraries, so the emulator needs their
folder on `PATH` at runtime:

```
%LETO_PATH%Common\win-debug\bin
```

Add it through *System → Environment Variables* (a user `PATH` entry is enough), then open yet another
terminal so the change is picked up.

## Step 4 — Verify

```bat
"%LETO_PATH%Console\win-st7735-debug\bin\leto-console.exe"
```

A window showing the console UI means the SDK is healthy. Running two instances — one started plainly,
one with `--client` — is the usual way to work on multiplayer features.

## The resulting layout

```
C:\LETO\                      <- LETO_PATH
├── LetoAPI\                  <- cloned
├── LetoCore\                 <- cloned
├── leto-console\             <- cloned
├── Common\                   <- built libraries, one folder per preset
│   ├── win-debug\bin|lib     <-   LetoAPI.dll, LetoCore.dll  (must be on PATH)
│   └── stm32f411xe-debug\lib <-   static libraries for the firmware
├── Console\                  <- built console, one folder per preset
│   └── win-st7735-debug\bin\leto-console.exe
└── Apps\                     <- games and app modules, one folder per preset
```

`clone_env.bat` ends `setx LETO_PATH %~dp0` with a backslash, which is why paths are written as
`%LETO_PATH%Common` rather than `%LETO_PATH%\Common`. Both expand to the same place; keep the spelling
the scripts use.

With `LETO_PATH` set the build consumes the shared libraries this guide has just produced. Leave it unset
and the same presets fetch and build their dependencies through `FetchContent` instead — which is what CI
does. [`CONTRIBUTING.md`](../../CONTRIBUTING.md) covers both paths.

## Building by hand (any OS, no scripts)

```sh
export LETO_PATH="$HOME/LETO"       # or leave unset for the FetchContent build
cd "$LETO_PATH/leto-console"

cmake --preset win-st7735-debug             # configure
cmake --build --preset win-st7735-debug -j  # build
cmake --install build/win-st7735-debug --prefix "$LETO_PATH/Console/win-st7735-debug" --component app
```

Binaries land in `bin/<preset>/`, libraries in `lib/<preset>/` and build trees in `build/<preset>/` — the
`base` preset pins all three, so nothing appears next to the sources. `scripts/preset_setup.bat <repo>
<preset>` wraps those three calls for a single preset, `scripts/build.sh` drives the ARM chain on Linux,
and both read the same `LETO_PATH` convention described in [`scripts/README.md`](../../scripts/README.md).

## Flashing the firmware

An STM32 build produces `leto-console.elf` plus `.hex` / `.bin`, flashed to `0x08000000` with
STM32CubeProgrammer, OpenOCD or `st-flash`; the `*.launch` files in the repository root set up the
STM32CubeIDE debug sessions.

## Linux and Termux

🚧 Still in progress (also in the [Russian version](README_ru.md)).

- `ubuntu-debug` is the only preset that requires Qt 6 Widgets.
- `termux-*` presets serve the display to a browser over HTTP and assume the sibling layout of this guide
  rather than the `FetchContent` fallback; `termux-win-debug` only configures, for debugging that display
  on a Windows host without a phone.

## Troubleshooting

| Symptom | Cause / fix |
| --- | --- |
| `LETO_PATH` warning, API headers not found | `setx LETO_PATH "C:/LETO"` (or `export`), then **restart the terminal and the IDE**. |
| Emulator starts with a "missing DLL" error | `%LETO_PATH%Common\win-debug\bin` is not on `PATH` — see step 3, then reopen the terminal. |
| `Qt6Config.cmake not found` on `ubuntu-debug` | Install Qt 6 Widgets, or use a `termux-*` / `win-*` preset. |
| `setup.bat` failed somewhere in the middle | Re-run the failing repository's own `scripts/setup.bat`; it names the preset it was building. |

<p align="center">
  <sub><a href="../../README.md">← Back to the project README</a></sub>
</p>


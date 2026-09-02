<p align="center">
  <b>English</b> •
  <a href="./README_ru.md" title="Версия на русском">Русский</a>
</p>

# Setting up the LETO SDK

Everything needed to build the SDK from source: the toolchain, the folder layout, the two bootstrap
scripts and what to do when something refuses to configure.

> [!TIP]
> In a hurry? On Windows the whole thing is two commands — [jump to step 2](#step-2--clone-and-build).

## What you are building

The SDK lives in three repositories that are designed to sit next to each other:

| Repository | Role | Produces |
| --- | --- | --- |
| [`LetoAPI`](https://github.com/leto-console/LetoAPI) | The public contract: the system API headers every game links against | shared lib on host, static lib on target |
| [`LetoCore`](https://github.com/leto-console/LetoCore) | Implementation: display drivers, input, storage, scene/task managers, app loader, radios | shared lib on host, static lib on target |
| [`leto-console`](https://github.com/leto-console/leto-console) | The console itself: peripheral setup, system scenes, settings, emulator shell | `leto-console.exe` / `leto-console.elf` |

Presets are shared across the three projects — `win-debug` and `stm32f411xe-debug` for the libraries,
`win-st7735-debug` and `stm32f411xe-st7735-debug` for the console. Every artifact is installed into one
common output tree, so a preset name always means the same folder everywhere.

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
`%LETO_PATH%Common\<preset>`, the console to `%LETO_PATH%Console\<preset>`. Expect a few minutes on a
clean machine; a failing step prints the preset it was building, so scroll up, not down.

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

## How dependencies get resolved

`CMakeLists.txt` reads `$ENV{LETO_PATH}` and takes one of two paths:

- **`LETO_PATH` is set** (this guide) — CMake consumes the *already built* artifacts of the sibling
  repositories: headers from their `include/` folders, libraries from
  `%LETO_PATH%Common/<CORE_BUILD_PRESET>/lib`, plus the host-side shared libraries from step 3.
  `CORE_BUILD_PRESET` in `CMakePresets.json` pins which core build folder is consumed, so after
  touching `LetoCore` you rebuild that library preset first.
- **`LETO_PATH` is unset** — `LetoCore` is fetched from GitHub with `FetchContent` and built statically
  from source, and it pulls `LetoAPI` (and `cpp-httplib` for the web display) the same way. This is the
  path CI takes: one clone, no environment, a slower configure step.

Both paths compile the same sources; only where the dependencies come from differs.

## Building by hand (any OS, no scripts)

```sh
export LETO_PATH="$HOME/LETO"       # or leave unset for the FetchContent build
cd "$LETO_PATH/leto-console"

cmake --preset win-st7735-debug             # configure
cmake --build --preset win-st7735-debug -j  # build
```

Binaries land in `bin/<preset>/`, libraries in `lib/<preset>/`, build trees in `build/<preset>/` — the
`base` preset pins all three, so nothing is written next to the sources. To install into the SDK layout
the way `scripts/setup.bat` does:

```sh
cmake --install build/win-st7735-debug --prefix "$LETO_PATH/Console/win-st7735-debug" --component app
```

`scripts/preset_setup.bat <repo> <preset>` wraps those three calls for a single preset, and
`scripts/build.sh` drives the ARM chain on Linux. Both read the same `LETO_PATH` convention described in
[`scripts/README.md`](../../scripts/README.md).

## Flashing the firmware

An STM32 build produces `leto-console.elf` plus `.hex` / `.bin` (generated post-build by `objcopy`,
together with a memory-usage report). Flash them to `0x08000000` with STM32CubeProgrammer, OpenOCD or
`st-flash`. Debug sessions for STM32CubeIDE are provided by the `*.launch` files in the repository root.

> [!NOTE]
> `Drivers/` and the `MX*_Init()` blocks are generated from `leto-console.ioc`. Change the `.ioc` in
> STM32CubeMX and regenerate rather than hand-patching generated files.

## Linux and Termux

🚧 This section is still in progress (also in the [Russian version](README_ru.md)). What is known today:

- `ubuntu-debug` requires Qt 6 Widgets — it is the only preset with that dependency.
- `termux-*` presets serve the display to a browser over HTTP and additionally copy
  `../LetoCore/include/GraphicsPC/web` next to the executable, so they assume the sibling layout above
  rather than the `FetchContent` fallback.
- `termux-win-debug` is a configure-only preset for debugging the web display on a Windows host with no
  phone attached.

## Troubleshooting

| Symptom | Cause / fix |
| --- | --- |
| `LETO_PATH` warning, API headers not found | `setx LETO_PATH "C:/LETO"` (or `export`), then **restart the terminal and the IDE**. |
| Emulator starts with a "missing DLL" error | `%LETO_PATH%Common\win-debug\bin` is not on `PATH` — see step 3, then reopen the terminal. |
| `arm-none-eabi-gcc: command not found` | Add the ARM toolchain's `bin/` to `PATH`. |
| `cl` not found while configuring a `win-*` preset | Run from *x64 Native Tools Command Prompt for VS* (CI uses `ilammy/msvc-dev-cmd`). |
| `Qt6Config.cmake not found` on `ubuntu-debug` | Install Qt 6 Widgets, or use a `termux-*` / `win-*` preset. |
| Links against a stale core | `CORE_BUILD_PRESET` pins which `LetoCore` build folder is consumed — rebuild that preset first. |
| `setup.bat` failed somewhere in the middle | Re-run the failing repository's own `scripts/setup.bat`; it names the preset it was building. |

<p align="center">
  <sub><a href="../../README.md">← Back to the project README</a></sub>
</p>


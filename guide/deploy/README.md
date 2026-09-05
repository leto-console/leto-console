<p align="center">
  <sub><a href="../../README.md">← Back to the project README</a></sub>
</p>

<p align="center">
  🌐 <b>English</b> •
  <a href="./README_ru.md" title="Версия на русском">Русский</a>
</p>

# Setting up the Leto SDK on Windows

## What you need to know first

The SDK consists of three repositories that have to sit in one folder — [`LetoAPI`](https://github.com/leto-console/LetoAPI) (the API contracts),
[`LetoCore`](https://github.com/leto-console/LetoCore) (drivers and services) and
[`leto-console`](https://github.com/leto-console/leto-console) (the console itself). Deploying the projects takes several steps: first they are cloned
from the repositories, then an environment variable holding their location is created, and after that every repository is built step by step for two
presets: `win-debug` and `stm32f411xe-debug` for the libraries, `win-st7735-debug` and `stm32f411xe-st7735-debug` for the console.

The build results are placed into the `Common` (libraries) and `Console` (executables) folders.

## Step 1 — Install the tools

| Tool | Needed for | Download | Verify |
| --- | --- | --- | --- |
| Visual Studio Build Tools | the emulator | [visualstudio.microsoft.com/downloads](https://visualstudio.microsoft.com/downloads/) | `cl` inside *x64 Native Tools* |
| GNU Arm Embedded Toolchain | the firmware | [Arm GNU toolchain downloads](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads) | `arm-none-eabi-gcc --version` |
| CMake ≥ 3.20 | everything else | [cmake.org/download](https://cmake.org/download/) | `cmake --version` |
| Ninja | everything else | [ninja-build releases](https://github.com/ninja-build/ninja/releases) | `ninja --version` |
| Git | everything else | [git-scm.com](https://git-scm.com/downloads) | `git --version` |

Two things people trip over most often:

- On Windows the GNU Arm toolchain installer does **not** add its own `bin` folder to `PATH` — you have to do
  it manually (the one that contains `arm-none-eabi-gcc.exe`).
- MSVC is visible only from a *Developer* prompt. Every command below assumes the
  **x64 Native Tools Command Prompt for VS**; in a plain `cmd` the configuration of a `win-*` preset will fail.

## Step 2 — Clone and build

**1. Create a folder for the SDK** — it becomes `LETO_PATH`. The path can be anything, `C:\LETO` is used everywhere below.

**2. Copy the two scripts from [`guide/deploy/`](.) into it:**

```
C:\LETO\clone_env.bat
C:\LETO\setup.bat
```

**3. Start `x64 Native Tools` and run the following commands in it:**

```bat
cd /d C:\LETO
clone_env.bat
```

**4. Restart the terminal — close it and open it again.**

This is necessary so that the newly created (or updated) `LETO_PATH` environment variable gets loaded.

**5. Build everything:**

```bat
setup.bat
```

The build can take a while (the duration depends on your computer).

## Step 3 — Put the libraries on PATH

The console emulator is linked to the `LetoAPI` and `LetoCore` libraries dynamically to make debugging faster. For the system to be able to find the
built `.dll` files correctly, the `PATH` variable has to be extended with the path to the library binaries:

```
%LETO_PATH%Common\win-debug\bin
```

You can do this in the settings under *System → Environment Variables* (the user-level `PATH` is enough).

## Step 4 — Verify

```bat
"%LETO_PATH%Console\win-st7735-debug\bin\leto-console.exe"
```

A window with the console UI means the SDK built up healthy. Two running instances — one started normally, the second one with `--client` — is the
standard way to develop multiplayer features.

## Linux and Termux

🚧 This section is still being written (same in the [Russian version](README_ru.md)).

- `ubuntu-debug` is the only preset that requires Qt6.
- `termux-*` show the display output in a browser over HTTP
- `termux-win-debug` is needed to debug the web output on Windows without a phone.

<p align="center">
  <sub><a href="../../README.md">← Back to the project README</a></sub>
</p>

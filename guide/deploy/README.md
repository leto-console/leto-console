<p align="center">
  <sub><a href="../../README.md">← Back to the project README</a></sub>
</p>

<p align="center">
  🌐 <b>English</b> •
  <a href="./README_ru.md" title="Версия на русском">Русский</a>
</p>

# Setting up the Leto SDK on Windows

## What you need to know first

The SDK consists of three repositories that have to sit in one folder: [`LetoAPI`](https://github.com/leto-console/LetoAPI) (the API contracts),
[`LetoCore`](https://github.com/leto-console/LetoCore) (drivers and services) and
[`leto-console`](https://github.com/leto-console/leto-console) (the console itself). Deploying takes several steps: the
repositories are cloned, an environment variable called `LETO_PATH` is created with the path to their common folder, and
then every repository is built step by step for two presets — `win-debug` and `stm32f411xe-debug` for the libraries,
`win-st7735-debug` and `stm32f411xe-st7735-debug` for the console.

The build results are placed into the `Common` (libraries) and `Console` (executables) folders.

## Step 1 — Install the tools

| Tool | Download | Verify |
| --- | --- | --- |
| Visual Studio Build Tools | https://visualstudio.microsoft.com/downloads/ | `cl` inside *x64 Native Tools* |
| GNU Arm Embedded Toolchain | https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads | `arm-none-eabi-gcc --version` |
| CMake | https://cmake.org/download/ | `cmake --version` |
| Ninja | https://github.com/ninja-build/ninja/releases | `ninja --version` |
| Git | https://git-scm.com/downloads | `git --version` |

After installing, check two things:

- The GNU Arm toolchain installer does **not** add its own `bin` folder to `PATH` by default — you have to
  add it manually (the one that contains `arm-none-eabi-gcc.exe`).
- The MSVC tools are visible only in `x64 Native Tools Command Prompt for VS` — in a plain `cmd`, configuring a `win-*` preset will fail.

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

This is needed so that the new (or updated) `LETO_PATH` environment variable takes effect.

**5. Build everything:**

```bat
setup.bat
```

The build can take a while — it all depends on your computer.

## Step 3 — Put the libraries on PATH

The console emulator is linked dynamically to the `LetoAPI` and `LetoCore` libraries — that is what makes debugging fast.
For the system to find the built `.dll` files, add the path to the LETO libraries to the `PATH` environment variable
(the user-level one is enough):

```
%LETO_PATH%Common\win-debug\bin
```

You can do this under *System → Environment Variables*.

## Step 4 — Verify

Run the emulator:

```bat
"%LETO_PATH%Console\win-st7735-debug\bin\leto-console.exe"
```

The program should start without errors and show the game console window.

## Linux and Termux

🚧 This section is still being written (same in the [Russian version](README_ru.md)).

- `ubuntu-debug` is the only preset that requires Qt6.
- `termux-*` show the display output in a browser over HTTP.
- `termux-win-debug` is needed to debug the web output on Windows without a phone.

<p align="center">
  <sub><a href="../../README.md">← Back to the project README</a></sub>
</p>

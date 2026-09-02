# Contributing

Start with [`README.md`](README.md) for what this project is, and with
[`guide/deploy/README.md`](guide/deploy/README.md) for building it. This file is about changing it.

## Workflow

1. Fork the repository and branch from `main` as `NN-short-slug`, where `NN` is the issue number —
   for example `33-add-auto-app-loading-by-win-args`.
2. Follow the existing commit style: `feature: …`, `fix: …`, `docs: …`.
3. Make the CI matrix pass locally before opening a PR:

   ```sh
   cmake --preset win-st7735-debug && cmake --build --preset win-st7735-debug
   ```

   plus at least one `stm32f4*-…-debug` preset whenever hardware code is touched.
4. Link the issue, describe the user-visible change, and attach a screenshot or recording for UI work —
   the emulator makes this cheap.
5. Keep the documentation honest: update [`guide/deploy/`](guide/deploy) whenever setup steps or runtime
   behavior change, in both [`README.md`](guide/deploy/README.md) and
   [`README_ru.md`](guide/deploy/README_ru.md).

Good first issues are labeled in the [issue tracker](https://github.com/leto-console/leto-console/issues).

## House rules

- **C and C++17.** On ARM the code must build with `-fno-exceptions -fno-rtti`, so the firmware path
  avoids `std::` containers and heap allocation — use `StaticList`, `StaticText` and `BitmapView` from
  `LetoCore`.
- **Platform switches are explicit.** Platform-specific code goes behind `USE_HAL_DRIVER`, `__WIN__`,
  `__LINUX__`, `__TERMUX__` and `USE_SSD1306` / `USE_ST7735`, never behind ad-hoc file-name tricks. The
  macros come from `PRESET_DEFINE_MACRO` in `CMakePresets.json`.
- **Emulator and firmware stay behaviorally identical.** If a change needs a hardware-only branch, push
  it down into `LetoCore` rather than writing it into a scene.
- **Mind the layering.** System and product logic belongs in this repository, reusable drivers and
  services in `LetoCore`, public contracts in `LetoAPI`.
- **Don't patch generated code.** Everything under `Drivers/` and the `MX*_Init()` blocks comes from
  `leto-console.ioc` — edit the `.ioc` in STM32CubeMX and regenerate.

## Dependencies: two resolution paths

`CMakeLists.txt` branches on whether `LETO_PATH` is defined:

- **`LETO_PATH` set** — the console consumes the shared builds of `LetoCore` and `LetoAPI` from
  `%LETO_PATH%Common/<preset>`, which is what `scripts/setup.bat` produces in the order API → core →
  console. `CORE_BUILD_PRESET` in `CMakePresets.json` pins which core build folder is consumed, so after
  touching `LetoCore` you rebuild that library preset first.
- **`LETO_PATH` unset** — `LetoCore` is fetched from GitHub with `FetchContent` and built statically from
  source, and it pulls `LetoAPI` (and `cpp-httplib` for the web display) the same way. This is the path CI
  takes: one clone, no environment, a slower configure step.

Both paths compile the same sources; only where the dependencies come from differs.

## Recipes

### Adding a scene

1. Add an id to the `SceneID` enum in `Core/Inc/ProjectScenes/SceneID.hpp`, keeping it between
   `__SCREENS_START` and `__SCREENS_COUNT`.
2. Drop `MyScene.hpp` / `MyScene.cpp` into `ProjectScenes/` next to the existing scenes. Sources are
   collected by the `Core/Src/*/*` and `Core/Inc/*/*` globs in `CMakeLists.txt`, so no build-file edit
   is required.
3. Register the scene in `Application::InitSystemScenes()` / `InitUserScenes()`
   (`Core/Src/Application/System.cpp`) and route to it from a menu or a console command.

### Adding a persistent setting

Declare a `StoredDataCell<T>` / `NamedDataCell` at a free EEPROM offset in
`Core/Inc/ProjectSettings/ProjectSettings.hpp`, define it in the matching `.cpp`, then surface it in a
settings scene. Cells are applied on the fly — that is how the nRF24L01 pipe and channel settings became
hot-swappable.

### Adding a build preset

Presets inherit from `base`, which pins `bin/`, `lib/` and `build/` output locations, so a new preset is
normally a copy of the closest existing one with `MCU_CHIP`, `LINKER_SCRIPT`, `USE_SCREEN` and
`CORE_BUILD_PRESET` adjusted in `CMakePresets.json`. Add it to the CI matrix in
`.github/workflows/build.yml` if it should stay green.

## Style

Match the file you are editing. Headers use `.hpp` and live in `Core/Inc/`, one class per file, named
after what it does rather than what it drives. Comments in the source are in Russian — that is the
language of the team and of the issue tracker; documentation for outsiders is written in English.

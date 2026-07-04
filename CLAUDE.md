# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What This Project Is

Firelight is a libretro-based emulation frontend built with Qt6/QML (C++20). It wraps libretro cores (`.dll`/`.so`) to run games and provides a full GUI for library management, save states, RetroAchievements, input mapping, mods/patches, and Discord presence.

## Build

**Platform**: Windows with MSYS2/MinGW64 is the primary dev environment.

**Prerequisites** (via MSYS2 `pacboy`): Qt6.8, SDL2, spdlog, GTest, cmake, clang/gcc, nlohmann-json, cpr, libarchive, ffmpeg (libavcodec/libavformat/libavutil/libswscale/libswresample), pkg-config.

```bash
# Configure (MinGW Makefiles on Windows)
cmake -B build -G "MinGW Makefiles"

# Build
cmake --build build

# Run tests
cd build && ctest --output-on-failure

# Run a single test binary directly
./build/fl_test --gtest_filter="TestSuiteName.TestName"
```

The build copies `_cores/windows/` into `build/system/_cores/` and `content.db` into `build/system/` automatically.

Set `FL_DEBUG=1` in the environment to enable debug-level logging at runtime.

## Repository Structure

```
src/
  main.cpp                  # Entry point — initializes all services, registers QML types
  app/                      # Core C++ application logic, organized by domain
    emulation/              # EmulationService + EmulatorInstance (libretro lifecycle)
    libretro/               # Core — dlopen wrapper for libretro .dll/.so
    library/                # SqliteUserLibrary, LibraryScanner2, ROM/patch file types
    saves/                  # SaveManager, suspend points, save file I/O
    input/ + input2/        # SDL input, gamepad profiles, keyboard handler
    settings/               # SettingsService + SqliteSettingsRepository
    activity/               # Play session logging (SqliteActivityLog)
    achievements/gui/       # Qt/QML-facing achievement item models
    platforms/              # Platform metadata + PlatformService
    mods/                   # Mod/patch repository
    discord/                # Discord rich presence
    db/                     # SqliteUserdataDatabase, SqliteContentDatabase
  gui/                      # Qt bridge layer: QML models and proxy objects
libs/
  firelight/achievements/   # firelight_achievements static lib (AchievementService,
                            #   SqliteAchievementRepository, rcheevos integration)
  rcheevos/                 # RetroAchievements C library (submodule)
  discord/                  # Discord Social SDK wrapper
include/                    # Public interface headers (firelight:: namespace)
qml/                        # All QML UI files; entry point is Main4.qml
tests/                      # GTest unit tests (fl_test executable)
thirdparty/SQLiteCpp/       # Bundled SQLite C++ wrapper
cmake/                      # rcheevos.cmake, clang-checks.cmake
```

## Architecture

### Service Locator Pattern

A single global accessor base, **`ServiceAccessor`** (`src/app/service_accessor.hpp`), provides singleton access to services. Its contract is narrow: it exists only for the types the QML engine default-constructs via `qmlRegisterType` (EmulatorItem, the item models), which can't take constructor arguments and so must reach services through a locator. Everything the app constructs itself (Qt proxies, context-property models, EmulationService/EmulatorInstance) should take its dependencies via the constructor instead of inheriting the locator.

`ServiceAccessor` holds InputService, PlatformService, SettingsService, AchievementService (RAClient), IUserLibrary, SaveManager, UserdataDatabase, ActivityLog, ModRepository, SettingsRepository, DiscordManager, GameImageProvider, and the core-system directory. All members are set once in `main.cpp` via `set*()` calls before services are used.

### Qt/QML Bridge

C++ services are not exposed to QML directly. Instead:

- `src/gui/qt_*_proxy.*` classes (e.g., `QtAchievementServiceProxy`, `QtEmulationServiceProxy`, `QtInputServiceProxy`) wrap C++ service interfaces as `QObject`s with `Q_PROPERTY` and `Q_INVOKABLE`.
- These proxies are registered as QML context properties in `main.cpp` under names like `AchievementService`, `EmulationService`, `InputService`.
- Item models (e.g., `EntryListModel`, `AchievementListModel`) are exposed as context properties like `LibraryEntryModel`.
- A handful of types are registered with `qmlRegisterType` (e.g., `EmulatorItem`, `RetroAchievementsGame`, `LibraryEntry`).

### QML Singletons

`Constants.qml`, `ColorPalette.qml`, `AppStyle.qml`, `GeneralSettings.qml`, and `AppearanceSettings.qml` are declared as QML singleton types in `CMakeLists.txt`.

### Achievements Subsystem

The `firelight_achievements` library (`libs/firelight/achievements/`) is a self-contained static library:
- `AchievementService` — domain logic, session tracking, offline sync
- `SqliteAchievementRepository` — persistence (rcheevos3.db)
- `rcheevos/rcheevos_offline_client.*` — wraps the rcheevos C library for offline evaluation
- `rcheevos/ra_client.*` — online RetroAchievements API client (HTTP + login flow)

`RAClient` (accessed via `ServiceAccessor::getAchievementManager()`) is called from `EmulatorInstance` on every frame via `doFrame()`. Hardcore mode blocks rewind.

### Emulation Lifecycle

1. `EmulationService::loadEntry(entryId)` — resolves the entry from `IUserLibrary`, picks a core, loads save data, constructs `EmulatorInstance`.
2. `EmulatorInstance::initialize()` — must be called from the render thread. Sets up the libretro `Core` with video/audio receivers.
3. `EmulatorInstance::runFrame()` — called each render frame; drives the core, triggers auto-save every N seconds.
4. Achievements are ticked each frame via `RAClient::doFrame(core)`.

### Library Scanning

`LibraryScanner2` watches directories and hashes ROM files using rcheevos `rc_hash` to produce content hashes. These hashes are the primary key linking ROMs to achievements, save data, and library entries.

## CMake Targets

Domain logic lives in self-contained static libs under `libs/firelight/<name>/` (each with its own
`include/firelight/<name>/` public headers, `src/`, and a `firelight_<name>_test` GTest target). They
avoid Qt/sqlite where possible and are linked into `firelight_lib`.

| Target | Type | Purpose |
|---|---|---|
| `firelight` | Executable | Main application |
| `firelight_lib` | Static | All app/GUI code except `main.cpp` |
| `firelight_achievements` | Static | Achievements domain (AchievementService, rcheevos integration) |
| `firelight_input` | Static | Input: SDL controllers, profiles, shortcuts, mapping |
| `firelight_cheats` | Static | Typed cheats: `CheatEngine` + `SqliteCheatRepository` |
| `firelight_media` | Static | Screenshots + clip capture (ffmpeg) + `MediaService` |
| `firelight_audio` | Static | Audio DSP: rate control (DRC) + resampler (`AudioManager` stays in the app) |
| `firelight_settings` | Static | Settings service + catalog + core-option resolution |
| `firelight_platforms` | Static | Platform metadata + `PlatformService` |
| `firelight_library` | Static | Library scanning and user library |
| `firelight_saves` | Static | Save manager + suspend points |
| `firelight_activity` | Static | Play-session activity log |
| `firelight_mods` | Static | Mod/patch repository |
| `firelight_db` | Static | SQLite userdata/content databases |
| `firelight_discord` | Static | Discord rich presence |
| `patching` | Static | IPS/BPS/UPS/YAY0 patch formats |
| `rcheevos` | Static | RetroAchievements C library (submodule) |
| `discord` | Static | Discord Social SDK wrapper |
| `fl_test` | Executable | GTest unit tests (app-level) |
| `fl_qml_test` | Executable | QML unit tests |
| `firelight_input_test` / `firelight_cheats_test` / `firelight_audio_test` / `firelight_media_test` | Executable | Per-module GTest unit tests |

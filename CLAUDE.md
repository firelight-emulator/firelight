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

## Verifying the UI without clicking

A cold launch only ever builds `Main4` and the `/library` page — the other 12 routes are incubated
lazily, so a broken screen is invisible until someone navigates to it. `verify-ui` mounts all 22
screens (13 routes + `/quick-menu` + the 8 settings sections) against the real service graph and
exits non-zero if any fails to mount or logs a fatal QML message.

```bash
QT_QPA_PLATFORM=offscreen ./build/debug-win/firelight.exe verify-ui          # ~35s, all routes
QT_QPA_PLATFORM=offscreen ./build/debug-win/firelight.exe verify-ui --json   # machine-readable
./build/debug-win/firelight.exe --route /gallery                             # open one route by hand
```

Exit codes: `3` root object failed to build · `4` a fatal QML message was logged · `5` a route failed.

Three things that will waste your time otherwise:

- **`QT_ASSUME_STDERR_HAS_CONSOLE=1` is what makes Qt/QML messages visible at all.** Without it a
  Windows GUI build sends them to `OutputDebugString`, so grepping stdout for QML errors finds
  nothing whether or not any exist. `main.cpp` now sets it unless you override it.
- **From Git Bash, export `MSYS2_ARG_CONV_EXCL='*'`** or MSYS rewrites `--route /gallery` into a
  Windows path and the route silently won't match.
- **`QT_QPA_PLATFORM=offscreen` works** — Qt 6 builds the software scenegraph into `Qt6Quick.dll`
  rather than shipping a `plugins/scenegraph/` directory, so the missing directory is not evidence
  it is unavailable. Vulkan init fails offscreen, which is harmless: no route touches `EmulatorItem`.

`--fatal-warnings` promotes every QML warning to fatal. There is a nonzero baseline today, so it is
opt-in rather than the default.

## Code Style

Formatting is enforced by `.clang-format` (C++) and `.qmlformat.ini` (QML); `.clang-tidy` carries the
naming rules. Run the formatters rather than hand-aligning. Line endings are LF everywhere
(`.gitattributes`).

```bash
scripts/check-format.sh              # check files changed vs main
scripts/check-format.sh --fix        # reformat them in place
scripts/check-format.sh --all        # check the whole tree
git config core.hooksPath scripts/hooks   # block unformatted commits (opt-in)
```

`qmlformat`'s `-n` is `--normalize`, not a dry run: it reorders attributes and drops
explicitly-written property names. Never use it.

### Comments

- Javadoc (`/** */`) on classes and methods. Otherwise `//`
- **Required** on every interface, every class (public *and* private) and every method (public *and*
  private)
- Everywhere else, comment only when the code is not clear on its own
- **Never explain the reason.** A comment says *what* unclear code does, not *why* a choice was made,
  what it replaced, or what was tried before
- **Stay local.** Don't reference other layers, services, or callers (e.g. "the QML layer goes through
  X") unless that reference is required to understand the code right there. Don't restate what the
  declaration already shows (e.g. re-listing a class's base classes)
- No trailing period
- No inline comments in QML unless needed to understand that specific part
- Section headers look like:
  ```
  //****************
  // thing
  //****************
  ```

**Assistant-written comments carry a review marker.** Every comment an assistant adds gets a bare
`// TODO` on its own line directly above it, so the author knows to reword it or drop it. Remove the
marker once the comment is in the author's own words. A bare `// TODO` means exactly this; a real
task is `// TODO: <what to do>`, so the two stay greppable apart:

```
grep -rn '// TODO$'    # comments awaiting a rewrite
grep -rn '// TODO:'    # actual outstanding work
```

### C++

- 120 columns, 2-space indent
- Always braces, even for a one-line body
- Blank line after every closing brace
- Blank line before an `if`/`for`/`while`, unless the line above is a variable used by that block
- `is`/`has`/`should` prefixes for boolean variables and methods
- `m_` on member variables; `UPPER_SNAKE_CASE` for static const and constexpr; never a `k` prefix
- Getters start with `get` (`getX()`); interfaces start with `I`
- Member order: constructor, destructor, public methods, public members (there should almost never be
  any), private methods, private members
- Use `auto` liberally
- Includes: angles for anything on the include path (our libs), quotes for the same target or source
  directory
- Return early; prefer fewer levels of nesting
- Abbreviate only when the full word runs past 12 letters. Established short forms are exempt:
  `id`, `num`, `min`, `max`, `ok`, `db`, `url`, `ui`
- Prefer plain words. Do not use "provenance"

### QML

- 4-space indent; `id` is always the first line
- `objectName` is the readable component name, a pipe, then the identifying variable —
  e.g. `objectName: "LibraryNavigationMenuItem|" + displayText`
- `id` is `root` for non-controls, `control` for controls, otherwise descriptive
- Signal handlers on one line when they fit, otherwise multi-line; always braces where relevant
- Move a component into its own file once a second file uses it
- Reusable building blocks live in `qml/components/v2/`; full pages live in `qml/components/v2/pages/`
- **Always write property names explicitly**, even where a linter calls them redundant. `qmlformat`'s
  `NormalizeOrder` and `GroupAttributesTogether` reorder attributes and drop comments, which is what
  strips them — both are off in `.qmlformat.ini` and must stay off

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

`Theme.qml`, `AppStyle.qml`, `GeneralSettings.qml`, and `AppearanceSettings.qml` are declared as QML singleton types in `CMakeLists.txt`.

### Design system: color ← Theme, size ← AppStyle, components ← FL*

Two token singletons, one rule: **color comes from `Theme`, size/metrics from `AppStyle`, never a
literal at a call site.** `Theme` (`qml/Theme.qml`) owns semantic colors and reacts to the user's
appearance settings; `AppStyle` (`qml/AppStyle.qml`) owns metrics (font sizes, `spacing*`,
`controlHeight`/`rowHeight`, `iconSize*`, `radius*`) and reacts to `scale`/`density`. Every metric token
is `Math.round(base * scale …)`, and interactive heights are floored at `AppStyle.minTarget` (24px, WCAG
2.5.8). A raw `width: 200` / `spacing: 8` / `font.pixelSize: 14` bypasses the whole thing and won't scale
— use a token.

Prefer the reusable **`FL*` components in `qml/components/v2/`** (`FLButton`, `FLIconButton`,
`FLSearchField`, `FLPanel`, `FLScrollView`, `FLListRow`, `GameTile`, …) over hand-rolling a
`Button`/`TextField`/`Pane`/`ListView`. They already read the tokens, so they scale and re-theme for
free. Add a new one rather than making a one-off.

New screens use the **`FLPage`** scaffold (`qml/components/v2/surfaces/FLPage.qml`) — a title +
`headerActions` over a body that fills the rest — so a screen stops re-laying-out its own chrome
(this header is per-page, *not* the window `TitleBar`). Press **F11** in the running app for the live
**component gallery** (`qml/components/v2/dev/ComponentGallery.qml`, route `/dev/gallery`) showing
every `FL*` with its variants; add new ones there. Full walkthrough: `docs/making-a-screen.md`.

Use the `AppStyle.fontSize{Small,Medium,Large,XLarge}` tokens for all text `font.pixelSize`. **Never use
`font.pointSize`** — it renders ~25% smaller on macOS than on Windows because the logical-DPI baseline
differs (72 vs 96), whereas `pixelSize` is consistent across platforms and still scales on HiDPI displays.

### Icons

Render icons with the `Icon` component (`qml/components/Icon.qml`): `Icon { name: "settings"; size: 22; color: Theme.textPrimary }`. It draws a Material Symbols Rounded glyph via `Text.NativeRendering` and colors via `color`. NativeRendering (hinted, pixel-snapped) is the sharpest for small static UI icons; the smooth modes both fall down on these detail-dense glyphs at small sizes — `CurveRendering` aliases the many edges and distance-field (`QtRendering`) muddies the detail. The tradeoff is that NativeRendering isn't sub-pixel-accurate under fractional scale/rotation, so flip a specific instance to `CurveRendering` if it lives inside such an animation. Icon names map to codepoints in the `MaterialSymbols` singleton (`qml/MaterialSymbols.qml`, generated from the font's cmap); the same names as the `qrc:/icons/*` aliases. `FLIcon` is a thin backwards-compatible wrapper over `Icon` (its `icon:` = `Icon.name`). For non-Material artwork (console logos, folder art) use `VectorImage { preferredRendererType: VectorImage.CurveRenderer }`, not a raster `Image`.

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

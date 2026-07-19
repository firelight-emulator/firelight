# Code review checklist

A pass over every C++ module and `src/`, ordered so that nothing asks you to understand something
you haven't read yet.

**Scope: 472 files, ~51,700 lines** (plus 52 test files / ~12,800 lines, which are listed with the
module they cover). QML is not in here — it is the disposable view layer and is covered by
[cleanup-checklist.md](cleanup-checklist.md).

This is not the prose pass. [cleanup-checklist.md](cleanup-checklist.md) is for rewriting comments
and names into your voice. This one is for *understanding what the code does* — they can be done in
the same sitting per module, but they are different questions.

## How to use it

Work top to bottom. The order is derived from actual header dependencies, so by the time a module
asks you to know what a `Platform` or a `GamepadInput` is, you will have read it.

For each module: read the **entry point** first (it is the public surface and tells you the shape),
then the implementation, then the tests. Tests are the cheapest description of intended behaviour —
if a module's behaviour surprises you, its test file is usually the fastest way to confirm which of
you is wrong.

Tick a module only when you could answer: *what is this for, what does it own, who calls it, and
what happens if it fails?*

## Reading order

### Stage 0 — Vocabulary (~1,000 lines)

The types everything else is phrased in. Small, and reading them first makes every later module
cheaper.

- [ ] `include/firelight/` — 20 files, 929 lines. The shared public headers: `ICore`
      (`libretro/icore.hpp`, the libretro boundary — 28 pure virtuals, the seam the whole emulator
      is built on), `event_dispatcher.hpp` (ambient global bus), `input/gamepad_input.hpp` (the
      input vocabulary), `image.hpp`
  `libs/firelight/libretro/` is gone — it held only an unreferenced 5-line stub and a README for
  code that lives elsewhere. The README now sits at `src/app/libretro/README.md`, with the module
  it describes (Stage 4). The shared interfaces remain in `include/firelight/libretro/`.

### Stage 1 — Leaf modules (no dependencies on other Firelight modules)

Each of these can be understood in isolation. Good warm-up, and several are small enough to read in
one sitting.

- [ ] `libs/firelight/activity/` — 4 files, 241 lines. Play-session logging.
      Entry: `activity_log.hpp` · Tests: `tests/app/activity/`
- [ ] `libs/firelight/mods/` — 4 files, 141 lines. Mod/patch metadata.
      Entry: `mod_repository.hpp` · (depends on `platforms`, but only barely)
- [ ] `libs/firelight/audio/` — 6 files, 337 lines. DSP only: rate control + resampling.
      Entry: `audio_rate_controller.hpp` · Tests: 1 in-module
- [ ] `libs/firelight/cheats/` — 10 files, 551 lines. Typed cheats + per-frame RAM poke engine.
      Entry: `cheat_engine.hpp` · Tests: 2 in-module
- [ ] `libs/firelight/settings/` — 14 files, 1,612 lines. Settings catalog + three-tier resolution.
      Entry: `settings_catalog.hpp` · Tests: `tests/app/settings/`
- [ ] `libs/firelight/metadata/` — 17 files, 1,176 lines. Offline metadata + art providers.
      Entry: `game_metadata_source.hpp` · Tests: 3 in-module
- [ ] `libs/firelight/saves/` — 21 files, 1,383 lines. Save files + suspend points.
      Entry: `isave_manager.hpp` · Tests: 3 in-module
- [ ] `libs/firelight/media/` — 24 files, 2,441 lines. Screenshots, clip capture/mux, thumbnails.
      Entry: `media_service.hpp` (see also `clip_recorder.hpp`) · Tests: 5 in-module
- [ ] `libs/firelight/netplay/` — 30 files, 3,328 lines. Lobby/session/protocol + WebRTC transport.
      Entry: `protocol.hpp`, then `lobby_backend.hpp` · Tests: 6 in-module

### Stage 2 — Platform identity

- [ ] `libs/firelight/platforms/` — 5 files, 856 lines. The canonical `Platform` model and lookup.
      Entry: `platform.hpp` then `platform_service.hpp` · Tests: `tests/app/platforms/`

      **Note the dependency shape:** `platforms`' public header includes
      `<firelight/input/gamepad_input.hpp>` — but that header lives in the *shared*
      `include/firelight/` tree, not in the input module, and `firelight_platforms` does not link
      `firelight_input`. `input`'s dependency on `platforms` is implementation-only (`.cpp` and
      tests; its public headers mention `platforms` nowhere). So this is **not** a cycle: the
      graph is one-way, shared vocabulary ← `platforms` ← `input`. Read `gamepad_input.hpp`
      (Stage 0) before this, and the rest of `input` after.

### Stage 3 — The large domains

The three biggest modules. Budget real time; each is a sitting or two on its own.

- [ ] `libs/firelight/library/` — 36 files, 3,793 lines. Scanning, content identification, entries,
      folders (manual + smart).
      Entry: `entry.hpp`, then `content_loader.hpp` / `entry_resolver.hpp` ·
      Tests: `tests/app/library/`
- [ ] `libs/firelight/input/` — 45 files, 6,457 lines. **The largest module.** SDL devices,
      profiles, bindings, shortcuts/hotkeys.
      Entry: `gamepad_profile.hpp` → `controller_repository.hpp` → the shortcut engine ·
      Tests: 10 in-module (111 tests)
- [ ] `libs/firelight/achievements/` — 27 files, 5,091 lines. RetroAchievements: domain service,
      sqlite repo, rcheevos online/offline clients.
      Entry: `achievement_service.hpp` · Tests: `tests/app/achievements/`, `tests/app/rcheevos/`

      `achievement_service2.hpp` is referenced by zero files on purpose — it is a sketch of a
      replacement service, not dead code. Read `achievement_service.hpp` for what actually runs.
- [ ] `libs/firelight/discord/` — 7 files, 691 lines. Rich presence + lobby backend.
      Entry: `discord_manager.hpp` (depends on `netplay` + `platforms`, so read it after both)

### Stage 4 — The application layer (`src/app`, 129 files, ~16,000 lines)

Where the modules get composed into a running emulator. Read in this order — it follows the frame
lifecycle.

- [ ] `src/app/libretro/` — 16 files, 3,200 lines. The `Core` dlopen wrapper and the environment
      callback surface. Dense, and the place where libretro's API quirks live
- [ ] `src/app/emulation/` — 14 files, 1,884 lines. `EmulationService` + `EmulatorInstance`: load,
      run a frame, autosave, teardown · Tests: `tests/app/emulation/`
- [ ] `src/app/` (root) — 8 files, 3,101 lines. `EmulatorItem` (the QML-facing item),
      `EmulatorItemRenderer`, `EmulatorVulkanRenderer`, `ServiceAccessor`. **This is the render
      thread**; the trickiest concurrency in the codebase
- [ ] `src/app/audio/` — 7 files, 618 lines. `AudioManager` and the sink
- [ ] `src/app/netplay/` — 11 files, 1,595 lines. Host/guest pipelines, encode/decode, tees
- [ ] `src/app/patching/` — 12 files, 689 lines. IPS/BPS/UPS/Yay0 formats · Tests: `tests/app/patching/`
- [ ] `src/app/metadata/` — 4 files, 285 lines. `MetadataService`
- [ ] `src/app/util/` — 1 file, 52 lines
- [ ] `src/app/input/` — 2 files, 51 lines

  The `*/gui/` subtrees are Qt item models — thin adapters over the modules above. Read each right
  after its module if you prefer, or as a block here:

- [ ] `src/app/input/gui/` — 14 files, 1,426 lines
- [ ] `src/app/library/gui/` — 10 files, 1,370 lines
- [ ] `src/app/achievements/gui/` — 8 files, 499 lines
- [ ] `src/app/saves/gui/` — 8 files, 450 lines
- [ ] `src/app/netplay/gui/` — 6 files, 260 lines
- [ ] `src/app/activity/gui/` — 4 files, 155 lines
- [ ] `src/app/media/gui/` — 2 files, 189 lines
- [ ] `src/app/mods/gui/` — 2 files, 140 lines

### Stage 5 — The Qt bridge

- [ ] `src/gui/` — 53 files, 4,899 lines. The `qt_*_proxy` classes that expose C++ services to QML
      as `Q_PROPERTY`/`Q_INVOKABLE`, plus the list models. Mostly mechanical, but this is the layer
      where a rename breaks the UI at *runtime* rather than compile time ·
      Tests: `tests/app/gui/`

### Stage 6 — Entry points

- [ ] `src/cli/` — 19 files, 1,001 lines. Flag parsing, `scan`/`login`/`list` subcommands ·
      Tests: `tests/app/cli/`
- [ ] `src/main.cpp` — 851 lines. **Read last.** Every service is constructed and wired here, so it
      only makes sense once you know what each one is. This file is the actual dependency graph
- [ ] `src/` root leftovers — `image_cache.{cpp,h}`, `http2config.hpp`, `network_cache.hpp`.
      **Review question: are these still used?** They sit outside every subdirectory

### Stage 7 — Tests as documentation (52 files, ~12,800 lines)

Not a separate reading pass — check this off once you have read each module's tests alongside it.
Worth a final sweep for coverage gaps.

- [ ] `tests/app/` — the app-level GTest suite (511 tests)
- [ ] In-module test suites: `input` (111), `media` (17), `audio` (5), `cheats` (4)

## Known issues to keep in mind while reading

Not blockers, but you will trip over them:

- `EmulatorInstanceE2ETest.LoadRunSaveRewindResetTeardown` fails, and has since before the cleanup
  pass. Untriaged
- 27 QML tests fail, all in `tst_FLFocusHighlight.qml`. Invisible unless you run
  `fl_qml_test.exe -o <file>,txt` — the default output is silent
- `firelight_cheats_test` was silently not compiling (a test fake had drifted from `ICore`); fixed,
  but worth knowing that a stale `.exe` can pass while the target is broken
- Open decisions still sitting in [dead-code-triage.md](dead-code-triage.md) (156 files) and
  [todo-triage.md](todo-triage.md)
- Naming findings that need judgment, not a blanket fix, in
  [clang-tidy-findings.md](clang-tidy-findings.md) — 312 of them in `src/app/libretro/core.hpp`
  alone, which you will be reading in Stage 4

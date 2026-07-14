# firelight_tas — the in-app TAS harness (roadmap Phase 1)

The Qt-free, core-agnostic core of the Tool-Assisted Speedrun harness. Everything
here links into the app **and** unit-tests headlessly with plain GoogleTest (no Qt
Quick, no Vulkan, no SDL), so the harness is CI-gated from the ground up, before any
UI sits on top of it.

## What's here (built + verified)

| Unit | File | Role |
|---|---|---|
| `ScriptedRetroPad` | `scripted_retropad.hpp` | An `IRetroPad` whose state is a fixed `InputFrame` — the leaf the `CoreInputRouter` samples each frame. Feeding it back through `captureJoypadFrame()` reproduces the frame bit-for-bit. |
| `MovieInputProvider` | `movie_input_provider.hpp` | An `IRetropadProvider` that makes a movie the authoritative input source (installed via `ICore::setRetropadProvider`, which is how live input is suppressed). Cursor advances once per frame, never inside a poll. `RecordingInputProvider` is the record-side decorator. |
| `GreenzoneStore` | `greenzone_store.{hpp,cpp}` | A sparse savestate-keyframe ladder: instant seek (restore nearest keyframe ≤ target, replay the delta), edit-invalidation, and a spread-preserving budget evictor. Opaque state blobs → core/Qt-agnostic. |
| `TasSession` | `tas_session.{hpp,cpp}` | The resident controller: owns the frame cursor, movie, greenzone, and provider, and drives an `ITasEmulator` through play / instant-seek / edit (with rerecord counting). |
| `ITasEmulator` | `tas_emulator.hpp` | The minimal emulator surface `TasSession` drives; the app implements it against `EmulatorInstance` + the core. |

Plus, outside this lib:
- **`tools/tas`**: the canonical `firelight::input::InputFrame` replaces the old
  mirror; the `.fltm` format is **v3** (records the core `.dll` hash) and `verify`
  hard-gates on core drift (`tas::hashCoreFile`).
- **`src/app/emulation/emulator_instance`**: `setTasMode()` makes `runFrame()` a pure
  step (suppresses the wall-clock autosave); `setAutosaveIntervalSeconds()` tunes it.
- **`tests/app/tas`**: `MovieInputProvider` proven against the real `CoreInputRouter`,
  and TAS-mode proven against a real `EmulatorInstance` (via `fl_test`).

Run the units: `cmake -B build -DFL_BUILD_TAS=ON && ninja check` then `ctest`
(`firelight_tas_test` = 25 cases; the `tas_*` cases live in `fl_test`).

## Deferred (needs the full Qt/Vulkan GUI build — scaffolding only)

These require the render thread and QML engine and can't be verified in a headless
tree; they are the next slice of Phase 1:

1. **Command-queue integration** — TAS command types on `EmulatorItemRenderer` +
   an `m_tasActive` gate on `EmulatorItem`'s pacing timer, pinning
   `playbackMultiplier = 1`, so exactly one frame advances per step.
2. **`PianoRollView`** — a virtualized `QAbstractListModel` + QML `TableView`
   (toggle-cell, drag-paint) bound to a `TasSession` via a `QtTasStudioProxy`.
3. **Docked live video + held-buttons overlay** — a `SplitView` restructure of
   `NewEmulatorPage.qml` behind an `enableTasStudio` feature flag (OFF by default).

To wire `TasSession` to the real pipeline, `EmulatorInstance` needs a small
`setRetropadProvider` passthrough to the core; that lands with (1).

## Verification boundary

Everything above under "built + verified" compiles and passes tests locally with
MSYS2 mingw64 **g++** (both standalone and via the configured Ninja tree, run through
`ctest`). Not exercised locally: the CI **clang** build and the Linux path (no runner
here) — all code is standard C++20, so risk is low; the first `tas-testing` CI run is
the confirmation.

The **app-vs-CLI framebuffer/RAM equivalence** gate (replaying a real mGBA core
through the app's `libretro::Core` and hashing frames against the CLI oracle's stored
checkpoints) is **not** done, but was investigated: a real mGBA core **does load
headlessly** in the app `Core` wrapper (constructor + `init()` reach "Libretro core
loaded"), so a hashing `IVideoDataReceiver` (mGBA is `RETRO_HW_CONTEXT_NONE`) is the
right shape and the leak-to-avoid-teardown-exit trick works. The blocker: a *minimal*
in-process harness (stub config provider + hashing video receiver, **no audio
receiver**) **segfaults during init/loadGame** — the app `Core` expects more of the
context `EmulationService` normally wires (audio receiver / DRC, HW-render
negotiation). So the gate needs either that full context assembled headlessly (a
stub audio output + pointer provider) or an out-of-process replay harness that
tolerates the real-core exit. This is the next hard item on the critical path.
A stronger fixture also wants a **PPU-rendering** test ROM — `tests/testrom.gb` never
drives the LCD, so its framebuffer is constant (equivalence over it checks
format/pitch/dimension parity but not changing content).

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
- **`tools/tas/tas_app_equiv`**: the app-vs-CLI framebuffer **equivalence gate**
  (critical path #2) — replays a movie through the real app `libretro::Core` and
  asserts each frame's hash matches the CLI oracle's checkpoints. Its own executable
  (linking `firelight_emulation_lib`) run as a `ctest` under `FL_BUILD_TAS`.
- **`src/gui/models/piano_roll_model`** (`PianoRollModel`): the editable input-grid
  model — a `QAbstractListModel` over the movie's frames (frameIndex / buttons /
  isCurrent / isKeyframe roles) with `toggleButton()` and range `paintButton()`, every
  edit routed through `TasSession::editFrame`. The substance of the piano-roll,
  unit-tested headlessly (`QApplication`, like the other `gui/models` tests).
- **`src/gui/qt_tas_studio_proxy`** (`QtTasStudioProxy`): the QML-facing facade —
  owns the session + model and exposes transport / editing / playhead state to QML
  (`qmlRegisterType` as `TasStudioController`). Its C++ logic is unit-tested (6 cases);
  `loadDemo()` gives the UI content without a running game.

Run the units: `cmake -B build -DFL_BUILD_TAS=ON && ninja check` then `ctest`
(`firelight_tas_test` = 25 cases; the `tas_*` cases live in `fl_test`).

## Deferred (needs the full Qt/Vulkan GUI build — scaffolding only)

These require the render thread and QML engine and can't be verified in a headless
tree; they are the next slice of Phase 1:

1. **Command-queue integration** — TAS command types on `EmulatorItemRenderer` +
   an `m_tasActive` gate on `EmulatorItem`'s pacing timer, pinning
   `playbackMultiplier = 1`, so exactly one frame advances per step.
2. **`PianoRollView`** — the model (`PianoRollModel`), the facade (`QtTasStudioProxy`),
   and the QML screen (`qml/screens/TasStudioScreen.qml`, behind the
   `enableTasStudio` flag) are **scaffolded**: the C++ is unit-tested and the QML is
   qmllint-clean + registration-compiled. What remains is QML-runtime-bound —
   verifying actual rendering/interaction in the GUI build, routing the screen into
   app navigation, and (critically) making the proxy drive the LIVE game's
   `EmulatorInstance::runFrame` through the render-thread command queue instead of the
   `loadDemo()` stand-in.
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

The **app-vs-CLI framebuffer equivalence** gate (critical path #2) is **done**
(`tools/tas/tas_app_equiv`): the real mGBA core loads headlessly in the app `Core`
(a hashing `IVideoDataReceiver` suffices — mGBA is `RETRO_HW_CONTEXT_NONE`; the Core
is leaked to avoid the real-DLL teardown-exit; the earlier segfault was a null
`audioReceiver`, since `loadGame` calls `audioReceiver->initialize()` — fixed with a
`NullAudioOutput` stub). It PASSES on Windows/g++ and catches a corrupted checkpoint.
Two follow-ups remain: (1) `tests/testrom.gb` renders a **constant** framebuffer, so
the gate checks format/pitch/dimension/hash parity but not changing content — a
PPU-rendering test ROM would strengthen it; (2) the fixture is Windows-CLI-authored,
so the ctest runs on Windows only (the exe still builds on Linux) — a Linux-authored
fixture would extend the run to the Linux runner.

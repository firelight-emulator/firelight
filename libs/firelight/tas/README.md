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

## Live-game path — done + runtime-verified

The TAS Studio now drives a real running game and shows it. All confirmed in the
built Qt/Vulkan app on Pokémon Yellow (mGBA):

- **Frame-advance primitive** — `EmulatorItemRenderer::TasStepFrame` advances the
  live game exactly one frame even while paused (render() honors it ahead of the
  pause skip; pinned to a single frame). `EmulatorItem::setTasActive()` gates the
  pacing thread (atomic `m_tasActive`) + pins single speed, so the TAS layer is the
  sole frame driver. *Verified:* engaging TAS froze the game (no free-run); stepping
  advanced it frame-by-frame.
- **Studio transport → live game** — `QtTasStudioProxy::bindLiveEmulator()` binds the
  running `EmulatorItem` (exposed as `NewEmulatorPage.liveEmulator`, passed by the
  `Main3.qml` `/tas-studio` route); in live mode the transport calls
  `tasStepFrame()`/`setTasActive()`. *Verified:* Step advances the real game.
- **Docked live video** — the studio's video pane is a `ShaderEffectSource` mirror of
  the bound `EmulatorItem` (no reparenting; the game keeps rendering on its own page),
  fit to the game's aspect ratio. *Verified:* the live game shows in the studio and
  updates as you step.
- **Record → replay → seek over the live game.** Record captures each live frame's
  input into a movie (`EmulatorItemRenderer` record hook → `tasFrameRecorded`, shown in
  the piano-roll). Replay reinstalls a `MovieInputProvider` on the live core, restores
  the record anchor, and plays the movie back hands-off. Seek/rewind uses a render-side
  `GreenzoneStore` (keyframe every 30 frames): a row click or step-back restores the
  nearest keyframe in `synchronize()` and fast-forwards through the proven step path to
  the target. *Verified:* record/replay reproduce the run; row-click and rewind land
  frame-exact and stable. Two hard constraints, learned the hard way: on the Vulkan/HW
  path only `renderFrame()` refreshes `colorTexture()` (a restored state can't be shown
  without running a frame), and the core must never be advanced by a bare `runFrame()`
  outside `renderFrame()` nor restored while a QRhi command buffer is live — both
  corrupt the HW path. Backward seeks therefore show a brief scrub (proven path only).

## Remaining Phase 1 increment

Record / replay / seek over the live game is **done** (above). What's left toward a
full studio: **input editing over the live movie** (toggling a cell rewrites the movie,
invalidates the greenzone tail, and bumps the rerecord count — the `TasSession` edit
path exists headlessly but isn't yet wired to the live core), and movie **save/load**
(`.fltm`) from the studio.

## Nice-to-haves

- A Settings-UI toggle for `enableTasStudio` (currently a source default; the
  QML `Settings` value gets reset at startup, so enabling it means flipping the
  default + rebuilding).
- A held-buttons HUD overlay on the docked video.

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

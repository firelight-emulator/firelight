# Runahead — implementation sketch

A design sketch, not an implementation. Nothing in `src/` changes; this file plus
`data/runahead/` are the whole of it. Line references are to the tree at the time of writing.

---

## 1. What the feature is

Nearly every game has *internal input lag*: the number of frames between the frame on which the game
reads the controller and the frame on which the result of that read reaches the screen. It is a
property of the game's own code (input read → logic → render → present pipeline), typically 1–6
frames, and it is baked into the ROM. An emulator that is otherwise perfect still reproduces it
faithfully.

Runahead removes some of it. On each visible frame the emulator advances the real timeline by one
frame, then runs `N` more frames from that point with the same input held, shows the last of those,
and rolls back to the real timeline. What you see is the game's state `N` frames in the future, which
means a button press you make now is already reflected in the picture drawn now — as long as `N` is
at least the game's internal lag.

The cost is that the core runs `N + 1` frames per displayed frame plus one savestate save and one
load, and the correctness precondition is that the core's savestates are exact.

Two things follow, and they shape the rest of this document:

- **`N` is a per-game number.** It is the game's internal lag. `N` below it leaves lag on the table;
  `N` above it buys nothing and costs CPU. Asking a user to guess is asking them to guess a number
  they have no way of knowing, which is why §7 proposes shipping the numbers.
- **Not every game/core can do it.** Hardware-rendered cores, cores whose savestates aren't
  round-trip exact, and cores too slow to run at `N + 1`× are all out.

---

## 2. Where it lands in this codebase

The relevant seam is narrow. Frames are driven from the render thread in
`EmulatorItemRenderer::render()` (`src/app/emulator_item_renderer.cpp:705-720`):

```cpp
for (auto frame = 0; frame < framesThisPass; ++frame) {
  for (auto repeat = 0; repeat < repeats; ++repeat) {
    m_emulatorInstance->runFrame();
  }
}
```

and `EmulatorInstance::runFrame()` (`src/app/emulation/emulator_instance.cpp:387`) does four things
per frame: the autosave check, `m_core->run(0)`, `m_cheatEngine.apply()`, and
`achievements->doFrame()`.

Runahead needs those four split into two tiers, because three of them belong to the *real* timeline
and only one belongs to every core frame:

| per real frame | per core frame |
| --- | --- |
| autosave check | `m_core->run(0)` |
| `achievements->doFrame()` | `m_cheatEngine.apply()` |
| keyboard drain | |
| netplay / clip / instant-replay feeds | |

Cheats apply on every core frame because a hidden frame that lets the game overwrite a poked value
would show the un-poked value on screen.

The other four facts that matter:

- **Video is expensive and must be gated.** `EmulatorItemRenderer::receive()`
  (`src/app/emulator_item_renderer.cpp:192`) does a `QImage::convertToFormat` and possibly a rotate
  per frame, then publishes to the `FrameSlot`. Running that `N + 1` times per displayed frame would
  waste most of the runahead budget and would stamp `N` junk frame ids into the slot.
- **Audio must be gated.** `IAudioOutput::receive()` feeds the ring buffer that the pacing loop reads
  its buffer level from (`include/firelight/libretro/audio_output.hpp`). Hidden frames would push
  `N + 1`× the audio and the "audio" sync method would immediately mispace.
- **Serialization allocates.** `ICore::serializeState()` returns a fresh `std::vector<uint8_t>` every
  call (`include/firelight/libretro/icore.hpp`). Once per rewind point that is fine; once per
  displayed frame at 60 Hz, on a state that can be megabytes, it is not.
- **Only one core can be live at a time.** `libretro::g_ctx` is a file-scope pointer set in the `Core`
  constructor (`src/app/libretro/core.cpp:22`, comment: *"Only supports one core at a time for
  now"*). Single-instance runahead doesn't care; the second-instance variant (§5) does.

---

## 3. The algorithm (single instance)

One displayed frame, with runahead `N`:

```
1.  hidden = true                      // video + audio gates closed
2.  core.run()                         // this is real frame t
3.  serialize into m_stateBuffer       // the real timeline, saved
4.  achievements.doFrame()             // sees the real state, once
5.  for i in 1 .. N-1:  core.run()     // still hidden
6.  hidden = false                     // gates open for the last one only
7.  core.run()                         // frame t+N — this is what you see and hear
8.  deserialize m_stateBuffer          // back to end of frame t
```

`N + 1` core runs, one save, one load. Step 4 is deliberately placed after the save and before the
hidden runs, so RetroAchievements only ever evaluates memory belonging to the real timeline — no
double-counting, no triggers fired by a future that gets rolled back.

Audio is continuous even though every displayed frame is recomputed from a rollback: successive
displayed frames come from real frames `t+N`, `t+1+N`, `t+2+N` …, one frame of samples each. The
audible artifact is a discontinuity at the instant input changes, since the future being played is
replaced by a different future. That artifact is the reason the second-instance variant exists.

### What each gate must let through

The video gate blocks `receive()` only. `setSystemAVInfo`, `setPixelFormat`, `setScreenRotation` and
the hardware-render plumbing must pass through unconditionally — a core is entitled to change its AV
info on a hidden frame, and swallowing that would leave the frontend describing the wrong geometry.

```cpp
// src/app/emulation/runahead_gates.hpp (sketch)

/**
 * Forwards to the real receiver, dropping only the frames produced by runahead's hidden frames.
 * Everything descriptive still passes through
 */
class GatedVideoReceiver final : public libretro::IVideoDataReceiver {
public:
  explicit GatedVideoReceiver(IVideoDataReceiver *inner) : m_inner(inner) {}

  void setHidden(bool hidden) { m_hidden = hidden; }

  void receive(const void *data, unsigned width, unsigned height, size_t pitch) override {
    if (m_hidden) {
      return;
    }

    m_inner->receive(data, width, height, pitch);
  }

  // every other override forwards unconditionally
private:
  IVideoDataReceiver *m_inner;
  bool m_hidden = false;
};

/**
 * Forwards to the real output, swallowing the samples produced by runahead's hidden frames. Reports
 * them as consumed so a core that checks the return value doesn't retry
 */
class GatedAudioOutput final : public IAudioOutput {
public:
  size_t receive(const int16_t *data, size_t numFrames) override {
    if (m_hidden) {
      return numFrames;
    }

    return m_inner->receive(data, numFrames);
  }
  // ...
};
```

Both are owned by `EmulatorInstance` and installed in `initialize()`
(`src/app/emulation/emulator_instance.cpp:113`), wrapping the receiver the renderer passes and the
output the factory builds. When runahead is off they are one predictable branch per frame.

### Allocation-free state round trip

Add to `ICore`, defaulted so test doubles and the `Core` implementation can adopt it independently:

```cpp
/**
 * Writes the state into caller-owned memory. Returns false if `size` doesn't match the core's
 * serialize size or the core refuses. Lets a caller that saves every frame reuse one buffer
 */
virtual bool serializeInto(uint8_t *data, std::size_t size) const = 0;
```

`Core` already holds `LibretroDll::serialize(void*, size_t)`, so the implementation is one line and
`serializeState()` becomes a thin wrapper over it. `RunaheadController` sizes its buffer from
`getSerializeSize()` once at arm time and re-checks it whenever the size changes (disc swaps on some
cores change it).

### The controller

```cpp
// src/app/emulation/runahead_controller.hpp (sketch)

namespace firelight::emulation {

/**
 * Runs each displayed frame ahead of the real timeline so a game's internal input lag is absorbed.
 * Owns the rollback buffer and the video/audio gates; owns no policy about whether runahead is
 * allowed, which is resolved before it is armed
 */
class RunaheadController {
public:
  /**
   * How many frames ahead may ever be run. Past this the cost is real and the gain is not
   */
  static constexpr int MAX_FRAMES = 6;

  RunaheadController(::libretro::ICore &core, GatedVideoReceiver &video, GatedAudioOutput &audio);

  /**
   * Sets how many frames ahead to run, 0 to turn it off. Clamped to MAX_FRAMES
   */
  void setFrames(int frames);
  [[nodiscard]] int getFrames() const;

  /**
   * Runs one displayed frame. `runRealFrame` does what a plain frame does — one core run plus the
   * per-core-frame work — and is called N+1 times, hidden for all but the last. `onRealFrame` is
   * called once, with the core holding the real timeline's state
   */
  void runFrame(const std::function<void()> &runCoreFrame, const std::function<void()> &onRealFrame);

private:
  /**
   * Resizes the rollback buffer to the core's current serialize size, disabling runahead if the
   * core reports no savestate support
   */
  bool ensureBuffer();

  ::libretro::ICore &m_core;
  GatedVideoReceiver &m_video;
  GatedAudioOutput &m_audio;
  std::vector<uint8_t> m_stateBuffer;
  int m_frames = 0;
};

} // namespace firelight::emulation
```

and `EmulatorInstance::runFrame()` becomes:

```cpp
void EmulatorInstance::runFrame() {
  drainKeyboardEvents();
  checkAutoSave();

  const auto coreFrame = [this] {
    m_core->run(0);
    m_cheatEngine.apply(*m_core);
  };

  const auto realFrame = [this] {
    if (const auto achievements = m_context.achievementManager) {
      achievements->doFrame(m_core.get());
    }
  };

  if (m_runahead && m_runahead->getFrames() > 0) {
    m_runahead->runFrame(coreFrame, realFrame);
    return;
  }

  coreFrame();
  realFrame();
}
```

The no-runahead path is unchanged in behaviour and costs one null check.

---

## 4. What has to be blocked, and why

Runahead is only correct under conditions the frontend can check. Each of these forces `N = 0`:

| Condition | Why | Where it's known |
| --- | --- | --- |
| Hardware-rendered core | The core draws into the live GPU context; a hidden frame still paints, and rolling back the CPU state doesn't unpaint it. Their savestates are also large and slow | `EmulatorItemRenderer::m_usingHardwareRenderer` (`:148`) |
| Core reports `getSerializeSize() == 0` | No savestates, no rollback | `ICore::getSerializeSize()` |
| RetroAchievements hardcore | Same policy the codebase already applies to rewind; a per-frame state round trip is exactly what hardcore refuses to trust | `IAchievementClient` |
| Netplay session active | The remote timeline is authoritative; running the local core ahead and rolling it back fights whatever keeps the peers in step | `EmulationContext::retropadProvider` / `NetplayService` |
| Fast-forward / slow-motion multiplier ≠ 1 | `m_playbackMultiplier` already repeats `runFrame()`; multiplying that by `N + 1` is a cost nobody asked for and lag is not perceptible at 4× | `EmulatorItemRenderer::m_playbackMultiplier` |
| Catalog marks the core or game unsafe | §7 | `data/runahead/runahead.json` |

Two more that don't block but need a decision:

- **Rewind.** Both write savestates every frame-ish. They coexist — rewind points are captured from
  the real timeline via the command queue, which runs between displayed frames — but the combined
  serialize cost should be measured before shipping both on by default for the same platform.
- **Analog pointer glide.** `CoreInputRouter::pollInput()`
  (`src/app/libretro/core_input_router.cpp:12-32`) advances the cursor by a per-poll step. Under
  runahead the core polls `N + 1` times per displayed frame, so a stick-driven light-gun cursor would
  move `N + 1`× too fast. The gate should freeze glide accumulation on hidden frames (nudge only when
  not hidden), which is a third gate on the same flag.

---

## 5. The second-instance variant

RetroArch offers a second mode, and the reason is worth stating precisely: with a second core
instance the steady-state cost is **2× core time regardless of `N`**, not `(N + 1)`×.

It works by keeping a secondary instance permanently `N` frames ahead. Each displayed frame the
primary runs one hidden real frame; the secondary runs one frame and displays it. State is only
copied from primary to secondary when the input changed since the last frame, which is the only thing
that can invalidate the secondary's lookahead. Input-idle frames — most of them — cost two core runs
and no serialization at all. It also removes the audio discontinuity, because the audible stream
comes from one continuously-running instance rather than from a fresh rollback each frame.

For an mGBA-class core `(N + 1)`× at `N = 3` is affordable and this is unnecessary. For anything
heavier it is the only version that can ship. What it needs here:

1. **A second copy of the core DLL on disk.** `LibretroDll` wraps `QLibrary`
   (`src/app/libretro/libretro_dll.cpp:10`); loading the same path twice returns the same module with
   shared globals, and libretro cores keep their entire machine state in globals. The secondary must
   load a byte copy at a different path (`system/_cores/<id>.dll` →
   `<userdata>/runahead/<id>_2.dll`).
2. **Per-instance callback context.** `g_ctx` is file-scope and set once at construction
   (`src/app/libretro/core.cpp:91`). Since both instances run on the render thread and never
   concurrently, the fix is small: set `g_ctx = &m_callbackContext` at the top of `Core::run()`,
   `serializeState()`, `deserializeState()` and `loadGame()` rather than only at construction.
3. **A save-directory split.** The secondary must never write SRAM, and `CoreRunConfig::saveDirectory`
   (`include/firelight/libretro/core_run_config.hpp`) is per-game — point the secondary at a scratch
   directory and never harvest its memory.
4. **No achievements, no cheats-repo writes, no autosave on the secondary.** It gets the cheat *pokes*
   (so the picture matches) and nothing else.

Recommendation: build single-instance first, behind the mode setting; add second-instance as
`runahead-second-instance` (advanced, per-platform) once the first is proven. The controller's
interface above doesn't change — only its implementation of "run the hidden frames".

---

## 6. Settings and UI

Three keys in `data/settings/40-emulation.json`, resolving through the existing
game → platform → global → catalog-default chain (`SettingsService::getEffectiveValue`), applied by
`CoreSettingsApplier::refresh()` alongside `rewind-enabled`:

```json
{
  "key": "runahead-mode",
  "label": "Reduce input lag (runahead)",
  "group": "emulation-general",
  "order": 15,
  "description": "Runs the emulator slightly ahead of itself so the game responds to your input sooner. Automatic uses a per-game amount Firelight ships with. Uses noticeably more CPU, and is unavailable in RetroAchievements hardcore mode and on 3D consoles that render through your graphics card.",
  "keywords": ["lag", "latency", "delay", "runahead", "response", "input"],
  "type": "options",
  "default": "off",
  "options": [
    { "label": "Off", "value": "off" },
    { "label": "Automatic", "value": "auto" },
    { "label": "Manual", "value": "manual" }
  ]
},
{
  "key": "runahead-frames",
  "label": "Frames to run ahead",
  "group": "emulation-general",
  "order": 16,
  "type": "spinbox",
  "default": "1",
  "min": 1,
  "max": 6,
  "step": 1,
  "visibleWhen": [{ "key": "runahead-mode", "values": ["manual"] }]
},
{
  "key": "runahead-second-instance",
  "label": "Use a second emulator instance",
  "group": "emulation-general",
  "order": 17,
  "description": "Costs memory instead of CPU, and avoids the brief audio artifact runahead can cause. Recommended for demanding systems.",
  "type": "boolean",
  "default": "false",
  "advanced": true,
  "visibleWhen": [{ "key": "runahead-mode", "values": ["auto", "manual"] }]
}
```

Because the *mode* resolves through the normal tiers, "automatic everywhere but off for the N64" and
"manual 4 for this one game" both fall out without a new settings tier.

Resolution, run in `CoreSettingsApplier` on every settings change:

```
resolveRunaheadFrames(contentHash, platformId, coreId):
  if hardcoreActive or hardwareRendered or netplayActive:      return 0
  if catalog.isBlocked(coreId, contentHash):                   return 0
  mode = effective("runahead-mode")
  if mode == "off":                                            return 0
  if mode == "manual":                                         return clamp(effective("runahead-frames"))
  return clamp(catalog.lookup(contentHash, coreId)             // shipped per-game value
               .or(catalog.platformDefault(platformId, coreId))// shipped per-platform fallback
               .or(0))                                         // unknown => off, never a guess
```

Unknown resolves to **off**, not to a guessed 1. A wrong non-zero value costs CPU on every frame of a
game nobody measured; a zero costs nothing and is what the user already has today.

The one piece of UI worth building beyond the rows themselves: when a game is running and the mode is
Automatic, the quick menu should say which number is in force and where it came from ("Running 3
frames ahead — measured for this game"), because otherwise a shipped value is invisible and
unfalsifiable to the person it's affecting.

---

## 7. Shipping known-good values

### Why per game *and* core

The number is the game's internal lag, which is a property of the ROM — but the core changes it. A
core that polls input at a different point in its frame, or that emulates the console's own
frame pipeline differently, shifts the measured value by a frame. So the key is
`(contentHash, coreId)`, with `(platformId, coreId)` as the fallback.

Both halves of that key already exist and are already stable: `contentHash` is the rc_hash content
hash the library scanner computes (`libs/firelight/library/src/content_hasher.cpp`) and the same key
`SettingsService` game-tier values, saves and achievements use; `coreId` is `CoreInfo::id`, the DLL
base name (`src/app/libretro/core_registry.hpp`), which is already the key for
`data/settings/cores/*.json`.

### Where it lives

`data/runahead/runahead.json`, deployed to `system/runahead/` the same way `data/settings` is
(`CMakeLists.txt:542` and `:837`). Not `content.db`: that file is 24 MB, generated out-of-tree, and
keyed on rom md5/sha1 rather than content hash. A JSON file in `data/` is diffable, reviewable in a
PR, appendable by the measurement tool, and shippable in a patch release without regenerating a
database.

Schema (see `data/runahead/runahead.schema.json`, and `data/runahead/runahead.json` for the file the
tool appends to):

```json
{
  "version": 1,
  "platformDefaults": [
    { "platformId": 3, "core": "mgba_libretro", "frames": 0, "confidence": "unmeasured" }
  ],
  "blocked": [
    { "core": "mupen64plus_next_libretro", "reason": "hardware-rendered" }
  ],
  "games": [
    {
      "contentHash": "…",
      "title": "…",
      "platformId": 3,
      "core": "mgba_libretro",
      "frames": 3,
      "measuredLag": 3,
      "confidence": "measured",
      "method": "video-divergence",
      "scene": "in-game, overworld",
      "buttons": ["a", "left"],
      "stateBytes": 1048576,
      "costRatio": 4.0,
      "measuredAt": "2026-08-20",
      "firelightVersion": "…"
    }
  ]
}
```

`frames` is what the app uses; everything else is why. `costRatio` is `(N + 1)` adjusted by the
measured save/load cost — a value the UI can use to warn "this game needs 4× the CPU to run ahead"
before enabling it on a weak machine.

The catalog is loaded once at startup into a `RunaheadCatalog` (`libs/firelight/settings/` is the
natural home, next to `SettingsCatalog`, which already parses shipped JSON and has the same
lifetime), and looked up by `CoreSettingsApplier`.

**The seeded file ships empty.** Populating it is §8's job. Inventing plausible-looking numbers for
well-known games would produce a file that looks authoritative and is unverifiable, which is worse
than no file.

### Confidence levels

- `measured` — produced by the tool below, on a named scene, reproducible.
- `community` — imported from an external list (RetroArch's, forum threads). Useful as a starting
  point, but should be re-measured before it outranks `unmeasured`.
- `unmeasured` — a placeholder row that documents the key and forces `frames: 0`.

Only `measured` and `community` rows should ever produce a non-zero value.

---

## 8. Measuring the values

The measurement is mechanical and needs no knowledge of the game, which is what makes shipping a
catalog realistic rather than aspirational.

**Method (video divergence).** From one savestate, run the game twice for `F` frames: once with no
input, once with a button held from the first frame. Hash each produced video frame. The first frame
index at which the two runs disagree *is* the internal lag — it is the number of frames between the
press being read and the press being visible. Call it `L`. Then `N = L`: with `N` runahead the
displayed frame is produced by core frame `t + N`, and the press made at `t` first shows up in core
frame `t + L`, so it is on screen the instant it's pressed exactly when `N ≥ L`. Above `L`, nothing
further happens except CPU burn.

It works regardless of what the game is, needs no RAM map, and is immune to per-frame effects like
interframe blending because both runs have them.

**CLI shape**, alongside the existing `scan` / `verify-ui` / `doctor` subcommands in
`src/cli/cli_app.cpp:63-80`:

```bash
firelight measure-lag --entry 42 --suspend-point 1 \
                      --buttons a,b,left --frames 20 --repeats 3 --json [--append]
```

**What it does:**

1. Loads the entry headless — no audio output factory, and an `IVideoDataReceiver` that hashes the
   raw core buffer instead of converting it to a `QImage`. `EmulationService` already supports a
   headless launch path (`--muted`, `setStartMuted`), and `EmulatorInstance::initialize()` takes the
   receiver as an argument, so no new seam is needed.
2. Restores the named suspend point (or runs `--warmup` frames from boot), then serializes state `S`.
3. **Determinism check.** Runs `F` frames from `S`, restores `S`, runs `F` frames again, compares the
   hashes. Any disagreement means the core's savestate round trip isn't exact for this game →
   emit `unsafe`, stop. This check is the whole reason a shipped catalog is trustworthy: it is the
   one thing a user cannot discover for themselves except as a bug report.
4. For each requested button: baseline run, test run, first divergent index. Repeats `--repeats`
   times and requires agreement.
5. `L = max` over the buttons (undershooting leaves lag; overshooting only costs CPU, so the max is
   the right aggregate).
6. Measures serialize/deserialize wall time and state size, computes `costRatio`, and emits a catalog
   row — appending it to `data/runahead/runahead.json` under `--append`.

**Curation workflow**, since step 2 needs a *representative* scene:

1. Play to a spot where the button being tested visibly does something within ~20 frames — in-game,
   not a menu, not a cutscene. Save a suspend point.
2. Run the tool. If it reports no divergence within `F` frames, the scene was wrong, not the game.
3. Repeat for a second scene if the game has distinct modes (overworld vs battle, driving vs menu)
   and keep the max.
4. Commit the row. The diff is one JSON object with the evidence attached.

**Known caveats, to be recorded in the row rather than papered over:**

- Lag can differ per action within one game; `buttons` records what was actually tested.
- Games with genuinely non-deterministic behaviour (a core reading a real clock) fail step 3 and are
  correctly excluded.
- The number describes emulation lag only. Display, compositor and controller lag are unaffected and
  are much larger for most people.

---

## 9. Testing

Everything above is testable in `fl_test` against a fake `ICore` — no GPU, no game, no timing.

- **Frame accounting.** A counter core: one displayed frame at `N = 3` runs the core 4 times,
  serializes once, deserializes once, and leaves the core's internal frame counter exactly 1 higher
  than before.
- **Gating.** A counting video receiver and audio output see exactly one frame and one audio batch
  per displayed frame at any `N`. AV-info calls made on a hidden frame still arrive.
- **Achievements.** A fake achievement client's `doFrame` is called once per displayed frame, and the
  memory it sees belongs to the real timeline (the counter core exposes its frame number as RAM).
- **Cheats.** The cheat engine applies on every core frame, hidden ones included.
- **Blocking.** Each condition in §4 resolves to `N = 0` through `CoreSettingsApplier`.
- **Catalog.** Lookup precedence (game → platform default → 0), `blocked` and `unsafe` handling,
  clamping to `MAX_FRAMES`, and a malformed file degrading to "no catalog" rather than throwing —
  the same contract `SettingsCatalog` already holds.
- **Measurement.** A fake core with a scripted lag of `k` frames: the tool must report `L == k`, and a
  fake core with a deliberately lossy savestate must fail the determinism check.

---

## 10. Suggested order

1. `ICore::serializeInto`, the two gates, and `RunaheadController`; `runFrame()` split into the two
   tiers. Manual mode only, hidden behind the advanced flag. Every §4 block enforced.
2. `RunaheadCatalog` + the shipped (empty) JSON + Automatic mode + the quick-menu readout.
3. `measure-lag`, then a first curation pass over a bounded, affordable set — 2D platforms where
   `(N + 1)`× is cheap (GB/GBC/GBA/NES/SNES/Genesis). That is where the feature is both safest and
   most felt.
4. Second-instance mode, which is what makes the heavier platforms possible at all.
5. Auto-degrade: `EmulationRateController` already knows the frame budget
   (`src/app/emulation/emulation_rate_controller.hpp`); if displayed frames miss their deadline for
   `K` consecutive frames with runahead on, drop `N` by one and log it, recovering after a sustained
   good period. Without this, an underpowered machine turns a latency feature into a stutter feature.

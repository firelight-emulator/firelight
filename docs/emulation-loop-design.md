# The emulation loop

A design for pacing that holds at every refresh rate, in every sync method, with vsync on or off. It
follows `frame-pacing-investigation.md`, which ends with the 60 Hz fault identified but not fixed.
Everything measured here came out of `tools/pacing-sim`, which drives the real `FramePacer`,
`RefreshCounter` and `EmulationRateController` — they are Qt-free, so the whole policy compiles and
runs without Qt, Windows or a GPU.

## The decision

Take the rate from a clock. Take the phase from the display. Never take a frame count from a present.

Run the core on its own thread into `FrameSlot` and let the render thread sample it. That is one code
path for every mode, refresh rate and vsync state; the current design needs three and fails one.

## The rules

1. **Frame counts never come from present timestamps.** A present says a frame was queued, not that a
   refresh happened.
2. **Rate from a timer, phase from observed presents.** In display mode a slow correction decides
   where in the refresh interval a frame lands. Phase only, never count. Native has no phase term.
3. **Debt is dropped, never repaid.** One frame per period. Late by less than a period, run now and
   keep the anchor. Late by more, re-anchor to now.
4. **The core runs on its own thread.** The GUI event loop leaves the emulation path entirely.

## Why the present train cannot be counted

Replaying present trains through the real classes, the loss against timestamp noise is a step, not a
gradient. 3600 presents on a 59.959 Hz panel, Snes9x at 60.0988 fps:

| noise (of a refresh) | absolute | frames run | frames shown | never seen |
|---|---|---|---|---|
| ± 0 % | 0 µs | 3600 | 3600 | 0 |
| ± 1 % | 170 µs | 3599 | 3220 | **379 (10.5 %)** |
| ± 5 % | 834 µs | 3599 | 3225 | 374 |
| ± 50 % | 8.3 ms | 3599 | 3226 | 373 |

A fifth of a millisecond costs what half a refresh costs. So the 3-image FIFO chain delivering
presents in pairs is *a* way to produce noise, not *the* problem: at `refreshesPerFrame == 1` the
counter has no tolerance at all, and D3D11 looks healthy only because `QDxgiVSyncService` hands Qt
clean timestamps. `rpf == 1` is the only broken configuration; at 2 and 4 a miscount is a fraction of
a frame and the carry absorbs it.

## The result that settles the clock objection

A clock matched to the display's *nominal* rate still runs against its *actual* rate. 300-second runs:

| case | clock only | clock + phase-lock |
|---|---|---|
| exact match (unrealistic) | 0 lost | 0 lost |
| display 50 ppm fast | 0 lost | 0 lost |
| display 100 ppm slow | 1 lost | 0 lost |
| reported 60.000, real 59.959 | **12 lost** | 0 lost |
| reported 60.000, real 59.94 | **17 lost** | 0 lost |

Crystal drift is a non-issue. A wrong *reported* rate is the real cost, and the phase term erases it.
That is what `RefreshCounter` was built to solve; it solved it with a mechanism that pays 10 % of
frames, where a slow phase nudge pays nothing.

Unverified: this assumes `QScreen::refreshRate()` can report a rounded 60.000 for a panel running
59.94 or 59.959. One startup log line settles it.

## The modes

Each mode is defined by what it refuses to compromise.

- **native** — always the core's rate. Never reads `displayHz`, so it needs no reconfiguration on
  `screenChanged` or `refreshRateChanged`.
- **display** — always a whole division of the monitor's rate, with no tolerance gate.
  `k = round(displayHz / contentFps)`, run at `displayHz / k`, bend the audio by the same ratio.
- **auto** — display when the resulting speed error is inside the threshold, native otherwise.
- **fixed** — a clock at `target-framerate`.

`resolveRates()` currently sends Auto and Display down the same branch, so display inherits auto's
threshold. That is why "monitor" silently becomes Fixed at 144 Hz. Splitting them is the fix.

Display mode's video cadence is exact on every row below — one frame per `k` refreshes, no drops, no
repeats. What varies is what it costs in speed, and therefore what auto picks. Snes9x at 60.0988 fps:

| panel | k | runs at | speed | pitch | auto picks |
|---|---|---|---|---|---|
| 59.959 Hz | 1 | 59.959 | −0.23 % | −4 c | display |
| 60.000 Hz | 1 | 60.000 | −0.16 % | −3 c | display |
| 119.961 Hz | 2 | 59.980 | −0.20 % | −3 c | display |
| 240 Hz | 4 | 60.000 | −0.16 % | −3 c | display |
| 174.962 Hz | 3 | 58.321 | −3.0 % | −52 c | native |
| 165 Hz | 3 | 55.000 | −8.5 % | −153 c | native |
| 100 Hz | 2 | 50.000 | −16.8 % | −318 c | native |
| 144 Hz | 2 | 72.000 | +19.8 % | +313 c | native |

Choosing "monitor" explicitly on a 144 Hz panel runs the game a minor third sharp. That is the mode
doing what it says; auto's threshold is what stops anyone arriving there by accident.

### Phase-lock belongs to display mode only

In native mode the phase drifts against the display by design — that drift is the beat, and
correcting it would be correcting away the mode. In display mode the claim is that the game runs at
the monitor's rate, and phase-locking to the observed vblank grid makes that mean the monitor's
actual rate rather than the reported one.

### The threshold is auto's, and stays tight

With display ungated, the tolerance decides one thing: when auto hands the user to native. That makes
pitch the criterion and 1 % the right size — the worst auto accepts is about 17 cents, and every
genuine match lands under 0.5 %. RetroArch, DuckStation and PCSX2 use 5 %, but there it is a ceiling
on allowed bend that real content never approaches. Adopting 5 % here would let auto pick display at
174.962 Hz (52 cents flat) and at 144 Hz with PAL content (71 cents flat), silently, as a default.

### Phase target is a latency dial

Once phase is controlled, where in the refresh interval the frame completes is a free choice worth
about 11 ms. Input poll to scanout, 59.959 Hz, zero frames lost at every target:

| target phase | mean | headroom left for the frame |
|---|---|---|
| 10 % | 16.01 ms | 15.0 ms |
| 50 % | 9.34 ms | 8.3 ms |
| 75 % | 5.17 ms | 4.2 ms |
| 90 % | 2.67 ms | 1.7 ms |

The target should be adaptive: `1 - (measured core time + upload time + margin) / period`. Start at
75 %. This is a refinement, not part of the correctness fix.

## Where the core has to run

The premise that the core must run on the render thread is inherited from OpenGL, and OpenGL is not
hooked up. `getPreferredHwRender()` returns `RETRO_HW_CONTEXT_VULKAN` under the Vulkan RHI and
`RETRO_HW_CONTEXT_NONE` otherwise; there is no third answer. The hardware set is two cores,
mupen64plus-next and PPSSPP, both Vulkan.

For a software core, `runFrame()` issues no graphics call of any kind. It drains keyboard events,
runs `retro_run`, applies cheats, ticks achievements; the video callback publishes a `QImage` to
`FrameSlot`. The one RHI-bound step is `uploadCurrentFrame()`'s `uploadTexture`, which happens after
the frame. The `beginExternal()`/`endExternal()` scope around the `runFrame()` loop is protecting a
frame that makes no GL or Vulkan calls, and `tests/app/emulation/emulator_instance_e2e_test.cpp`
already calls `runFrame()` with no window, no RHI and no render thread.

On Vulkan the separation already exists. The core calls `negotiation->create_device()` and gets its
own `VkDevice` and `VkQueue`; frames cross as an image shared through `VK_KHR_external_memory_win32`;
`lock_queue`/`unlock_queue` are wired, which is the libretro contract for a core submitting from its
own threads. Only `ensureSharedImage()` touches Qt's `QRhi`, and only on a resize.

The real obstacle to moving `runFrame()` is not graphics. `AudioManager` is constructed inside
`initialize()` "on the render thread for Qt audio thread-affinity", and `QAudioSink`/`QIODevice` are
not thread-safe with `receive()` called from that thread. Settle where the sink lives first.

### Qt interop facts, read from the 6.11 source

- `QRhi::setQueueSubmitParams()` with `QRhiVulkanQueueSubmitParams` (6.9+) hands Qt semaphores to wait
  on, signal and present-wait. It applies to the frame's single submit. Binary semaphores only — Qt
  builds a plain `VkSubmitInfo` with no `pNext` — and Vulkan-only; other backends no-op it silently.
- Qt captures an imported image's layout once at `createFrom()` and tracks it after. A core queue
  transitioning it behind Qt's back leaves the tracking stale; correct with `setNativeLayout()`.
- Qt never destroys an imported `VkImage` and gives no signal when it is done with one. Retire it only
  after `framesInFlight` (2 on Vulkan) `afterFrameEnd` emissions, from `graphicsStateInfo()`.
- `beforeFrameBegin` fires before `rhi->beginFrame()` and is where a CPU wait stalls least.
- `QQuickWindow::update()` is not thread-safe from an arbitrary thread. Use
  `QMetaObject::invokeMethod(window, "update", Qt::QueuedConnection)`.
- Qt Quick sets `QRhiSwapChain::NoVSync` from `requestedFormat().swapInterval() == 0`, and
  `QRhiVulkan` then picks MAILBOX where supported, IMMEDIATE otherwise. `presentationLocked` reading
  `requestedFormat()` is correct. MAILBOX is non-blocking but not tearing.

## Ruled out — do not re-derive

- **Widening `DISPLAY_MATCH_TOLERANCE` to 5 %.** Helps in two cases, both at half a semitone of
  detune, and does nothing for 144 or 165 Hz with 60 fps content.
- **Controlling Qt's swapchain depth or present mode.** Qt Quick never sets
  `QRhiSwapChain::MinimalBufferCount`, so Vulkan gets 3 images and FIFO. No API, no
  `QQuickGraphicsConfiguration` setting, no environment variable. `QT_VK_PHYSICAL_DEVICE_INDEX` is
  Qt's only Vulkan variable and it selects a GPU.
- **`QSG_RENDER_LOOP=windows`.** Removed in Qt 6.0; the name is accepted and mapped to `basic`.
- **`QQuickGraphicsDevice::fromDeviceObjects()`** to make Qt adopt the core's device. It must be set
  before the scene graph initialises, and `create_device` happens when a game loads.
- **`makeThreadLocalNativeContextCurrent()`** as a route to a GL context on another thread. It makes
  Qt's own context current, and from a third thread `QOpenGLContext::makeCurrent` calls `qFatal`.

## What is inherent

Every mode pays something on a fixed-refresh panel. The point of separating them is that the player
chooses which cost.

- **Native beats against the display.** Where the rates are close this is a slow periodic repeat:
  Snes9x on 59.959 Hz every 7.15 s (8.4/min), GBA every 4.32 s (13.9/min), NTSC every 52.6 s
  (1.1/min). The 7.15 s figure matches the investigation's measured 7.1 s.
- **At 144 and 165 Hz native is pulldown, not a beat.** 60 fps content occupies 2.396 refreshes on a
  144 Hz panel, so frames alternate 2- and 3-refresh holds in roughly a 60/40 split. There is no
  periodic hitch to count. VRR is the only cure.
- **Display mode's cost is speed, everywhere.**
- **Qt's 3-image swapchain** costs up to a refresh of latency. Only a native presentation surface
  removes it, and that collides with QML glass over gameplay.

## Work, in dependency order

1. **Stop the stats resetting mid-game.** `SET_GEOMETRY` reaches `setSystemAVInfo`, which calls
   `PerformanceStats::reset()` unconditionally, re-zeroing a 120-frame warm-up. If SNES cores send
   geometry more than once every two seconds, `frameTimeMs` and `frameRate` never update — and
   `reset()` does not clear the snapshot fields, so the overlay shows frozen plausible numbers rather
   than zeros. Fix before trusting any reading.
2. **Delete `RefreshCounter`, `framesDueOnPresent()` and the `m_submitCount` path.** `SyncMode::Display`
   survives as a rate choice, not an execution mode: one clock-driven executor, and the mode supplies
   a rate and a phase-lock flag.
3. **Split Auto from Display in `resolveRates()`.** Display computes `k` and runs at `displayHz / k`
   unconditionally. Auto keeps the threshold. Native stops reading `displayHz`.
4. **Anchor schedule with drop-don't-repay**, replacing `m_owedFrames` and `MAX_CATCH_UP_FRAMES`.
   `PrecisionWaiter` is already the right shape.
5. **Phase estimator, display mode only.** Consumes `frameSwapped` timestamps, keeps a slow vblank-grid
   average, returns a phase correction and never a count.
6. **Resolve the `AudioManager` thread affinity**, then move `runFrame()` to the emulation thread.
7. **Report the resolved mode, not the request.** `Pacing:` shows `getSyncMethod()`, and an
   unrecognised `sync-method` becomes `Native` silently. Both cost a wrong diagnosis once already.
8. **Later:** replace the blit fence with `setQueueSubmitParams()`; DRC gain 0.02 to 0.005.

## Open

- Does `QScreen::refreshRate()` round on real panels?
- How often do SNES cores send `SET_GEOMETRY`?
- `layer.enabled: true` on the EmulatorItem inserts an extra FBO pass before the swapchain.
- `SET_HW_RENDER` inspects neither `context_type` nor version and returns true to anything. A core
  asking for `OPENGL_CORE` under Vulkan is accepted, then runs no frames.
- `k = round()` at 144 Hz gives 72 fps, 20 % fast. `k = 3` would give 48 fps, 20 % slow. Both are the
  same distance out, but fast and slow do not play the same.

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

**libretro does not require a render thread, and never mentions one.** Nothing in `libretro.h` says
`retro_run` must be called on any particular thread; the concept does not exist in the API. The only
threading language is about something else: callbacks the core registers *"can be invoked from any
thread, so their implementations must be thread-safe"* (`:1114`), the camera callback *"runs in the
same thread as `retro_run()`"* (`:1204`) — which treats that thread as a given the frontend chose —
and netpacket and microphone calls that *"must be called during `retro_run`"* (`:5520`, `:5532`,
`:7185`), which is about when, not where.

`libretro_vulkan.h` says the opposite of a render-thread requirement, three times:

> *"The Vulkan API is heavily designed around multi-threading, and the libretro interface for it
> should also be threading friendly. A core should be able to build command buffers and submit
> command buffers to the GPU from any thread."* (`:113-118`)

> *"Vulkan cores should be able to be freely threaded without lots of fuzz."* (`:252`)

> *"Queue submission can happen on any thread. Even if queue submission happens on the same thread as
> `retro_run()`, the lock/unlock functions must still be called."* (`:476-478`)

That last is the only unconditional threading mandate in either header, and the current code honours
it — `emulator_vulkan_renderer.cpp:167` takes `m_vkQueueMutex` around its own submit, which is
required even for same-thread submission because the frontend also submits to that queue.

So the render-thread premise comes from this repository's own comments
(`emulator_instance.hpp:29-31/44/52`, `emulator_instance.cpp:464-465`, `CLAUDE.md`), and the
reasoning is GL-shaped: it holds only when the core draws into *the frontend's* context.

**The real constraint is a pairing, not a location.** A GL context is thread-affine — current on at
most one thread at a time — so `retro_run` must be called on whichever thread holds the context. Which
thread that is, is the frontend's choice. Putting the context on the emulation thread and putting it
on the render thread are both legal; the first keeps one threading model for every core type, the
second lets a GL core render straight into Qt's framebuffer when Qt is also on GL.

`getPreferredHwRender()` returns `RETRO_HW_CONTEXT_VULKAN` under the Vulkan RHI and
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

### Hardware rendering on any backend, with any core API

The goal is that the app runs on whatever QRhi backend suits the host and the core uses whatever
graphics API it supports. libretro is built for that, and the protocol is not one mechanism but
three. Read from the vendored headers:

| core API | how the frontend supplies it | can the core own its device? |
|---|---|---|
| OpenGL, GLES | plain `retro_hw_render_callback` — the frontend sets `get_proc_address` and `get_current_framebuffer`, and the context is whatever is current on the thread calling `retro_run` | No |
| Vulkan | negotiation interface (`create_device`) plus `retro_hw_render_interface_vulkan` | **Yes**, uniquely |
| D3D9, D3D10, D3D11, D3D12 | `retro_hw_render_interface_*` only — the frontend hands over its own device | No |

`retro_hw_render_interface_type` (`libretro.h:3128-3157`) lists Vulkan, D3D9, D3D10, D3D11, D3D12 and
GSKIT_PS2. `retro_hw_render_context_negotiation_interface_type` (`libretro.h:3387`) lists **Vulkan and
nothing else**. So Vulkan is the only API where a core creates its own device; everywhere else the
frontend owns it. Note also that only `libretro.h` and `libretro_vulkan.h` are vendored here — the
D3D interface structs live in `libretro_d3d.h` upstream and would have to be added.

There is no OpenGL entry in either enum, and that is correct rather than an omission: GL needs no
interface struct, because a GL core simply renders into whatever framebuffer is bound in whatever
context is current. `GET_HW_RENDER_INTERFACE` documents this directly — *"Since not every
libretro-supported hardware rendering API has a `retro_hw_render_interface` implementation, a result
of `false` is not necessarily an error"* (`libretro.h:1634-1637`).

**The two layers are independent.** Giving the core a context is one problem; getting its image into
Qt is another, and neither constrains the other. The second always has a working fallback — read the
frame back to the CPU, publish to `FrameSlot`, and let the existing `uploadTexture` path show it,
which is exactly what software cores already do. So this is a performance matrix, not a capability
matrix: every pair works, and specific pairs get a zero-copy path.

| core API vs. Qt backend | same API | different API |
|---|---|---|
| Vulkan | external memory, as `EmulatorVulkanRenderer` already does | GL imports the Vulkan allocation, or read back |
| OpenGL | share group (`AA_ShareOpenGLContexts`) | `GL_EXT_memory_object_win32` onto the shared allocation, or read back |
| D3D11/12 | DXGI shared handle | Vulkan imports the DXGI handle, or read back |

The interop extension names are not verified against any driver here; readback is the baseline that
always works and the fast paths are optimisations on top of it.

### Negotiation handlers: what each one owes the core

Negotiation only works if the frontend says what it can actually do, and every relevant handler
currently answers yes regardless. These are the ones to reconnect before any backend beyond Vulkan
can work; the list is what each is supposed to return, not a claim that any of it is news.

- **`SET_HW_RENDER`** returns `true` for any `context_type`, without inspecting it. The header says to
  return *"`false` if `data` is `NULL` or the frontend can't provide the requested rendering API"*
  (`libretro.h:933-936`). A core asking for `OPENGL_CORE` under Vulkan is told yes, then runs no
  frames, because `render()` has only a Vulkan hardware branch.
- **`GET_PREFERRED_HW_RENDER`** always returns `true`. That return value is not "did it work" — it
  means *"the frontend is able to use a hardware rendering API besides the one returned"*
  (`libretro.h:1981-1985`). We claim flexibility while `getPreferredHwRender()` writes
  `RETRO_HW_CONTEXT_NONE` on every non-Vulkan backend.
- **`GET_HW_RENDER_INTERFACE`** returns `true` even on the path where `getHwRenderInterface()` logged
  an error and wrote nothing to `*iface` (`emulator_item_renderer.cpp:192-200`), handing the core an
  uninitialised pointer.
- **`GET_CURRENT_SOFTWARE_FRAMEBUFFER`** returns `true` without touching `data` at all. A core that
  takes the offer renders into an uninitialised `retro_framebuffer`.
- **`GET_HW_RENDER_CONTEXT_NEGOTIATION_INTERFACE_SUPPORT`** answers the wrong field. `data` is a
  `retro_hw_render_context_negotiation_interface *` — a struct of `{interface_type,
  interface_version}` — and the handler casts it to the *enum* type and writes `VULKAN` (which is
  `0`), clobbering `interface_type` and never setting `interface_version`. The header wants the
  reverse: *"Frontend looks at `retro_hw_render_interface_type` and returns the maximum supported
  context negotiation interface version. If the type is not supported … a version of 0 must be
  returned in `interface_version` and `true` is returned"* (`libretro.h:2434-2438`).
- **`SET_HW_SHARED_CONTEXT`** is `// TODO: ?` followed by `return true`.

`getPreferredHwRender()` should name the API with the best path for the current backend and return
`true` only once others genuinely work; every other handler should refuse what it cannot serve.

### Negotiation version, and why it matters now

The vendored header is negotiation **v2** (`RETRO_HW_RENDER_CONTEXT_NEGOTIATION_INTERFACE_VULKAN_VERSION 2`),
where `create_device2` *"takes precedence over `create_device`"* and v1 `create_device` is deprecated
because *"it cannot express PDF2 features or optional extensions"*. `emulator_vulkan_renderer.cpp:280`
calls `create_device` with no null guard and no version check; `create_device2` and `create_instance`
appear nowhere in the tree. Since the support query above never reports a version, a v2 core may fill
only `create_device2` and leave `create_device` null.

Two further obligations the header places on a frontend that does implement v2: it *"must not reject a
negotiation interface version that is larger than what the frontend supports"* — downgrade to the
entry points you recognise instead — and `destroy_device` is *"called even if `context_reset` was not
called"*, always before the frontend's `VkInstance` goes.

### Do not mask `RETRO_ENVIRONMENT_EXPERIMENTAL`

`libretro.h:722` advises *"Frontends should mask out this bit before handling the environment call."*
Taking that advice would break things: `SET_HW_SHARED_CONTEXT` is `(44 | EXPERIMENTAL)` and
`SET_SERIALIZATION_QUIRKS` is a plain `44`, so masking collides them. They are the only such pair.
`Core::environmentCallback` dispatches on the full value through `m_envHandlers.find(cmd)` and never
masks, which is correct — leave it that way.

### OpenGL, when it is wired up

The GL path worked before the Vulkan work and was taken apart during it; most of the graphics
callbacks are known to be disconnected rather than newly broken. What follows is what the headers
require, as a checklist for reconnecting it.

A GL core gets `get_proc_address` and `get_current_framebuffer`, both
marked *"Set by frontend"* (`libretro.h:5110-5117`); the frontend creates and owns the context. The
context need not be Qt's, and should not be — borrowing Qt's is what makes GL look like it forces the
core onto the render thread. Create a `QOpenGLContext`, `moveToThread()` it to the emulation thread
while it is current nowhere, and make it current against a `QOffscreenSurface` created on the GUI
thread. Under a Vulkan or D3D Qt backend there is no Qt GL context to share with anyway.

`QRhi::makeThreadLocalNativeContextCurrent()` is not the route. It makes Qt's own context current, and
from a third thread `QOpenGLContext::makeCurrent()` calls `qFatal` rather than warning.

Two header details worth having:

- `get_current_framebuffer` carries *"TODO: This is rather obsolete. The frontend should not be
  providing preallocated framebuffers"*, but cores still call it, and the typedef notes it *"could
  change every frame potentially"*. Handing back an FBO the frontend owns is stable, which is simpler
  than the arrangement where `m_currentFramebufferId` is sampled once inside `context_reset` and goes
  stale as Qt rotates framebuffers.
- `context_reset` *"is possible … called multiple times during an application lifecycle"*, and when it
  is called without a preceding `context_destroy` the core's resources are already gone and must be
  recreated rather than freed (`libretro.h:5095-5107`). Whatever owns the context has to survive that.

`depth`, `stencil` and `bottom_left_origin` are all still in the struct and all marked obsolete;
`bottom_left_origin` is the one that still matters, since it decides whether the frame needs flipping.

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
- **Metal as a core-facing API.** `retro_hw_context_type` is NONE, OPENGL, OPENGLES2, OPENGL_CORE,
  OPENGLES3, OPENGLES_VERSION, VULKAN, D3D11, D3D10, D3D12, D3D9 — there is no Metal. A core can never
  request it and we can never advertise it. This does not block Qt on Metal, since the core's API is
  independent of Qt's; it means the core-facing set is GL, GLES, Vulkan and D3D, and nothing else.
- **Assuming a refused core retries.** The headers are silent on whether `SET_HW_RENDER` may be called
  more than once — no permission, no prohibition, no described frontend behaviour. All
  `GET_PREFERRED_HW_RENDER` promises is that a core which cannot use the preferred API *"should exit
  or fall back to software rendering"*. Any belief about a second attempt is RetroArch convention.

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

Items 2 to 5 are the pacing fix and **do not depend on item 6**. With the loop asking for exactly one
frame per period and dropping rather than doubling when a pass is late, the paired-pass fault goes
away even with the core still running inside `render()`. Moving the core buys immunity to GUI-thread
stalls and removes the dependency on Qt scheduling a pass; it is not required for correctness, and it
can be skipped indefinitely for GL cores if their context stays on the render thread.

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
8. **Make the hardware-render handlers answer honestly.** Independent of the pacing work and a
   prerequisite for any backend beyond Vulkan: refuse in `SET_HW_RENDER` what cannot be provided,
   return the real flexibility from `GET_PREFERRED_HW_RENDER`, and stop returning `true` from
   `GET_HW_RENDER_INTERFACE` and `GET_CURRENT_SOFTWARE_FRAMEBUFFER` after writing nothing.
9. **Later, and not standalone: the blit fence.** Replacing the unconditional `WaitForFences` with
   `setQueueSubmitParams()` also requires real sync-index rotation. Today `get_sync_index` returns 0,
   `get_sync_index_mask` returns 1 and `wait_sync_index` is a no-op, which is only correct *because*
   that fence makes the frontend fully synchronous. Remove it without implementing those and the core
   is told the frontend has finished with an image while it is still being read — corruption, not a
   missed optimisation. `set_command_buffers` being null is fine and stays fine: the header makes it
   optional and cores must null-check it.
10. **Later:** DRC gain 0.02 to 0.005.

## Open

- Does `QScreen::refreshRate()` round on real panels?
- How often do SNES cores send `SET_GEOMETRY`?
- `layer.enabled: true` on the EmulatorItem inserts an extra FBO pass before the swapchain.
- Which backend/core-API pairs are worth a zero-copy path, and which are fine on readback. The
  extension names above are unverified against any driver.
- What real cores do when refused, given the protocol does not define it (see "Ruled out"). Worth
  testing against mupen64plus-next and PPSSPP before relying on any fallback behaviour.
- `k = round()` at 144 Hz gives 72 fps, 20 % fast. `k = 3` would give 48 fps, 20 % slow. Both are the
  same distance out, but fast and slow do not play the same.

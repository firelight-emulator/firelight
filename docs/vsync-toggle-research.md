# Implementing a vsync toggle

Research into what it would take to expose vsync as a user-facing setting. Qt source references are
against 6.11, the version `CMakeLists.txt:88` requires.

## 1. Where vsync is decided today

`src/main.cpp:212-221` sets the swap interval on the default surface format from `FL_VSYNC`:

```cpp
QSurfaceFormat format;
format.setSwapInterval(qEnvironmentVariableIntValue("FL_VSYNC"));
QSurfaceFormat::setDefaultFormat(format);
```

Unset means 0, so the shipped default is "present as soon as a frame is ready". That value reaches
the GPU through exactly one path. In `QSGRenderThread::ensureRhi()`
(`qsgthreadedrenderloop.cpp:911-950`):

```cpp
if (rhi && !cd->swapchain) {
    QRhiSwapChain::Flags flags = QRhiSwapChain::UsedAsTransferSource;
    const QSurfaceFormat requestedFormat = window->requestedFormat();
    ...
    if (requestedFormat.swapInterval() == 0) {
        qCDebug(QSG_LOG_INFO, "Swap interval is 0, attempting to disable vsync when presenting.");
        flags |= QRhiSwapChain::NoVSync;
    }
    cd->swapchain = rhi->newSwapChain();
    ...
    cd->swapchain->setFlags(flags);
}
```

The basic (non-threaded) loop does the same at `qsgrenderloop.cpp:520-540`. Three things follow:

- The flag is computed **once per swapchain**, guarded by `!cd->swapchain`. Nothing re-reads the
  format afterwards.
- It is read from `requestedFormat()`, not `format()` — the value the app asked for.
- `QWindow::setFormat()` is a pure setter (`qwindow.cpp:938-942`: `d->requestedFormat = format;`,
  no side effects, no re-creation). So changing it at runtime is legal and inert — it only matters
  the next time a swapchain is built.

That is the whole mechanism. There is no `setVSync()` anywhere in Qt, which is why the toggle looks
impossible: the knob exists, but it is only sampled at swapchain creation.

## 2. What `NoVSync` means per backend

Relevant because a live toggle is only as good as the backend's response to it, and Firelight can
run on D3D11 (the Windows default) or Vulkan (`src/main.cpp:446`, currently commented out, plus the
hardware-core path in `src/app/emulator_vulkan_renderer.cpp`).

| Backend | Handling | Where |
|---|---|---|
| D3D11 | `swapInterval = NoVSync ? 0 : 1`, and `DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING` when 0 and supported. Also disables the frame-latency waitable object. | `qrhid3d11.cpp:5335-5352` |
| D3D12 | Same. | `qrhid3d12.cpp:6944-6952` |
| Vulkan | `VK_PRESENT_MODE_MAILBOX_KHR`, else `IMMEDIATE`, else `FIFO`. | `qrhivulkan.cpp:2318-2327` |
| Metal | `CAMetalLayer.displaySyncEnabled = false`. | `qrhimetal.mm:6549` |
| OpenGL | **Ignored.** `NoVSync` does not appear anywhere in `qrhigles2.cpp`. | — |

All of these are evaluated inside `createOrResize()`, not at construction. For D3D11 the resize path
passes the recomputed flags through: `swapChain->ResizeBuffers(..., colorFormat, swapChainFlags)`
(`qrhid3d11.cpp:5499`). That is what makes option C below work.

The OpenGL row is the exception that matters: there the interval comes from the format the
`QOpenGLContext` was created with, so nothing short of recreating the context changes it. If GL ever
has to be supported, the live options need a `wglSwapIntervalEXT`/`glXSwapIntervalEXT` call on the
render thread as a fourth case.

## 3. Three ways to implement it

### A. Restart-scoped setting — recommended first step

Replace the env var with a persisted setting and keep everything else as it is. The only constraint
is ordering: the default format has to be set before the `QQuickWindow` is constructed, i.e. before
`engine.load()` at `src/main.cpp:756` — *not* necessarily before `QApplication`. The settings
service already exists by `src/main.cpp:399-403`, so the block moves down:

```cpp
QSurfaceFormat format;
const auto wantsVsync = settingsService.getGlobalValue("wait-for-display").value_or("false") == "true";
format.setSwapInterval(qEnvironmentVariableIsSet("FL_VSYNC") ? qEnvironmentVariableIntValue("FL_VSYNC")
                                                             : (wantsVsync ? 1 : 0));
QSurfaceFormat::setDefaultFormat(format);
```

`FL_VSYNC` survives as a developer override. Zero risk, and `presentationLocked`
(`src/app/emulator_item.cpp:410`) picks it up unchanged, because it already reads
`requestedFormat().swapInterval()`. The UI has to say "takes effect on restart".

### B. Live toggle by recreating the window

`QWindow::destroy()` tears down the native surface, which fires
`QPlatformSurfaceEvent::SurfaceAboutToBeDestroyed`; the render loop's event filter
(`qsgthreadedrenderloop.cpp:1377-1400`) catches it and posts `WMReleaseSwapchainEvent`. The
comment there is explicit that this is a supported sequence:

> keep this filter on the window - needed for uncommon but valid sequences of calls like
> `window->destroy(); window->show();`

`releaseSwapchain()` (`:1209-1219`) deletes only the swapchain, its render-pass descriptor and its
depth/stencil buffer. The `QRhi` and the scene graph survive, because `persistentGraphics` and
`persistentSceneGraph` both default to true (`qquickwindow.cpp:720`). So:

```cpp
auto format = window->requestedFormat();
format.setSwapInterval(on ? 1 : 0);
window->setFormat(format);

const auto geometry = window->geometry();
const auto visibility = window->visibility();
window->destroy();
window->setVisibility(visibility);
window->setGeometry(geometry);
```

Re-exposing runs `handleExposure()`, which builds a fresh `Window` record and re-reads
`win.actualWindowFormat = window->format()` (`:1271`) — the only path that keeps Qt's own
animation-driver decision honest (see §5). Costs: the HWND changes, so there is a visible flicker,
window state has to be restored by hand, and anything holding the native handle (the Steam overlay
hooks at `src/app/emulator_vulkan_renderer.cpp:197-236`) sees a new window. Worth testing with a
game running and with a hardware Vulkan core.

### C. Live toggle by rebuilding the swapchain in place

`QQuickWindow::swapChain()` is public (`qquickwindow.h:168`), `QRhiSwapChain::setFlags()` is public,
and every backend recomputes its present behaviour inside `createOrResize()`. So the surgical
version is:

```cpp
connect(window, &QQuickWindow::afterFrameEnd, this, [this, window] {
    if (!m_vsyncChangePending.exchange(false)) {
        return;
    }

    auto *swapchain = window->swapChain();
    auto flags = swapchain->flags();
    flags.setFlag(QRhiSwapChain::NoVSync, !m_vsyncWanted);
    swapchain->setFlags(flags);
    swapchain->createOrResize();
}, Qt::DirectConnection);
```

`afterFrameEnd` is emitted on the render thread after `rhi->endFrame()`
(`qsgthreadedrenderloop.cpp:773-803`), which is the only point where calling `createOrResize()` is
not inside a frame. Do not do this from `beforeRendering` — that runs after `beginFrame()`.

Keep `window->setFormat()` in sync anyway, so `requestedFormat()` still describes reality: it is what
`presentationLocked` reads, and what Qt would use if the swapchain is ever rebuilt after an
unexpose (minimize/restore).

No flicker, no lost window state, all public API. Two caveats: OpenGL does nothing (§2), and
`actualWindowFormat` goes stale (§5).

## 4. What it does to pacing

The pacing code is already built for both states and needs no changes beyond being told to re-run.

- `presentationLocked` is derived from the swap interval (`src/app/emulator_item.cpp:407-410`).
- The rate controller degrades on its own: an explicit `Display` mode with
  `presentationLocked == false` falls back to a clock at the display-locked rate rather than
  counting presents (`src/app/emulation/emulation_rate_controller.cpp:123-129`), and `Auto` never
  picks `Display` without it (`:97-104`).
- `m_renderContinuously` — the render-every-present loop at `src/app/emulator_item.cpp:106-116`,
  which is only safe while presentation cannot free-run — is set from the resolved mode
  (`:466-467`), so it clears itself when vsync goes off.

The one requirement is to call `reconfigurePacing()` after the flip lands. It is already a slot
invoked from the settings subscription at `src/app/emulator_item.cpp:138-146`, so adding the new key
there is enough — but for options B and C the reconfigure must happen **after** the new swapchain
exists, otherwise it reads the old state. Queue it from the same place that does the rebuild.

Turning vsync off while `Display` mode is resolved is the ordering that matters: reconfigure
promptly so the continuous-update loop stops, or the item keeps requesting a render per present
against a free-running presenter.

## 5. The gotcha: Qt's animation driver

`QSGThreadedRenderLoop` decides how to advance QML animations from `actualWindowFormat`, captured
once per window at expose (`:1271`) and never refreshed:

```cpp
if (w.actualWindowFormat.swapInterval() == 0)
    ++unthrottledWindows;
...
const bool canUseVSyncBasedAnimation = exposedWindows == 1 && unthrottledWindows == 0 && badVSync == 0;
```

(`:1095`, `:1122`.) With option C this record is stale after a toggle:

- **on → off**: Qt still believes it is throttled and advances animations by a fixed vsync step
  while frames now render unthrottled. It self-corrects — the `badVSync` detector at `:1577-1600`
  averages 20 frames, notices the elapsed time is under half the expected value, and switches to the
  system timer — but there is a visible transient where UI animations run fast.
- **off → on**: `unthrottledWindows` stays 1 forever, so animations remain on the system timer. Less
  harmful, and it is the state the app ships in today.

Option B avoids both, because the expose rebuilds the record. If option C is chosen and the
transient is unacceptable, forcing an unexpose/expose cycle afterwards defeats the point — at which
point it is option B with extra steps.

## 6. Recommendation

**Ship A now, add C behind it if a live toggle is actually wanted.**

A is a half-hour change that removes an undiscoverable env var and costs nothing. It also answers the
question the setting is really asked for — the user picks tearing-vs-latency once and restarts —
without touching the render thread.

C is the better live experience and is entirely public API, but it is render-thread work with a
per-backend matrix and a known animation-driver transient. B works everywhere and is easier to
reason about, but recreating the window under a running game is the more user-visible of the two.

Note that the setting is not independent of `sync-method` (`data/settings/40-emulation.json:100-130`):
vsync off makes "Sync to monitor" unachievable, and the code silently substitutes a clock at the same
rate. The description should say so, or the toggle should live next to it in the `emulation-video`
group with `vsync`/`tearing`/`latency` keywords so the existing settings search finds it — `vsync`
and `tearing` currently point at `sync-method`.

## 7. Verification

```bash
cmake --preset debug-win && cmake --build build/debug-win
QT_QPA_PLATFORM=offscreen ./build/debug-win/firelight.exe verify-ui
```

Then by hand, because none of this is covered by tests:

- `QSG_INFO=1` prints `Swap interval is 0, attempting to disable vsync when presenting.` at swapchain
  creation — the cheapest confirmation that the flag reached Qt, and for options B/C that it was
  re-evaluated.
- The existing pacing log line (`src/app/emulator_item.cpp:479-484`) already prints
  `presentation waits for the display` / `does not wait`; toggling should flip it and change the
  resolved mode.
- Toggle both directions with a game running, in windowed and borderless fullscreen, watching for
  tearing on a fast-scrolling game.
- Toggle with `sync-method` set to `monitor` explicitly, and confirm the fallback to a clock is
  audible-free.
- Watch a QML animation (any spinner) for a second after each toggle for the §5 transient.
- Repeat on a hardware Vulkan core, which is the backend most likely to reject an in-place swapchain
  recreate.

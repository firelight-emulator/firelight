# Porting Firelight to another language / GUI framework

Research report. Evaluates what it would cost to move Firelight off Qt6/QML, judged against three
priorities: **high performance, low input lag, and game output composited as part of the scene
graph** — without losing functionality.

**Conclusion up front: don't port. The specific pain that motivates porting is not caused by Qt
being the UI toolkit — it is caused by Qt owning the Vulkan device. That is fixable in place, with
a documented Qt API, for roughly 1% of the cost of any port.** Details in §3 and §6.

---

## 1. What is actually on the table

Measured, not estimated:

| Area | Lines | Files |
|---|---|---|
| `qml/` | 25,164 | 182 `.qml` |
| `src/` C++ | 23,358 | 205 |
| `libs/firelight/` C++ (excl. tests) | 23,667 | — |
| **First-party total to re-home** | **~72,000** | — |

That is the floor. It excludes tests (12,845 lines under `tests/`), vendored code, and the fact
that a port also re-opens every bug already fixed once.

The UI is not a thin shell over the emulator. It is the larger half of the product:

- **14 routes** with a custom router (`qml/routing.js`, `qml/Router.qml`,
  `qml/components/v2/RouteView.qml`) that does LRU caching of 5 live screens plus **asynchronous
  incubation** — frame-sliced object construction so opening a heavy page doesn't stall.
- **24 `QAbstractListModel` subclasses**, 63 `QObject`s, 175 `Q_PROPERTY`, 127 `Q_INVOKABLE`.
- **A catalog-driven settings form generator**: 12 widget types dispatched from
  `data/settings_catalog.json` by `qml/components/settings/SettingsGroup.qml`. Adding a setting is
  a JSON edit. Any port must reproduce the generator, not just the widgets.
- **A frameless custom-chrome window** (`qml/components/v2/MainWindow.qml`) with five hand-built
  resize edges, native position tracking, custom maximize, persisted geometry.
- **Gamepad focus navigation** implemented by synthesizing real key events
  (`src/gui/qt_input_service_proxy.cpp:54-108`) with hand-rolled 60 Hz auto-repeat, plus
  `FLFocusHighlight.qml` — a focus ring that crossfades by reparenting and auto-scrolls its target
  into view. `FocusScope` appears in 49 files.
- **A blur/glass theming engine** (`qml/Theme.qml`) deriving two parallel token families from one
  accent color, with image-sampled tinting, composited via `MultiEffect` at 13 sites.
- **`GameplayLayer.qml`** — a five-state machine that animates the *live game surface* between
  fullscreen and a 72px "now playing" bar.

Two things in the current codebase materially reduce port cost and are worth knowing:

- **There are zero shaders.** No `.qsb`, `.frag`, `.vert`, no `ShaderEffect`, no CRT filters. Nothing
  to port — but also nothing Qt is doing for you that you couldn't do yourself.
- **The domain layer is already substantially Qt-free.** 7 of 14 `libs/firelight/*` modules link no
  Qt at all; `settings` and `saves` are Qt-free *by explicit design* with `AUTOMOC OFF` and READMEs
  explaining why. A hand-rolled `EventDispatcher` is used in 58 files, already having replaced Qt
  signals. `include/firelight/image.hpp` is a deliberately Qt-free PNG blob with a conversion
  boundary at `src/gui/image_qt.hpp`. This is a deliberate, in-progress decoupling and it is
  working.

## 2. Licensing is not a reason to port

Firelight is GPL-3.0 (`LICENSE.md`). Qt's LGPLv3 and GPLv3 options are both compatible. There is no
license pressure pushing off Qt. Worth stating because it's the usual unspoken motive.

## 3. The real constraint: who owns the graphics device

This is the finding that reframes the whole question.

Today `src/main.cpp:417` hardcodes `QQuickWindow::setGraphicsApi(Vulkan)`, and Qt creates the
`VkInstance`, `VkPhysicalDevice`, and `VkDevice`. A libretro core that wants hardware rendering
therefore has to create a **second `VkDevice`** on Qt's physical device via the negotiation
interface, and every frame has to be handed across the device boundary. What that costs today, all
in `src/app/emulator_vulkan_renderer.cpp`:

- ~750 lines of interop, the gnarliest code in the repo.
- Reaching into `vulkan-1.dll` with `GetModuleHandleA` to bypass Qt's layer-chained
  `vkGetInstanceProcAddr`, because Steam/OBS overlays return null for a second instance (`:197-206`).
- Setting `VK_LOADER_LAYERS_DISABLE=~implicit~` around `create_device` so overlay layers don't crash
  (`:229-236`).
- Manual resolution of ~35 device function pointers (`:304-341`).
- An exportable **Win32 timeline semaphore** created on the core device and imported into Qt's
  (`:348-471`).
- A `VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT` shared image, exported and re-imported, plus a
  `vkCmdBlitImage` core→shared every frame (`ensureSharedImage`, `:616-753`).
- A hard `vkWaitForFences` CPU block on the render thread every frame (`:172`).
- **`#ifdef _WIN32` only** — `ensureSharedImage` returns `false` elsewhere (`:750-752`), so hardware
  cores do not work on Linux or macOS at all.

**None of that is inherent to Qt Quick. All of it exists because Qt created the device first.**

Qt has a documented API for the other direction:

```cpp
// Qt 6.0+, verified against Qt 6.11 docs
QQuickGraphicsDevice::fromDeviceObjects(VkPhysicalDevice, VkDevice, int queueFamilyIndex, int queueIndex = 0);
// -> QQuickWindow::setGraphicsDevice(...)
```

If Firelight creates the Vulkan device and hands it to Qt, the core and the scene graph are on the
**same `VkDevice`**. Then: no second device, no negotiation gymnastics, no external memory, no
timeline semaphore, no per-frame blit, no `_WIN32` guard. `emulator_vulkan_renderer.cpp` largely
deletes itself, and hardware cores start working on Linux and macOS. `QQuickRenderControl` +
`QQuickGraphicsDevice::fromRhi()` goes further and hands you the swapchain and present timing too,
which is where input latency actually lives.

Every alternative framework surveyed below is being evaluated, in effect, on whether it lets you own
the device. Qt already does.

## 4. Candidate comparison

| Option | Language | Game in scene graph | You own the device | UI parity with today | Cost | Verdict |
|---|---|---|---|---|---|---|
| **Qt Quick, device inverted** | C++ (unchanged) | Yes (already) | **Yes** | 100% | ~1–2 kLOC | **Recommended** |
| Qt Quick + own swapchain (`QQuickRenderControl`) | C++ (unchanged) | Yes | Yes, incl. present | 100% | ~3–5 kLOC | Follow-on, if latency demands |
| Bespoke Vulkan + Dear ImGui | C++ | Native — you present it | Yes | **~40%** | ~40 kLOC UI rewrite | Only for the in-game overlay |
| Rust + Slint (wgpu) | Rust or C++ | Yes, via `Image::try_from(wgpu::Texture)` | Shared wgpu device | ~60% | 70 kLOC+ | No |
| Rust + wgpu + egui / iced | Rust | Yes | Yes | ~40% | Full rewrite | No |
| C# + Avalonia | C# + C++ host | Yes, `ICompositionGpuInterop` | Interop'd | ~85% | Full rewrite + FFI | No |
| Godot (GDExtension) | C++/GDScript | Yes | Engine owns it | ~70% | Full rewrite | No |
| Flutter | Dart + C++ | **Vulkan external textures unimplemented** | No | ~80% | Full rewrite | Disqualified |
| GTK4 | C/C++ | Yes (`GtkGLArea`/dmabuf) | Partly | ~70% | Full rewrite | No |
| Web / Electron / Tauri | TS | Compositor-mediated | No | ~90% | Full rewrite | Disqualified on latency |

## 5. Notes on the serious candidates

**Dear ImGui + your own Vulkan renderer** is the only option that beats Qt on the stated priorities
in isolation. You own the device and the swapchain, you present the core image directly with zero
sharing, and the UI is drawn into your own frame — this is what RetroArch and PCSX2 overlays do, and
latency is as low as physically achievable. But immediate mode means every piece of retained state
in §1 is hand-rolled: virtualized grids over thousands of games with async art loading, the LRU page
cache, drag-and-drop, animated transitions, the focus ring, Markdown, variable-font icons. Text
shaping and accessibility are explicitly not supported. Upstream is candid that it targets tools,
not end-user UI. **The right conclusion is a hybrid: keep Qt Quick for the frontend, and consider
ImGui only for the in-game quick menu** (`QuickMenu.qml`, 930 lines) if that surface ever needs to
be latency-critical. Note that today the quick menu is not obviously the bottleneck.

**Slint** is the most credible declarative alternative and has a real C++ API, so you could keep the
47k lines of C++. Since 1.12 it can embed externally-produced textures via
`slint::Image::try_from(wgpu::Texture)`. Two problems. First, it requires sharing Slint's `wgpu`
device — so you get wgpu between you and Vulkan, and libretro Vulkan cores want a raw `VkDevice`;
extracting one through `wgpu-hal` is possible but is a worse version of the interop you're trying to
escape. Second, the widget set is far thinner than 25k lines of custom QML: no `MultiEffect`
equivalent, no async incubation, weaker rich text. You would lose functionality, which was the
stated constraint.

**Avalonia** has the strongest precedent — Ryujinx moved GTK→Avalonia in 2024 — and
`ICompositionGpuInterop` genuinely supports importing external GPU textures. But the emulator side
cannot move to C#: the libretro C ABI, per-frame `video_refresh`/`audio_sample` callbacks, and
savestate serialization would all cross a P/Invoke boundary, and a GC with a 16.6 ms frame budget is
precisely the wrong shape for the software frame pacer at `src/app/emulator_item.cpp:86-200`. You'd
end up maintaining a C++ core host *and* a C# UI *and* the interop between them — strictly more
complexity than today, for a worse latency story.

**Flutter is disqualified on facts, not taste.** External texture support is not implemented for
Vulkan in the embedder, and the Windows desktop path is pixel-buffer only — meaning GPU→CPU→GPU per
frame. That is the single thing this application most needs to avoid.

## 6. Recommendation

**Do not port. Invert device ownership instead, and fix the three specific defects that are being
mistaken for framework problems.**

In priority order:

1. **Create the Vulkan device yourself; hand it to Qt** via
   `QQuickGraphicsDevice::fromDeviceObjects()` before the window initializes. Deletes most of
   `src/app/emulator_vulkan_renderer.cpp`, removes the `_WIN32` guard, and unblocks hardware cores on
   Linux and macOS. This is the highest-value change available anywhere in this document.
2. **Fix the readback heap corruption.** `src/app/emulator_item_renderer.cpp:219-240` constructs a
   `QImage` from `QRhiReadbackResult::data` **without passing `bytesPerLine`**, so it assumes
   `width * 4` stride. If the backend pads rows, `.copy()` reads past the end of the buffer. That
   matches the open `c0000374` render-thread heap corruption recorded in project memory. The same
   block also uses a self-deleting lambda capturing its own `new`'d pointer, which leaks on device
   loss and double-frees if it ever fires twice.
3. **Stop converting every software frame on the CPU.** `emulator_item_renderer.cpp:189-213`
   allocates and runs `convertToFormat` to RGBA8888 every frame. Upload the native RGB565/XRGB8888
   and convert on the GPU.
4. **Consider `QQuickRenderControl`** only if, after 1–3, measured input latency still isn't good
   enough. It gives you the swapchain and present timing, which is where the remaining latency is.
   Note `QSurfaceFormat::setSwapInterval(0)` is already set (`src/main.cpp:203-205`) and pacing is
   already fully software (`emulator_item.cpp:86-200`, spin loop at `:137-152`), so much of the
   latency win is already banked.

Cheap Qt cleanups that reduce future optionality cost, in case a port is ever revisited:

- Drop `Quick3D` (linked at `CMakeLists.txt:334`, **zero usage**) and `Svg` (found, never linked).
- Retire `QSettings` (21 sites) in favor of the existing Qt-free SQLite settings service — you
  currently run two parallel settings systems.
- Move `QImage`/`QString` out of `libs/firelight/media`'s **public headers**, which currently force
  `Qt6::Gui` on every consumer.
- Move `ra_client.hpp` — a full `QObject` with 12 `Q_PROPERTY` living inside a domain lib — behind
  the same kind of boundary the rest of `libs/` already uses.
- Migrate the 7 `QJsonDocument` sites to nlohmann, which is already used in 33 files.

Each of these is independently worth doing, and together they mean that if the answer ever changes,
the domain layer walks away intact.

## 7. Verification

For the device-inversion work (step 1), the existing tooling already covers it:

```bash
cmake --preset debug-win && cmake --build build/debug-win
cd build/debug-win && ctest --output-on-failure
QT_QPA_PLATFORM=offscreen ./build/debug-win/firelight.exe verify-ui   # all 22 screens still mount
```

Then, by hand, because none of this is caught by tests:

- A **software** core (frame upload path) and a **hardware Vulkan** core — PPSSPP is the strict case,
  since it requests Vulkan 1.0 and resolves external memory through KHR entry points.
- Screenshot capture, instant-replay clip capture, and a netplay guest stream — all three share the
  readback path being changed in step 2.
- Rewind, which PNG-encodes a thumbnail every 3 s on the render thread.
- Run under the Steam overlay, which is the specific scenario the loader hacks at
  `emulator_vulkan_renderer.cpp:197-236` exist to survive.
- Audio by ear — pacing changes surface as stutter, not as test failures.

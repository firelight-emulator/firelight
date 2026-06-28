# Firelight Android Port — Gap Analysis

> Status: Engineering assessment / planning document. Author: lead engineer synthesis from per-subsystem code audit, web research, and adversarial verification.
> Verdict-corrected: where adversarial verification qualified or refuted a research claim, this document follows the **verified, more cautious** position and flags it inline.
> **Version/policy drift caveat:** the Android-ecosystem floors cited in Section 2 (Play target-SDK floor, NDK page-size mandate, Qt JDK/NDK requirements, dated Play-policy changes) move every few months. They are stated as best-known-at-authoring, not as settled fact. **Re-verify every version/policy floor against current Qt release notes and Google Play policy at port kickoff.**

---

## 1. Executive Summary

**Feasibility verdict: Feasible, but a large, multi-quarter effort — and the verdict is *conditional* on one unproven prototype (see below) — not a weekend port.** Nothing in Firelight's architecture is fundamentally incompatible with Android. Qt 6.8+ supports every Qt module the app uses on Android (including QtWidgets/QApplication), every libretro core Firelight ships has an official prebuilt arm64-v8a Android binary, and the libretro dynamic-core-loading model is viable on Android. The work is real but mostly bounded — **with one critical exception.**

**Conditional-feasibility flag (load-bearing).** The single highest-risk item is the **Qt-Quick + libretro `RETRO_HW_RENDER` GLES handshake** required for the two HW-3D cores (N64/PSP). No shipping Android libretro frontend uses Qt Quick for the emulator surface — this path is an *untested hypothesis*, and it is the single most likely place the port stalls. The "feasible/bounded" verdict therefore applies firmly to the **software-core MVP**; full parity (N64/PSP) is feasible *only if* the Phase 3 HW-render prototype succeeds. **If that prototype proves unworkable, N64/PSP may require abandoning Qt Quick for the emulator surface entirely** (a native `SurfaceView`/`GLSurfaceView` composited with the QML UI) or shipping those cores via the existing Vulkan scaffolding — a scope change the roadmap treats as an explicit fallback (Section 8, Phase 3) rather than a baseline.

**The biggest blockers — listed here by *risk* (Section 9 lists the same work by *critical-path sequence*; note build bring-up is sequenced first there even though graphics is riskiest):**

1. **Graphics / GLES renderer rewrite.** `src/app/emulator_item_renderer.cpp` is written for *desktop* OpenGL and requests `RETRO_HW_CONTEXT_OPENGL`. Android only has OpenGL ES. The software cores need a GLES texture-upload path; the two HW-3D cores (mupen64plus_next, ppsspp) need a working `RETRO_HW_CONTEXT_OPENGLES3` context. **Verification caveat:** no shipping Android libretro frontend uses Qt Quick — RetroArch uses raw EGL, Lemuroid uses a native `GLSurfaceView`. The Qt-Quick-render-thread-as-HW-render-driver path is an *untested hypothesis*, not an established pattern. This is the single most likely place the port stalls (see conditional-feasibility flag above).
2. **Input on Android.** SDL2's `SDL_GameController` does **not** "just poll" controllers when Qt owns the Activity; Android delivers controller input only to the focused View/Activity via callbacks, with no background polling API, and SDL's Android joystick backend is fed entirely from `SDLActivity` JNI callbacks. With Qt owning the Activity, SDL sees nothing unless events are explicitly bridged into SDL's JNI layer. The robust answer is a **native Android input backend** behind Firelight's existing `IInputService`/`IGamepad` abstraction (which inherently also resolves the SDL-on-background-thread vs. UI-thread-dispatch mismatch — see 5.4), plus an on-screen touch gamepad. Note: **a `RETRO_DEVICE_POINTER` provider path already exists** (`core.cpp` lines 66–81, wired through `IPointerInputProvider`) and can be reused for touchscreen pointer input; what is missing is touch-driven *gamepad* input and a QML touch overlay.
3. **Core packaging + native-build blockers.** The shipped cores are x86_64 `.dll`/`.so` — useless on arm64. Prebuilt Android arm64 cores exist on the libretro buildbot but ship as `.so.zip` and must be inflated. Hard compile blockers: `core.cpp` includes `<bits/fs_path.h>` (line 6) and `sdl_controller.cpp` includes `<bits/stl_algo.h>` (line 3) — both GCC/libstdc++-internal, absent in the NDK's libc++.
4. **Storage / scoped storage.** Firelight's desktop "Documents/Firelight/roms" model breaks on Android 11+. App-private dirs (no permission) host DBs/saves/BIOS; user ROM folders need the Storage Access Framework (SAF). Critically, `need_fullpath` cores (mupen64plus_next, ppsspp) cannot `fopen()` a `content://` URI — those ROMs must be materialized to a real app-private path first. **Open code question:** `core.cpp` already passes only a bare filename as `info.path` for *every* core (loadGame line 1201; `GET_GAME_INFO_EXT` line 896), so how N64/PSP load on desktop today is unaudited and must be confirmed (see 5.6).
5. **Build/toolchain + distribution constraints.** No Android CMake preset, no Gradle/manifest, stale `android-toolchain.cmake`. Build bring-up **gates everything** (it is the first critical-path item in Section 9). Plus Google Play's 16 KB page-size mandate (NDK r28+), the current API-target floor, and emulator-specific Play policies (no bundled BIOS, no runtime core downloads, SAF over `MANAGE_EXTERNAL_STORAGE`). Note `src/later/` **is** compiled into the main library today (see 5.1), so excluding it is real work, not a no-op.

**Rough total effort: XL — roughly 5–9 engineer-months for one experienced engineer (≈22–38 engineer-weeks across workstreams; see Section 9 for per-workstream week ranges).** Build bring-up and the software-core MVP are the smaller part; the GLES renderer, native input + touch overlay, and SAF storage layer are the bulk. **The only hard dollar cost is commercial Qt licensing** (a per-developer annual fee, required for static linking / safer for Play distribution) plus the one-time Google Play developer-registration fee; everything else is engineering time.

**Recommended strategy (one paragraph):** Adopt the official `qt-cmake` Android workflow (discard the hand-rolled `android-toolchain.cmake`), fix the NDK compile blockers, and bring up a **software-core MVP** first: bundle the software-rendered cores as prebuilt arm64 `.so` files, present their framebuffers via a GLES texture-upload path, wire a native Android input backend plus a QML touch-gamepad overlay, and route storage through app-private dirs + SAF. **Defer** the two HW-3D cores (N64/PSP) to a later phase gated on a working GLES `RETRO_HW_RENDER` context, with a documented fallback if that prototype fails. Compile out the Discord SDK on Android. Treat distribution (Play vs sideload), licensing (LGPL/commercial Qt + non-commercial cores), and the GLES renderer as the three decisions that most shape the timeline.

---

## 2. Scope & Target Assumptions

> **These floors drift.** Treat every version/policy number below as "as of the target SDK / Qt release in effect at authoring." Re-verify at kickoff — the Play target-SDK floor in particular rises annually (it was API 35 for 2025 submissions and is expected to move to API 36 for 2026).

| Dimension | Assumption |
|---|---|
| **Primary ABI** | `arm64-v8a` (the only ABI that matters for real devices). `x86_64` optionally for the emulator only. |
| **Min API level** | **API 28+** is the practical floor (Qt 6.8 requires min API 28 / Android 9). For Google Play, target the **then-current target-SDK floor** (API 35 for 2025 submissions; verify the current floor at kickoff — it rises yearly). The repo's `android-toolchain.cmake` says API 21 — obsolete; must be raised. |
| **NDK** | Qt 6.8 recommends NDK r26b/r27c, but Google Play's 16 KB page-size mandate (phasing in for apps targeting recent API levels) forces **NDK r28+** for all native libs. Expect to validate Qt against a newer NDK or move to a later Qt build. Re-verify the page-size deadline against current Play policy. |
| **Toolchain floors (Qt 6.8)** | JDK 17, Gradle 8.10, AGP 8.6.0. These drift every Qt minor release (later Qt builds raise the JDK requirement — e.g. 6.11 expects JDK 21) — pin exactly in CI and confirm against the Qt version you actually ship. |
| **Distribution** | Tiered. Sideload APK = most flexible (can download cores, can use broad storage). Google Play = constrained (cores bundled in-APK, SAF, no bundled BIOS). F-Droid = only if all components are FLOSS (Discord SDK must be stripped). |
| **MVP definition** | One or two software-rendered cores (e.g. fceumm/NES, gambatte/GB) running full-speed with audio, controller + touch input, and a basic library that can load a user-selected ROM. Auto-pause/save on background. |
| **Full parity** | All systems including N64/PSP via a GLES (or Vulkan) HW-render path, SAF ROM-folder library with scanning, achievements, all save/state features. |

"Running on Android" for the MVP explicitly **excludes** the two HW-3D cores and Discord rich presence.

---

## 3. Current Architecture Snapshot — Desktop Assumptions Baked In

Firelight is C++20 + Qt6/QML, libretro-based, desktop-only (Windows/Linux/macOS). The layering is clean (QML → Qt proxy → service layer → SQLite repos), which helps the port because input/audio/storage are already behind interfaces.

The desktop assumptions that break on Android:

- **Entry point** (`src/main.cpp`): `QApplication` (Widgets); `QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL)` hardcoded; `#include <unistd.h>`; storage via `QStandardPaths::DocumentsLocation` + `/Firelight`; a `portable.txt` mode assuming a writable app dir; SIGINT handler; **three window event filters installed** (`resizeHandler`, `inputMethodDetectionHandler`, `keyboardHandler` — lines 411–415) plus window x/y/size persistence. Note `keyboardHandler` is the desktop keyboard-input path *and* is installed as a window event filter, so it interacts with the input rewrite in 5.4 (cross-reference).
- **Rendering** (`emulator_item_renderer.cpp`): uses `QRhi`/`QQuickRhiItem`, branches OpenGL vs Vulkan; requests `RETRO_HW_CONTEXT_OPENGL`; the `RETRO_HW_CONTEXT_OPENGLES_VERSION` lines are commented out; an **extensively scaffolded but non-functional Vulkan path** (hundreds of lines, no working `create_device`); per-frame readback via `QRhiReadbackResult`. The actually-used desktop-GL-flavored calls are `QOpenGLContext::currentContext()->getProcAddress` (line 134), `glGetIntegerv(GL_FRAMEBUFFER_BINDING)` (line 423, inside the context-reset block), and `initializeOpenGLFunctions()` (line 333, from `QOpenGLFunctions`). The includes `<QOpenGLPaintDevice>`, `<QAudioInput>`, `<QMediaFormat>`, `<QVideoFrame>` are **dead** (present but never instantiated) — not porting blockers.
- **Core loading** (`libretro/core.cpp`): loads via `QLibrary` (the `SDL_LoadObject` path is commented out) from a relative `./system/_cores/<os>/` path; `#include <bits/fs_path.h>`; cores fed in-memory `info.data` with a **filename-only** `info.path` (loadGame line 1201).
- **Input** (`input2/sdl/`): SDL2 GameController on a detached `QtConcurrent` thread running `SDL_WaitEvent` (`main.cpp` line 420); keyboard via Qt event filter; `RETRO_DEVICE_POINTER` already plumbed via `IPointerInputProvider` (`core.cpp` 66–81); **no touch-driven gamepad input** and **no QML touch overlay**.
- **Audio** (`audio/audio_manager.cpp`): `QAudioSink` (Qt Multimedia) sink, FFmpeg `libswresample` for same-rate buffer compensation; `SfxPlayer` uses several `QSoundEffect` instances.[^counts]
- **UI** (`qml/Main3.qml` etc.): hover-driven, right-click context menus, resizable-window assumptions, no safe-area/orientation handling.[^counts]
- **Services**: cpr/libcurl for RetroAchievements HTTP (multiple `cpr::Post` call sites across 5 files — see 5.8); `QNetworkAccessManager` for image cache; Discord Social SDK (desktop-only binary blob); `ca-bundle.crt` for TLS.

[^counts]: Precise file/site counts in this document (e.g. files using `hoverEnabled`, number of right-click menus, `QSoundEffect` instance count) are order-of-magnitude indicators from a working-tree grep, not audited exact figures, unless a specific source line is cited. They are intended to convey scope, not to be load-bearing.

---

## 4. Compatibility Matrix

| Subsystem | Current state | Android status | Effort | Key risk |
|---|---|---|---|---|
| Build / toolchain / CMake | Desktop presets; stale `android-toolchain.cmake` | ⚠️ needs work | M | NDK/Qt/Play version drift; 16 KB alignment |
| Dependencies (vcpkg + pkg-config) | vcpkg deps + pkg-config FFmpeg; bare `z`/`sqlite3` | ⚠️ needs work | M–L | FFmpeg & cpr/curl/OpenSSL chain on arm64-android |
| Libretro core loading | `QLibrary` from relative path, x86_64 binaries | ⛔ blocker | L | Wrong-arch binaries; `<bits/fs_path.h>`; absolute-path + `.so.zip` packaging |
| Core binary availability | x86_64 only | ✅ works (prebuilts exist) | S–M | Nightly drift; must pin/snapshot |
| Graphics / GLES renderer | Desktop OpenGL; HW path scaffolded, non-functional | ⛔ blocker | L–XL | Qt-Quick HW-render path unproven; N64/PSP device variance |
| Input (controllers) | SDL2 on background thread; no Android Activity integration yet | ⛔ blocker | L | SDL needs Activity event bridge; native backend recommended |
| Input (touch) | Pointer path exists; no gamepad overlay | ⛔ blocker (mobile) | L | Build virtual gamepad from scratch |
| Audio | `QAudioSink` + swresample | ⚠️ needs work | M | Latency/buffer tuning; FFmpeg duplication |
| Storage / scoped storage | `QStandardPaths` Documents model | ⛔ blocker | L | SAF `content://` vs `need_fullpath` cores |
| Qt/QML UI & lifecycle | Desktop QApplication, hover, resize | ⚠️ needs work | L | Touch UX, back button, lifecycle pause/save, safe-area |
| Networking / RetroAchievements | cpr + QNetwork | ⚠️ needs work | S–M | TLS/OpenSSL packaging; QNetworkInformation backend |
| Discord | Desktop binary blob | ⛔ on Android (compile out) | S | Rich presence is desktop-RPC-only regardless |
| Patching / DB / portability | std::filesystem, SQLite, POSIX-isms | ⚠️ needs work | S | `<bits/*>` includes; yay0 leak; `<unistd.h>`; x86-SIMD CPU-feature reporting |

Legend: ✅ works · ⚠️ needs work · ⛔ blocker. Effort: S/M/L/XL.

---

## 5. Per-Subsystem Gap Analysis

### 5.1 Build / Toolchain / Dependencies

**Current state.** CMake 3.23.5+; presets `mingw64-debug` (GCC/MSYS2), `debug-win`/`release-win` (Clang+vcpkg), `xcode`. `vcpkg.json` lists cpr, ffmpeg, qtquick3d, qtmultimedia, qtbase, qtdeclarative, sdl2, spdlog, gtest, nlohmann-json, libarchive. In `CMakeLists.txt`, FFmpeg is found via `pkg_check_modules` (lines 64–68), and crucially **`z` (zlib) and `sqlite3` are linked as bare system libraries** (lines 221, 224) — exactly the zlib-mismatch risk called out below. The vendored `rcheevos`, `library`, `patching`, and **`libs/discord` (added unconditionally via `add_subdirectory(libs/discord)`, line 99)** targets are linked into `firelight_lib` (lines 219–250). `android-toolchain.cmake` exists but is stale: API 21, `c++_shared`, dead `aarch64-linux-android-4.9` GCC paths, and uses CMake's *built-in* Android workflow (`CMAKE_SYSTEM_NAME Android`), which Google explicitly does not test or support.

**Android requirement.** A proper NDK toolchain integration, an Android CMake preset, a Gradle/manifest layer, and arm64-android builds of every native dep.

**Gap.** No Android preset; wrong toolchain approach; no Gradle infrastructure; FFmpeg `pkg-config` won't resolve in the NDK sysroot; vcpkg Android triplet not wired; `add_subdirectory(libs/discord)` is unconditional and must be guarded on Android (see below).

**Recommended approach.** Adopt the **official `qt-cmake` Android workflow** and *delete* `android-toolchain.cmake` and the commented-out `qt-android-cmake` FetchContent. Invoke the per-ABI `~/Qt/<ver>/android_arm64_v8a/bin/qt-cmake` with `ANDROID_SDK_ROOT`/`ANDROID_NDK_ROOT`; build with `cmake --build <dir> --target apk`/`aab`. For vcpkg deps, chainload the NDK toolchain (`VCPKG_CHAINLOAD_TOOLCHAIN_FILE=$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake`, `VCPKG_TARGET_TRIPLET=arm64-android`) — **do not** rely on the built-in-Android CMake workflow. Supply custom packaging via `QT_ANDROID_PACKAGE_SOURCE_DIR` (AndroidManifest, permissions, icon, Gradle). **Take zlib from the NDK system libz** (replace the bare `z` link at line 221) to avoid the documented vcpkg/NDK zlib-mismatch bug (vcpkg #18015/#20057); link `sqlite3` from the Android-cross-compiled port rather than a host system lib. **Guard `add_subdirectory(libs/discord)` and the `discord` link behind `if(NOT ANDROID)`** to compile out Discord — this is a concrete step, not implicit. **Remove `src/later/video_encoder.cpp` and `src/later/video_decoder.cpp` from `firelight_lib`** on Android (they are unconditionally compiled today — see 5.5) and guard the `av::VideoEncoder`/`av::VideoDecoder` references in `emulator_item_renderer.cpp` (currently commented out but present). Pin a vetted NDK (newer NDKs broke OpenSSL on r26, SDL2 on r27, FFmpeg on r26).

Dependency status: sqlite3, zlib, nlohmann-json, spdlog/fmt, libarchive (minimal features, **disable crypto** to avoid the OpenSSL chain), SDL2, GTest, rcheevos all cross-compile to arm64-android. FFmpeg and the cpr→curl→TLS chain are the two problem areas (see 5.5, 5.8).

**Effort: M–L.** **Risks:** NDK/Qt/Play version drift; vcpkg Android triplet loads as "community … not guaranteed to succeed"; FFmpeg arm64-android historically flaky; `c++_shared` STL must be consistent across the app **and** every prebuilt core.

### 5.2 Libretro Core Loading & Packaging

**Current state.** `QLibrary` loads cores by *relative* path `./system/_cores/<os>/` (`platform_metadata.hpp` `getCoreDirectoryPath`, line 418); x86_64 binaries; `info.data` in-memory load with filename-only `info.path` (`core.cpp` loadGame line 1201). `core.cpp` includes `<bits/fs_path.h>` (line 6).

**Android requirement.** Load arm64-v8a `.so` cores by **absolute** path, either from the APK `nativeLibraryDir` (bundled) or app-private internal storage (downloaded).

**Gap & verification-corrected position.** The original audit framed dynamic `.so` loading as nearly a blocker. **Verification refutes the strong form:** on modern Android, `dlopen()` of a well-formed PIC `.so` from the app's **private internal data dir** (`/data/data/<pkg>/...`) *does* work even at targetSdk 29+ — only `execve()` of standalone binaries is blocked (the `app_data_file` SELinux context keeps `execute`, loses only `execute_no_trans`). This is exactly how RetroArch ships sideloaded cores. `QLibrary` uses `dlopen` under the hood, so this applies. The genuine constraints are narrower:

- **External/shared/scoped storage** (`getExternalFilesDir`, Downloads, SD card) is typically `noexec` — you **cannot** `dlopen` a core directly from there; it must be copied into app-private storage first.
- **Google Play policy** (separate from the OS) restricts downloading executable code from non-Google servers — this is why RetroArch maintains separate Play and sideload builds, not a technical W^X limit.
- **16 KB alignment** (NDK r28+) is mandatory for Play from the page-size deadline.
- The relative `./system/...` path won't resolve on Android.

**Recommended approach.** Add an `OS_ID_ANDROID` branch to `platform_metadata.hpp` (`getCoreDirectoryPath` line 418, `getCoreDllExtension` line 430, `getCoreDllPath` line 443) returning an **absolute** path + `.so`. **Critical ordering detail:** the `OS_ID` selection block (lines 245–258) currently chooses Linux via `#elif __linux__`. Because **`__ANDROID__` also defines `__linux__`**, the new `#elif defined(__ANDROID__)` case **must precede** the `__linux__` case, or Android will be misdetected as Linux. For bundled cores, plumb `nativeLibraryDir` from Java/JNI at startup; name them `lib<core>.so` (the platform only extracts `lib*.so` from `lib/<abi>/`) and let AGP package them uncompressed and 16 KB-aligned (`useLegacyPackaging=false`). For downloaded cores, copy to app-private storage and `dlopen` from the absolute path. Add an **unzip step** — buildbot Android cores ship as `.so.zip`. Replace `<bits/fs_path.h>` with `<filesystem>`.

**Effort: L.** **Risks:** Play policy on runtime core downloads; `nativeLibraryDir` path changes every install (read at runtime); STL/ABI consistency with cores.

### 5.3 Graphics / GLES Rendering (highest-risk workstream)

**Current state.** `main.cpp` hardcodes `setGraphicsApi(OpenGL)`. `emulator_item_renderer.cpp` uses `QRhi`/`QQuickRhiItem`, branches OpenGL vs Vulkan, requests `RETRO_HW_CONTEXT_OPENGL`, and has `getHwRenderContext()` stubbed (the `RETRO_HW_CONTEXT_OPENGLES_VERSION; major=3; minor=1` lines are commented out). The **actually-used desktop-GL entry points** are `QOpenGLContext::currentContext()->getProcAddress` (line 134) and `initializeOpenGLFunctions()` (line 333, via `QOpenGLFunctions`). The FBO id is captured by `glGetIntegerv(GL_FRAMEBUFFER_BINDING, &m_currentFramebufferId)` at **line 423, inside the `if (m_resetContextFunction)` guard (line 416)** — i.e. it is read **once on context reset**, not on every `initialize()` call and not per frame. The `<QOpenGLPaintDevice>` include (line 5) and the multimedia includes are **dead** (never instantiated) and are not porting blockers.

The **Vulkan path is far more built-out than a stub:** `setHwRenderInterface`/`setHwRenderContextNegotiationInterface` (lines 206–323) wire `set_image`, `get_sync_index`, device/instance proc-addr lambdas, queue handles from `QRhiVulkanNativeHandles`, and context negotiation. It is **extensively scaffolded but non-functional** — `create_device2` (line 217) returns `false`, so there is no working `create_device`. This matters for Open Question #1: the Vulkan scaffolding already exists, which may make it a *competitive* path to N64 (parallel-rdp) rather than an obviously-riskier one.

**Android requirement.** OpenGL ES only (desktop GL does not exist on Android). Qt's RHI maps `QSGRendererInterface::OpenGL` to a **GLES** EGL context on Android automatically, so the OpenGL *branch* still executes — but every GL call runs against a GLES context.

**Gap.** Software cores need a GLES texture-upload path (mostly fine — Firelight uploads a `QImage` via `QRhi`, which is shader-agnostic, and uses Qt's precompiled `.qsb` scene-graph shaders, so **no hand-written GLSL porting** is needed). The HW-3D cores need `RETRO_HW_CONTEXT_OPENGLES3` declared and a working frontend-owned GLES context.

**Verification-corrected position (load-bearing).** The research claim that "GL-heavy cores run under a GLES context driven by Qt Quick's render thread, *proven in real Android frontends*" is **only partially supported**. What is proven: GL-heavy cores run on Android under GLES (RetroArch via EGL, Lemuroid via native `GLSurfaceView`). What is **not** proven: the *Qt-Quick-render-thread-as-driver* mechanism — **no shipping Android libretro frontend uses Qt Quick.** Qt 6 owns the RHI context on its render thread and provides no supported way to recover a raw GL context the way Qt 5 allowed; feeding a libretro core an FBO that shares Qt's RHI context requires careful `QSGRenderNode`/`QSGRendererInterface` plumbing that is **unproven** for libretro HW cores and may fight Qt's threaded render loop and context-reset semantics. Treat it as a hypothesis to prototype, not an established pattern — and note (Section 1) that the *GLES handshake under Qt Quick is itself unproven*, so Vulkan is not automatically the riskier choice.

**Context-lifetime hazard (concrete).** Context reset/destroy ownership is **split across two classes**: `Core` stores `m_destroyContextFunction` (`core.cpp` line 244) and calls it **unconditionally** in `~Core()` (lines 1185–1187), while `EmulatorItemRenderer` separately stores/calls `m_resetContextFunction`/`m_destroyContextFunction` (lines 153–159, 416–428). On Android surface-loss/recreate cycles, this dual ownership is exactly where double-destroy / use-after-destroy bugs surface. **Consolidate `context_reset`/`context_destroy` ownership in one place** before relying on surface recreation.

**Recommended approach.** Keep GLES for v1 (broadest device support; defer the scaffolded Vulkan path unless Phase 3 chooses it). Make `getPreferredHwRender()`/`getHwRenderContext()` platform-aware: return `RETRO_HW_CONTEXT_OPENGLES_VERSION` (major=3, minor=1) on Android — uncomment the existing lines. **Re-read `GL_FRAMEBUFFER_BINDING` every frame** rather than caching it at context reset (Qt's RHI rotates framebuffers across swapchain frames; the current once-on-reset capture at line 423 will render to a stale FBO — find it via the `m_resetContextFunction` guard at line 416). Keep all core GL calls inside `cb->beginExternal()/endExternal()` (already done). Be **GL-context-loss resilient**: assume the EGL surface and all GL resources can be destroyed on background and must be recreated on resume (mirror Lemuroid/LibretroDroid `onSurfaceCreated → reinitialize`); consolidate the split context-destroy ownership first. Bring up cores in risk order: validate the handshake with **ppsspp** first (self-contained GLES shaders), then attempt **mupen64plus_next** with the GLES3 GLideN64 build (expect device-specific black screens/crashes).

**Effort: L–XL.** **Risks:** the unproven Qt-Quick HW-render path; split context-destroy ownership; N64 GLES instability across Adreno/Mali; per-frame `QRhiReadbackResult` readback is expensive on tile-based mobile GPUs (used for rewind/thumbnails — may tank performance).

### 5.4 Input & Touch

**Current state.** `input2/sdl/` runs `SDL_WaitEvent` on a detached `QtConcurrent` thread (`main.cpp` line 420); keyboard via Qt event filter (`keyboardHandler`, installed as a window event filter, `main.cpp` line 415); `gamecontrollerdb.txt` mappings; **a working `RETRO_DEVICE_POINTER` provider** (`core.cpp` 66–81 via `IPointerInputProvider`); **no touch-driven gamepad input** and **no QML touch overlay**. The input layer is well-abstracted behind `IInputService`/`IGamepad`/`IRetroPad`, publishing platform-neutral `GamepadInputEvent`s via `EventDispatcher`.

**Android requirement.** Controller input must reach the app; touch must drive both UI and game input.

**Gap & verification-corrected position (refuted claim).** The research premise that "SDL2 GameController supplies input while Qt owns the Activity — SDL need not own the window/event loop" is **REFUTED**. Android delivers controller input *only* to the focused View/Activity via `dispatchKeyEvent`/`onGenericMotionEvent`, with **no polling API**. SDL's Android joystick backend (`SDL_sysjoystick.c`) is fed *exclusively* from Java/JNI callbacks driven by `SDLActivity`/`SDLControllerManager`. With Qt owning the Activity (`QtActivity`), SDL's event queue stays empty for controllers unless every `KeyEvent`/`MotionEvent` is manually bridged into SDL's private JNI entrypoints — undocumented, fragile across SDL versions, and with known ANR/deadlock issues. There is also a thread mismatch: Firelight pumps SDL on a **background thread** (`main.cpp` line 420) while Android input is dispatched on the UI thread — **but this is not an independent blocker; it is inherently resolved by the recommended native backend below** (which receives events on the Activity dispatch thread). Also note: analog sticks/triggers arrive only as generic `MotionEvent` axes; Qt's `QKeyEvent`/QML `Keys` delivery covers buttons but **loses analog data** needed for N64/PSP.

**Recommended approach.** **Drop SDL for Android input** and use a **native Android input backend** behind the existing `IInputService`, compiling `SDLInputService` desktop-only. This single decision resolves both the routing problem (events reach the focused Activity) and the thread mismatch (events arrive on the Activity dispatch thread, no background poll). Implement a custom `QtActivity` subclass overriding `dispatchKeyEvent`/`onKeyDown`/`onKeyUp` (gamepad buttons, `SOURCE_GAMEPAD`/`SOURCE_DPAD`) and `dispatchGenericMotionEvent` (analog axes, `SOURCE_JOYSTICK`, `getAxisValue`), forwarding to C++ via JNI to produce the same `GamepadInputEvent`s (reusing the `GamepadInput` enum, ±8192 deadzone logic, `gamepad_profile`, `controllers.db`). **Always call `super`** on events you don't consume, or Qt UI/back-button (and the `keyboardHandler` event filter, see 5.7) breaks. Use `InputDevice`/`InputManager.InputDeviceListener` for enumeration/hotplug. This mirrors RetroArch, which uses a native NDK input driver, **not** SDL. Build a **QML touch-gamepad overlay** (`MultiPointTouchArea`/`TapHandler`) feeding the same `IRetropadProvider`: per-platform layouts, adjustable opacity/size/position, haptics, auto-hide when a physical controller connects. The existing `IPointerInputProvider`/`RETRO_DEVICE_POINTER` path can be reused to feed touchscreen pointer input to pointer-driven cores (DS, lightgun). SDL is still used for **core loading**, so initialize it with no input subsystems on Android.

**Effort: L.** **Risks:** analog-axis path easy to miss in testing (digital pads "work" in menus while analog silently fails); JNI thread-attachment correctness; over-consuming events breaks Qt key handling.

### 5.5 Audio

**Current state.** `AudioManager` feeds int16 stereo frames into `QAudioSink` (Qt Multimedia) via a `QIODevice`, using FFmpeg `libswresample` for **same-rate buffer compensation** (`swr_set_compensation` — not rate/channel conversion). `SfxPlayer` uses several `QSoundEffect` instances from `qrc:`.[^counts] Buffer is 8192·mult bytes. `SDL_audio.h` is included but unused.

**Android requirement.** `QAudioSink` is supported on Android (AAudio API 28+ / OpenSL ES). FFmpeg must be available for arm64.

**Gap & key leverage.** Firelight's actual FFmpeg need is **narrow**: only `libswresample` + a few `libavutil` helpers at runtime; `avcodec`/`avformat`/`swscale` are used **only in `src/later/`** (planned video/netplay). **Correction to a common assumption:** `src/later/video_encoder.cpp` and `video_decoder.cpp` are **currently compiled into `firelight_lib`** (`CMakeLists.txt` lines 163–164) and referenced (commented-out) in `emulator_item_renderer.cpp` — so excluding `src/later/` on Android is **real work** (remove those two sources, guard the `av::VideoEncoder`/`av::VideoDecoder` references), not a no-op.

On the runtime side, **Qt 6.8 bundles and auto-deploys FFmpeg 7.1 into the APK** (FFmpeg is Qt's default Android Multimedia backend; MediaCodec deprecated in 6.8). The duplication-avoidance benefit is at **runtime** (ship one set of FFmpeg `.so`s, not two), *not* at build time: **you still need FFmpeg headers and a link target for arm64**, because Qt deploys those `.so`s for its own multimedia plugin and does not document/export them as an app-facing link target. So plan to obtain an arm64 FFmpeg **dev** package (headers + import) for `avutil`+`swresample`; the win is avoiding a *second runtime copy*, achievable by linking against the same `.so` Qt ships — which requires care, since Qt may not export those symbols.

**Recommended approach.** Exclude `src/later/` from the Android target (remove the two sources from `firelight_lib` and guard the renderer references); make `AVCODEC`/`AVFORMAT`/`SWSCALE` desktop-only and require only `AVUTIL`+`SWRESAMPLE` on Android. Add a portable `av_err2str` shim (the macro is a GCC-ism that breaks under NDK clang). Keep swresample for buffer compensation in the MVP (lighter resamplers — soxr, libsamplerate, Oboe — lack a `swr_set_compensation` equivalent and buy little). Re-tune `m_bufferSize` and the buffer-level moving average for Android latency; consider a lower buffer on AAudio (API 28+). Explicit device selection + Android audio-focus handling (via JNI) are nice-to-haves, not MVP blockers. **Do not use ffmpeg-kit** (retired Jan 2025, archived Jun 2025). Remove the dead `SDL_audio.h` includes.

**Effort: M.** **Risks:** version skew if a non-7.1 FFmpeg sneaks in via vcpkg static linkage (measure shipped `.so` for duplicates); `QAudioSink` latency may be too high for tight emulation feel (Oboe is the fallback, but start with `QAudioSink`).

### 5.6 Storage / Scoped Storage

**Current state.** `QStandardPaths::DocumentsLocation` + `/Firelight/{roms,saves}`; multiple SQLite DBs + `core-system/` BIOS dir in `AppDataLocation`; a large read-only `content.db`; `ca-bundle.crt`; `portable.txt` mode; `QFileSystemWatcher` on ROM dirs; LibArchive extraction in-place; `std::ofstream`/`QSaveFile` over `std::filesystem::path`.

**Important code reality — directories are collapsed today.** In `core.cpp`, `GET_SYSTEM_DIRECTORY` (lines 172–181), `GET_SAVE_DIRECTORY` (lines 472–481), and `GET_CORE_ASSETS_DIRECTORY` (lines 461–471) **all return the same `systemDirectory` string**, and `Core::setSaveDirectory` (line 1265) sets a `saveDirectory` member that is **never used** in the env callbacks (it is dead). So cores currently write saves *into the system directory*; the save/system separation the SAF plan below assumes **does not exist in code today** and must be added. This directly affects the Tier-A layout.

**Android requirement.** Scoped storage (Android 11+). `AppDataLocation` (app-private) works and is the right home for DBs/saves/BIOS/`content.db`. Public `DocumentsLocation` paths are not returned/writable; user ROM folders need SAF.

**Gap (decisive for this codebase).** Libretro `need_fullpath` cores **`fopen()` a real path** and **cannot read a `content://` URI**. Firelight currently loads ROMs in-memory (`info.data`) with a **filename-only** `info.path` — set for *every* core, not just software ones (`loadGame` line 1201 `strdup(... .filename().string())`; `GET_GAME_INFO_EXT` line 896 likewise passes `full_path = filename`). The byte-fed software cores will work from a SAF descriptor, but **mupen64plus_next (N64) and ppsspp (PSP)** will try to `fopen` a path that does not exist. Qt 6.6+ makes `QFile` understand `content://` URIs, but that **does not** help a third-party core calling libc `fopen`.

> **Unresolved code question (must audit before Phase 3):** because `info.path` is *already* a bare filename for all cores on desktop, **how do N64/PSP load on desktop today?** Either (a) they are fed a real path somewhere this audit did not cover, (b) `info.data` in-memory loading suffices for the current desktop builds, or (c) they do not currently work. This is a material gap — confirm the desktop loading path for `need_fullpath` cores before committing to the materialize-to-real-path plan.

**Recommended approach (two-tier, matching PPSSPP/Dolphin/Lemuroid).** *Tier A (no permission):* put all SQLite DBs, `core-system`/BIOS, **saves, and states** under `getExternalFilesDir()` (`AppDataLocation`) — and **first split the save directory from the system directory in `core.cpp`** (wire `GET_SAVE_DIRECTORY` to the now-live `saveDirectory` member instead of `systemDirectory`). *Tier B (user content):* ROM library folders added via SAF (`ACTION_OPEN_DOCUMENT_TREE` + `takePersistableUriPermission`). Bridge by core type: for in-memory cores, open the picked file via `openFileDescriptor`, read bytes into `info.data`; for `need_fullpath` cores, **copy/cache the ROM to a real app-private path** (`getExternalFilesDir()/cache/roms`, content-hash keyed to skip re-copies) before `loadGame` and pass that real path. Detect `need_fullpath` from `retro_system_info` rather than hardcoding. Provide an **"Import BIOS"** flow (`ACTION_OPEN_DOCUMENT`) copying into `core-system`. Extract `content.db`/`ca-bundle.crt` from APK assets to app-private storage on first run (idempotent — app-private storage is wiped on uninstall). Add a SAF backup/export for saves. Replace in-place LibArchive extraction with extract-to-cache. Disable `portable.txt` and `QFileSystemWatcher` (limited inotify on Android external storage — add manual refresh) on Android. Use file descriptors, not per-byte `DocumentFile` reads (SAF I/O is slow).

**Effort: L.** **Risks:** multi-GB PSP ISOs copied into app-private storage double usage and add latency; persisted SAF grants can be revoked / are capped (~512); app-private data lost on uninstall without explicit backup; the collapsed save/system directory must be untangled before saves land in the right place; `MANAGE_EXTERNAL_STORAGE` is a Play-policy minefield (see 7).

### 5.7 Qt/QML UI & Lifecycle

**Current state.** `QApplication` (Widgets); `ApplicationWindow` with persisted window x/y/size; hover-driven UI (`hoverEnabled`/`HoverHandler` across many files);[^counts] right-click menus;[^counts] `Qt.Key_Back` handled only in QML key handlers; no orientation/safe-area/lifecycle hooks; `FileDialog`/`FolderDialog`; `QSettings::IniFormat`. Three event filters are installed on the window in `main.cpp` (`resizeHandler`, `inputMethodDetectionHandler`, `keyboardHandler` — lines 411–415); **`keyboardHandler` is both the desktop keyboard-input path and a window event filter**, so it interacts with the input rewrite in 5.4 (cross-reference: disabling the *resize* filter on Android is fine, but the keyboard filter must keep functioning / be reconciled with the native input backend).

**Android requirement & verification-corrected position.** Qt for Android **supports QtWidgets/QApplication** — confirmed; Firelight's `QApplication` entry point can stay (switching to `QGuiApplication` is an optimization, not a requirement). All needed modules (Quick, QuickControls2, Quick3D, Multimedia, Svg, Sql, Network, OpenGL) are supported on Android. But the app is confined to a **single fullscreen Activity/window**, and Widgets are not touch-optimized.

**Gap.** No lifecycle integration (the biggest functional gap): Qt surfaces Android `onPause`/`onResume`/`onStop` only via `QGuiApplication::applicationStateChanged`, which Firelight handles nowhere. Hover-driven UI and right-click menus don't work on touch. Back button must suspend/save, not just pop the stack. No safe-area/orientation handling. Touch targets too small (≈24 px vs ≥48 dp).

**Recommended approach.** Add an `applicationStateChanged` handler that, on every transition out of `Qt::ApplicationActive`, **auto-pauses emulation and synchronously flushes SRAM/save-state** (treat `Suspended` as "process may die next"). Intercept `Qt.Key_Back` to open the pause/quick menu (explicit Quit to exit). Build a touch UX layer: replace hover tooltips with focus/long-press, replace right-click with long-press/overflow menus, bump touch targets to ≥48 dp. Adopt edge-to-edge/immersive fullscreen (mandatory on recent target SDKs) with `layoutInDisplayCutoutMode=ALWAYS`, insetting chrome and the touch overlay by display-cutout/gesture safe areas (QML `SafeArea` attached property, Qt 6.7+; budget a JNI `WindowInsets` fallback). Lock gameplay to landscape; scale the overlay by density. Replace `FileDialog`/`FolderDialog` with SAF (Qt routes `QFileDialog` to SAF, or use JNI intents). **Disable window-resize/position persistence and the resize event filter on Android** (keep the keyboard event filter working, reconciled with 5.4). Make GL resources recreatable on surface loss (see 5.3).

**Effort: L.** **Risks:** Qt-Android safe-area/cutout support is less battle-tested (may need JNI); forgetting `super` in overridden Activity dispatch silently breaks key handling; lifecycle save-on-background must be synchronous and correct or users lose progress.

### 5.8 Networking / Services

**Current state.** cpr (libcurl) for RetroAchievements. **cpr is referenced in 5 files** (`regular_http_client.cpp`, `ra_client.cpp`, `rcheevos_offline_client.cpp`, `achievement_service.cpp`, `AchievementSetItem.cpp`) with **multiple `cpr::Post` call sites** — at least `regular_http_client.cpp:15`, `achievement_service.cpp:271`, and `achievement_service.cpp:352`. `QNetworkAccessManager` + `QNetworkDiskCache` for image/HTTP; `QNetworkInformation::loadDefaultBackend()` for reachability; `ca-bundle.crt` for TLS; offline rcheevos client; rcheevos vendored C (cross-compiles cleanly).

**Android requirement.** TLS works on Android, but curl **cannot use Android's native TLS** — it must be built with an explicit OpenSSL/mbedTLS backend, and OpenSSL has documented arm64-android vcpkg build failures (NDK 26+).

**Gap.** The cpr→curl→OpenSSL chain is the most fragile dependency. Removing it is **not a one-line change**: the migration must cover **all cpr references/`cpr::Post` call sites across the 5 files**, not a single `Post`. `QNetworkInformation` backend availability on Android needs verification.

**Recommended approach.** **Migrate the cpr HTTP surface to `QNetworkAccessManager`** across all call sites. The Post calls map cleanly onto `QNAM::post`, and Qt is already linked. Scoping the migration to every cpr reference (not one site) removes cpr, libcurl, and a hand-built OpenSSL from the Android dependency graph. For TLS, use Qt's documented OpenSSL-for-Android path (package `libcrypto_3.so`/`libssl_3.so` via `QT_ANDROID_EXTRA_LIBS` or KDAB's `android_openssl`; gate on `QSslSocket::supportsSsl()`) — Qt does **not** bundle OpenSSL for legal reasons, but this is a well-trodden prebuilt path, far easier than hand-building the vcpkg OpenSSL port. Add a `QNetworkInformation` fallback (assume online with graceful degradation) if the Android backend is unavailable. rcheevos is fine as-is. Consider defaulting to offline-tolerant achievement behavior. (cpr may remain desktop-only behind a build guard if a full migration is deferred — but then it is not removed from the Android graph.)

**Effort: S–M.** **Risks:** still must ship OpenSSL `.so` in the APK; multiple call sites to migrate, not one; `QNetworkInformation` backend uncertainty.

### 5.9 Patching / DB / General C++ Portability

**Current state.** IPS/BPS/UPS/PM Star Rod patchers; Qt SQL + vendored SQLiteCpp; `std::filesystem` in patchers/saves; `core.cpp` includes `<bits/fs_path.h>` (line 6), `sdl_controller.cpp` includes `<bits/stl_algo.h>` (line 3); `<unistd.h>` in `main.cpp` and `emulator_item_renderer.cpp`; `filesystem_utils.cpp` has Windows/macOS branches only; `platform_metadata.hpp` has no Android case; `yay_0_codec.cpp:41` leaks a `new uint8_t[]`; bare `printf`/`strdup` throughout `core.cpp`.

**CPU-feature reporting (concrete, easy to miss).** The `GET_PERF_INTERFACE` `get_cpu_features` callback (`core.cpp` lines ~396–423) queries **x86-only SIMD** via `SDL_HasAVX`/`AVX2`/`MMX`/`SSE*` and returns an x86 `RETRO_SIMD_*` bitmask. On Android/arm64 these all return false, so cores receive a "no SIMD" mask and may fall back to slow scalar paths. **Audit and replace** with ARM NEON detection on arm64 (report `RETRO_SIMD_*` NEON flags), or HW-3D and DSP-heavy cores lose their SIMD fast paths.

**Android requirement.** NDK clang/libc++.

**Gap.** Hard compile blockers: `<bits/fs_path.h>` and `<bits/stl_algo.h>` (GCC-internal, absent in libc++). `<unistd.h>` exists in Bionic but usage must be audited. `std::filesystem` is fine on API 21+ with modern libc++.

**Recommended approach.** Replace `<bits/fs_path.h>` → `<filesystem>` and `<bits/stl_algo.h>` → `<algorithm>`. Audit `<unistd.h>` usage; guard or replace POSIX calls (`sleep`/`usleep` → `std::chrono`/`QThread`). Add `OS_ID_ANDROID` to `platform_metadata.hpp` (with the `__ANDROID__`-before-`__linux__` ordering of 5.2) and an Android branch to `filesystem_utils.cpp` (intent-based or no-op). Fix the `yay_0_codec.cpp` leak (`std::vector`). **Fix `get_cpu_features` to report ARM NEON on arm64.** Document the (benign, little-endian-correct) endianness assumptions in patchers. Route logging through spdlog (with an Android sink), not bare `printf`. Run a full `grep` for hidden POSIX deps (`fork`, `system`, `popen`).

**Effort: S.** **Risks:** more libstdc++-isms likely surface under libc++ beyond the two identified; the x86-SIMD CPU-feature path silently degrades performance rather than failing loudly; minor but must be fixed early to unblock the build.

---

## 6. Libretro Core Feasibility Table

**What Firelight actually ships today: 12 distinct core binaries** wired in `platform_metadata.hpp::getCoreDllPath` (lines 443–497): gambatte, mgba, fceumm, snes9x, mupen64plus_next, melondsds, genesis_plus_gx (covers SG-1000 / Genesis / GameGear / Master System), ppsspp, mednafen_supergrafx (covers **both** TurboGrafx-16/PC Engine **and** SuperGrafx), pokemini, mednafen_wswan, mednafen_ngp. **Neo Geo (geolith) has no case in `getCoreDllPath` — it is *not* integrated in code today and is aspirational in this plan, not a shipped core.**

All listed cores have official prebuilt arm64-v8a Android nightlies on `buildbot.libretro.com/nightly/android/latest/arm64-v8a/` (this is a planning artifact — the buildbot directory should be re-checked at port time rather than treated as a guaranteed live fetch). Prebuilt availability is **not** a blocker for the integrated cores. Cores ship as `<core>_libretro_android.so.zip` and must be inflated.

| Platform(s) | Core binary | Wired in code? | Android prebuilt | HW3D needs | BIOS | License | Notes |
|---|---|---|---|---|---|---|---|
| NES | fceumm | ✅ | ✅ | Software | None | GPLv2 | MVP candidate |
| GB/GBC | gambatte | ✅ | ✅ | Software | Optional | GPLv2 | MVP candidate |
| GBA | mgba | ✅ | ✅ | Software | Optional | MPL-2.0 | MVP candidate |
| SNES | snes9x | ✅ | ✅ | Software | Optional | **Non-commercial** | License blocks a paid app |
| Genesis/MS/GG/SG-1000 | genesis_plus_gx | ✅ | ✅ | Software | Optional | **Non-commercial** | One binary for 4 platforms; no Sega CD wired (no mandatory BIOS) |
| TurboGrafx-16 / PC Engine | mednafen_supergrafx | ✅ | ✅ | Software | None | GPLv2 | **Shares the supergrafx core** with SuperGrafx (lines 477–479) |
| SuperGrafx | mednafen_supergrafx | ✅ | ✅ | Software | None | GPLv2 | Same binary as TG16/PC Engine |
| WonderSwan | mednafen_wswan | ✅ | ✅ | Software | None | GPLv2 | |
| NeoGeo Pocket | mednafen_ngp | ✅ | ✅ | Software | None | GPLv2 | |
| Pokémon Mini | pokemini | ✅ | ✅ | Software | None | GPLv3 | |
| Nintendo DS | melondsds | ✅ | ✅ | Software (GL optional) | Optional | GPLv3 | Software mode works; not HW-gated |
| N64 | mupen64plus_next | ✅ | ✅ (gles2 **and** gles3) | **HW 3D (GLES)** | None | GPLv2 | **Phase 3.** No desktop-GL build — must move renderer to GLES; use gles3 variant, gles2 fallback. Riskiest core. |
| PSP | ppsspp | ✅ | ✅ | **HW 3D (GLES/Vulkan)** | None | GPLv2 | **Phase 3.** Best first HW-render validation core. |
| Neo Geo *(planned)* | geolith | ❌ **not wired** | ✅ | Software | **Mandatory** | BSD-3 + MIT | **Aspirational — no `getCoreDllPath` case today.** Would need `neogeo.zip`/`aes.zip` MAME ROMs and a clear "BIOS required" UX if added. |

Notes: (1) Nightlies track master — **pin/snapshot** specific buildbot revisions for reproducibility (Firelight's vendored cores are version-pinned; nightlies are not). (2) "Build exists" ≠ feature/save-state parity or identical BIOS/asset wiring. (3) If any Firelight core is a patched fork, the stock Android build will differ. (4) `melonds` (older) and `melondsds` (active) both exist — target `melondsds`. (5) The mednafen core suffixes match the code exactly: `mednafen_supergrafx_libretro`, `mednafen_ngp_libretro`, `mednafen_wswan_libretro`. (6) **If Firelight intends to ship Neo Geo, that core must first be wired into `getCoreDllPath` — it is not a packaging task but a code addition.**

---

## 7. Cross-Cutting Android Concerns

**Dynamic `.so` loading & core packaging.** Viable. `dlopen`/`QLibrary` of a PIC `.so` from **app-private internal storage** works at targetSdk 29+ (only `execve` of standalone binaries is blocked). Bundle the default cores in the APK `nativeLibraryDir` (named `lib*.so`, uncompressed, 16 KB-aligned) — always works and Play-compliant. Downloaded cores go to app-private storage and `dlopen` from an absolute path. **Cannot** `dlopen` from external/shared/scoped storage (`noexec`). Play policy independently restricts downloading executable code from non-Google servers (the reason RetroArch maintains two builds).

**Scoped storage & ROM/BIOS access.** Two-tier: app-private dirs for managed data (DBs, saves, BIOS, `content.db`), SAF for user ROM folders. `need_fullpath` cores (N64/PSP) require materializing ROMs to a real path. Note the current code collapses save/system/assets into one directory (5.6) — fix before relying on the tiering. `MANAGE_EXTERNAL_STORAGE` would restore desktop-like access but is a **Play-policy minefield** — emulators are *not* an enumerated permitted use, so a Play release using it risks rejection/removal. Verified: SAF + app-specific dirs is how Dolphin/PPSSPP/Lemuroid/RetroArch actually ship; PPSSPP explicitly states its `MANAGE_EXTERNAL_STORAGE` request "will likely end up being denied." `MANAGE_EXTERNAL_STORAGE` remains usable for **sideload/F-Droid** builds only — gate it behind a build flavor.

**Activity lifecycle (auto-suspend/save).** Hook `QGuiApplication::applicationStateChanged`; on leaving `ApplicationActive`, pause emulation and synchronously flush saves/state. Be resilient to GL surface/context destruction on background and recreate GL resources on resume (consolidate the split `Core`/`EmulatorItemRenderer` context-destroy ownership first — see 5.3).

**On-screen touch controls.** Build a QML touch-gamepad overlay feeding `IRetropadProvider` as RetroPad P1: per-platform layouts, adjustable opacity/size/position, haptics, auto-hide on physical-controller connect. Firelight today has the `RETRO_DEVICE_POINTER` provider path but **no touch-driven gamepad input and no overlay** — both must be built.

**Orientation / cutouts / back button.** Lock gameplay landscape; inset chrome/overlay by safe-area margins; intercept `Qt.Key_Back` → pause menu (not exit). Target the current edge-to-edge SDK floor.

**Distribution & legal (cross-cutting).** Google's developer identity-verification expansion is expected to apply even to **sideloaded** APKs (re-verify scope/timing at kickoff). Qt is LGPLv3 — dynamically link Qt + ship an in-app LGPL attribution/relink notice, **or** buy a commercial license (required for static linking / safer for Play). **snes9x and genesis_plus_gx are non-commercial** — they cannot ship in a paid app; identify GPL-compatible alternatives if Firelight monetizes. The GPL cores impose source-availability obligations and interact badly with proprietary blobs (Discord) — the dynamically-loaded-`.so` model is the standard mitigation but warrants counsel review. Never bundle copyrighted BIOS/ROMs. F-Droid requires stripping the proprietary Discord SDK.

---

## 8. Recommended Porting Strategy & Phased Roadmap

### Phase 0 — Build Bring-Up (foundation) — *≈3–5 weeks*
- Adopt `qt-cmake` Android workflow; delete `android-toolchain.cmake` + dead FetchContent. Add an `arm64-android` CMake preset.
- Fix compile blockers: `<bits/fs_path.h>` → `<filesystem>`, `<bits/stl_algo.h>` → `<algorithm>`; audit `<unistd.h>`; `av_err2str` shim; yay0 leak; `get_cpu_features` ARM-NEON fix.
- Author `QT_ANDROID_PACKAGE_SOURCE_DIR` (AndroidManifest, permissions `INTERNET`/storage, icon, Gradle).
- Cross-compile deps (zlib from NDK, replacing the bare `z` link; `sqlite3` from the Android port; libarchive minimal; **migrate cpr→QNAM**; FFmpeg avutil+swresample against arm64 dev headers / Qt's runtime 7.1; **remove `src/later/` sources from `firelight_lib`** and guard the renderer's `av::` references). **Guard `add_subdirectory(libs/discord)` and the `discord` link behind `if(NOT ANDROID)`** to compile out Discord.
- **Milestone:** APK builds and launches to the QML UI on an arm64 emulator/device (no emulation yet).

### Phase 1 — Software-Core MVP — *≈5–8 weeks*
- `OS_ID_ANDROID` in `platform_metadata.hpp` (**`__ANDROID__` before `__linux__`**; absolute paths, `.so`); bundle 2 software cores (e.g. fceumm, gambatte) in `nativeLibraryDir`; `.so.zip` inflate step.
- GLES texture-upload path for software cores; **per-frame FBO read** (not cached at context reset); consolidate split context-reset/destroy ownership; GL-context-loss handling.
- Native Android input backend (buttons + analog) behind `IInputService`; SDL input desktop-only. QML touch-gamepad overlay (reuse `IPointerInputProvider` for pointer cores).
- Storage: app-private DBs/saves; **split save/system dirs in `core.cpp`**; SAF single-ROM pick → bytes into `info.data`; `content.db` first-run extraction.
- Lifecycle: `applicationStateChanged` auto-pause + synchronous save; back-button → pause menu; landscape lock; safe-area insets.
- Audio: `QAudioSink` + swresample, Android buffer tuning.
- **Milestone:** load a user-picked NES/GB ROM, play full-speed with touch + controller, audio, auto-save on background.

### Phase 2 — Library + Full Software Parity — *≈4–6 weeks*
- All software cores bundled. SAF ROM-folder library (persistable grants, scanning, manual refresh). BIOS import flow. Achievements over QNAM + Android OpenSSL. Save/state, suspend points, backup/export. (If Neo Geo is desired, **wire geolith into `getCoreDllPath`** here — it is a code addition, plus mandatory-BIOS UX.)
- **Milestone:** all software systems playable with a real library and achievements.

### Phase 3 — HW-3D Cores (highest risk, conditional) — *≈4–8+ weeks, unbounded*
- **First audit:** confirm how `need_fullpath` N64/PSP cores load on desktop today (5.6) before building the Android path.
- Prototype the Qt-Quick + libretro `RETRO_HW_RENDER` GLES handshake (**unproven**). Validate with **ppsspp** first, then **mupen64plus_next** (gles3). Materialize `need_fullpath` ROMs to real paths. Per-device defaults, resolution scaling, frameskip.
- **Decision node — if the Phase 3 prototype fails (Qt-Quick HW handshake unworkable):** choose a fallback rather than abandoning N64/PSP silently:
  - **(a) Native surface path (recommended fallback):** render the emulator surface via a native Android `SurfaceView`/`GLSurfaceView` composited under/over the QML UI (Lemuroid-style), accepting a UI-architecture change for the emulator view only.
  - **(b) Vulkan via existing scaffolding:** finish the already-extensive Vulkan `retro_hw_render_interface_vulkan` path (implement a working `create_device`) and ship N64 (parallel-rdp) / PSP via Vulkan — leverages real existing code.
  - **(c) Software-render N64/PSP:** technically possible, **unacceptably slow** — last resort / not shippable.
  - Ship the software-core build (Phases 0–2) as the v1 product regardless; HW-3D is additive.
- **Milestone:** N64 and PSP playable on target devices (accept device-specific caveats), or a documented fallback decision.

### Phase 4 — Distribution & Hardening — *≈3–5 weeks*
- 16 KB alignment (NDK r28+) across all `.so` + cores. Decide Play vs sideload vs F-Droid; build flavors (Discord stripped for F-Droid; `MANAGE_EXTERNAL_STORAGE` for sideload only). LGPL attribution / commercial Qt license. Developer identity verification. Performance/thermal profiling.

---

## 9. Effort & Risk Summary

> **Section 1 orders blockers by *risk*; this section orders the same work by *critical-path sequence*.** Build bring-up is sequenced first (it gates everything) even though the GLES HW-3D renderer is the highest-risk item. T-shirt sizes map to the rough week ranges below; total ≈ **22–38 engineer-weeks (≈5–9 months for one engineer)**.

| Workstream | T-shirt | ≈ Weeks | Risk | Critical path? |
|---|---|---|---|---|
| Build/toolchain bring-up | M | 3–5 | Med | **Yes** (gates everything — *first*) |
| Compile-blocker fixes (`bits/*`, NEON, yay0) | S | 1–2 | Low | **Yes** (gates build) |
| Dependency cross-compile (FFmpeg, TLS, cpr→QNAM, Discord guard, `src/later` removal) | M–L | 3–5 | Med | Yes |
| Core packaging + loading | L | 3–4 | Med | Yes (gates MVP) |
| **GLES renderer (software path)** | M | 3–4 | Med | Yes (gates MVP) |
| **GLES renderer (HW-3D path)** | L–XL | 4–8+ | **High** | No (Phase 3, conditional + fallback) |
| **Native input + touch overlay** | L | 3–5 | Med–High | Yes (gates MVP) |
| Audio port/tuning | M | 1–2 | Low–Med | No |
| Storage / SAF / `need_fullpath` / dir-split | L | 3–5 | Med–High | Yes (Phase 1–2) |
| Qt/QML UI + lifecycle | L | 3–5 | Med | Yes (gates MVP) |
| Networking / achievements | S–M | 1–2 | Low–Med | No |
| Distribution / legal / 16 KB | M | 2–3 | Med | No (Phase 4) |

**Critical path (by sequence):** build bring-up → compile fixes → core loading + software GLES path + native input + lifecycle/UI + storage → **MVP**. The **HW-3D renderer (Phase 3)** is the highest-risk, least-bounded task, is deliberately off the MVP critical path, and is *conditional* (with a documented fallback). Input and storage are the next two riskiest because both involve bespoke JNI and refuted/qualified assumptions.

**Hard dollar costs (the only non-time costs):** commercial Qt license (per-developer annual fee — required for static linking / safer for Play; the LGPL dynamic-link path avoids it but constrains packaging) and the one-time Google Play developer-registration fee. These, not engineering hours, are the quotable line items behind the "XL, multi-month, one engineer" estimate.

---

## 10. Open Questions / Decisions Needed

1. **GLES vs Vulkan for HW-3D.** GLES = broadest devices but N64 instability *and* an unproven Qt-Quick handshake; Vulkan = better N64 (parallel-rdp) and Firelight's Vulkan path is **already extensively scaffolded** (needs a working `create_device`), though Qt-Android Vulkan is less mature. Because the GLES-under-Qt-Quick handshake is *itself* unproven, Vulkan is **not obviously the riskier choice**. **Recommend GLES for v1**, but keep Vulkan as a live Phase-3 fallback (8b).
2. **Input: native Android backend vs forcing SDL.** Verification says native is the robust path (SDL needs an undocumented Activity event bridge; native also resolves the background-thread mismatch). **Recommend native backend**, SDL desktop-only.
3. **Networking: keep cpr vs migrate to QtNetwork.** Multiple `cpr::Post` sites across 5 files; migrating removes the fragile curl/OpenSSL chain. **Recommend QNAM on Android** (migrate all call sites, not one).
4. **FFmpeg: reuse Qt's bundled 7.1 vs ship own.** **Recommend reuse at runtime** (avutil+swresample only) — but you still need arm64 FFmpeg headers/import at build time; the win is avoiding a *second runtime copy*.
5. **Distribution channel.** Play (constrained: bundled cores, SAF, no BIOS, current target SDK, 16 KB) vs sideload (flexible: downloadable cores, `MANAGE_EXTERNAL_STORAGE`) vs F-Droid (FLOSS-only, no Discord). Drives storage and packaging design — **decide early.**
6. **Storage model.** App-private-only (simplest, but uninstall wipes + hard to browse) vs SAF ROM folders (better UX, slower I/O, `need_fullpath` copy cost). Plus: **how do N64/PSP load on desktop today** given the filename-only `info.path`? Audit before Phase 3. **Recommend two-tier.**
7. **Monetization vs core licensing.** snes9x/genesis_plus_gx are non-commercial — if Firelight is ever paid, identify replacement SNES/Genesis cores. Requires legal review of the dynamically-loaded-GPL-`.so` model alongside the Discord blob.
8. **Qt licensing.** Dynamic-link LGPL + attribution screen vs commercial license (per-developer annual fee; required for static linking / safer for Play).
9. **Cores: prebuilt nightlies vs self-built pinned.** Recommend prebuilts, but **pin a dated snapshot** for reproducibility.
10. **Neo Geo / geolith.** Not wired in code today. **Decide whether to integrate it at all** (it requires a `getCoreDllPath` case + mandatory-BIOS UX) or drop it from scope.
11. **Min API / NDK.** Reconcile Qt 6.8 floor (API 28, NDK r26b/r27c) with Play's 16 KB mandate (NDK r28+) and the current target-SDK floor — likely a newer Qt + NDK r28+. Re-verify all floors at kickoff.

---

## 11. References (by topic)

**Qt for Android (build, modules, lifecycle, licensing)**
- https://doc.qt.io/qt-6/deployment-android.html
- https://doc.qt.io/qt-6/android-deploy-qt-tool.html
- https://doc.qt.io/qt-6/android-building-projects-from-commandline.html
- https://doc.qt.io/qt-6.8/android.html · https://doc.qt.io/qt-6/android.html
- https://www.qt.io/blog/qt-for-android-supported-versions-guidelines
- https://doc.qt.io/qt-6/cmake-variable-qt-android-abis.html
- https://doc.qt.io/qt-6/android-how-it-works.html · https://doc.qt.io/qt-6/android/org/qtproject/qt/android/QtActivityBase.html
- https://doc.qt.io/qt-6/android-platform-notes.html · https://doc.qt.io/qt-6/qtmodules.html
- https://doc.qt.io/qt-6/qapplication.html · https://doc.qt.io/qt-6/sql-driver.html · https://doc.qt.io/qt-6/whatsnew68.html
- https://wiki.qt.io/How_to_Create_and_Run_Qt_Application_for_Android · https://www.kdab.com/using-qt-make-native-android-apps/
- https://www.qt.io/development/open-source-lgpl-obligations · https://wiki.qt.io/Licensing-talk-about-mobile-platforms · https://forum.qt.io/topic/84051/distribution-with-lgpl

**Graphics: OpenGL ES / Qt RHI / libretro HW render**
- https://doc.qt.io/qt-6/topics-graphics.html · https://www.qt.io/blog/graphics-in-qt-6.0-qrhi-qt-quick-qt-quick-3d
- https://doc.qt.io/qt-6/qtquick-visualcanvas-adaptations.html · https://doc.qt.io/qt-6/qsgrendererinterface.html · https://doc.qt.io/qt-6/qrhi.html
- https://doc.qt.io/qt-6/qtquick3d-requirements.html · https://doc.qt.io/qt-6/qrhigles2nativehandles.html · https://doc.qt.io/qt-6/qopenglcontext.html
- https://www.qt.io/blog/2015/09/09/cross-platform-opengl-es-3-apps-with-qt-5-6 · https://doc.qt.io/qt-6/qtquick-scenegraph-openglunderqml-example.html
- https://docs.libretro.com/development/cores/opengl-cores/ · https://github.com/libretro/RetroArch/blob/master/libretro-common/include/libretro.h
- https://deepwiki.com/libretro/RetroArch/5.2-opengl-context-management
- https://github.com/Swordfish90/LibretroDroid · https://github.com/Swordfish90/Lemuroid
- https://www.khronos.org/opengl/wiki/Type_Qualifier_(GLSL) · https://registry.khronos.org/OpenGL/specs/es/3.0/GLSL_ES_Specification_3.00.pdf

**Libretro cores: availability, build, BIOS, license**
- https://buildbot.libretro.com/nightly/android/latest/arm64-v8a/
- https://docs.libretro.com/development/retroarch/compilation/android/ · https://developer.android.com/ndk/guides/ndk-build
- https://docs.libretro.com/library/mupen64plus/ · https://docs.libretro.com/library/ppsspp/ · https://docs.libretro.com/library/melonds_ds/ · https://docs.libretro.com/library/geolith/ · https://docs.libretro.com/library/genesis_plus_gx/
- https://raw.githubusercontent.com/libretro/libretro-core-info/master/geolith_libretro.info · https://raw.githubusercontent.com/libretro/libretro-core-info/master/snes9x_libretro.info
- https://github.com/libretro/libretro-super/tree/master/recipes/android · https://github.com/JesseTG/melonds-ds

**Android native lib loading / W^X / 16 KB pages**
- https://android.googlesource.com/platform/bionic/+/master/android-changes-for-ndk-developers.md
- https://developer.android.com/about/versions/10/behavior-changes-10
- https://github.com/agnostic-apollo/Android-Docs/blob/master/site/pages/en/projects/docs/apps/processes/app-data-file-execute-restrictions.md
- https://developer.android.com/guide/practices/page-sizes · https://android-developers.googleblog.com/2025/05/prepare-play-apps-for-devices-with-16kb-page-size.html
- https://developer.android.com/build/releases/agp-3-6-0-release-notes · https://developer.android.com/guide/playcore/asset-delivery/integrate-native
- https://wiki.libsdl.org/SDL2/SDL_LoadObject · https://developer.android.com/ndk/guides/cpp-support

**Input: SDL2 on Android / native input**
- https://github.com/libsdl-org/SDL/blob/main/docs/README-android.md · https://wiki.libsdl.org/SDL2/Android · https://wiki.libsdl.org/SDL2/README-android
- https://github.com/SDL-mirror/SDL/blob/master/src/joystick/android/SDL_sysjoystick.c · https://github.com/libsdl-org/SDL/issues/5356 · https://github.com/libsdl-org/SDL/issues/12204
- https://developer.android.com/develop/ui/views/touch-and-input/game-controllers/controller-input
- https://github.com/libretro/RetroArch/blob/master/input/drivers/android_input.c · https://docs.libretro.com/development/retroarch/input/input-drivers/
- https://docs.libretro.com/development/retroarch/input/overlay/ · https://www.arnorehn.de/blog/2023/10/31/qtgamepad-ported-to-qt-6/

**Storage / scoped storage / SAF**
- https://source.android.com/docs/core/storage/scoped · https://developer.android.com/about/versions/11/privacy/storage · https://developer.android.com/training/data-storage/use-cases
- https://developer.android.com/training/data-storage/app-specific · https://developer.android.com/training/data-storage/shared/documents-files
- https://support.google.com/googleplay/android-developer/answer/10467955 · https://developer.android.com/training/data-storage/manage-all-files
- https://www.qt.io/blog/qt-for-android-storage-updates · https://doc.qt.io/qt-6/qstandardpaths.html
- https://www.ppsspp.org/docs/getting-started/save-data-and-storage/ · https://dolphin-emu.org/docs/guides/controlling-global-user-directory/ · https://github.com/Swordfish90/Lemuroid/wiki/Frequently-Asked-Questions
- https://github.com/libretro/RetroArch/issues/12181 · https://github.com/dolphin-emu/dolphin/pull/9696

**FFmpeg / audio**
- https://doc.qt.io/qt-6.8/qtmultimedia-index.html · https://doc.qt.io/qt-6/qtmultimedia-building-ffmpeg-android-linux.html · https://wiki.qt.io/QtMultimedia_on_Android · https://doc.qt.io/qt-6/qaudiosink.html
- https://www.ffmpeg.org/legal.html · https://vcpkg.io/en/package/ffmpeg.html · https://tanersener.medium.com/saying-goodbye-to-ffmpegkit-33ae939767e1
- https://developer.android.com/games/sdk/oboe/low-latency-audio

**vcpkg / NDK cross-compilation**
- https://learn.microsoft.com/en-us/vcpkg/users/platforms/android · https://developer.android.com/ndk/guides/cmake · https://cmake.org/cmake/help/latest/manual/cmake-toolchains.7.html
- https://github.com/microsoft/vcpkg/issues/33881 (OpenSSL) · https://github.com/microsoft/vcpkg/issues/40268 (SDL2) · https://github.com/microsoft/vcpkg/issues/33963 / #11522 / #23654 (FFmpeg) · https://github.com/microsoft/vcpkg/issues/18015 / #20057 (zlib)
- https://vcpkg.io/en/package/spdlog.html · https://vcpkg.io/en/package/libarchive.html · https://vcpkg.io/en/package/curl.html · https://github.com/libcpr/cpr/issues/333
- https://doc.qt.io/qt-6/android-openssl-support.html · https://doc.qt.io/qt-6.8/ssl.html

**Discord Social SDK**
- https://docs.discord.com/developers/discord-social-sdk/core-concepts/platform-compatibility · https://discord.com/developers/docs/social-sdk/release_notes.html
- https://discord.com/developers/docs/social-sdk/getting_started.html · https://docs.discord.com/developers/discord-social-sdk/development-guides/setting-rich-presence · https://docs.discord.com/developers/discord-social-sdk/development-guides/account-linking-on-mobile

**Distribution / Play policy / F-Droid**
- https://developer.android.com/google/play/requirements/target-sdk · https://support.google.com/googleplay/android-developer/answer/11926878
- https://support.google.com/googleplay/android-developer/answer/9888072 · https://support.google.com/googleplay/android-developer/answer/15582165
- https://developer.android.com/develop/ui/views/layout/display-cutout · https://developer.android.com/develop/ui/views/layout/insets
- https://f-droid.org/en/docs/Inclusion_Policy/ · https://f-droid.org/en/docs/Anti-Features/ · https://forum.f-droid.org/t/retroarch-and-cores/22320
- https://www.timeextension.com/news/2025/08/google-could-be-killing-android-emulation-with-its-new-policy-update

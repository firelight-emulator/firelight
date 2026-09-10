# Frame pacing at 60 Hz

An investigation into two symptoms on the `emulator-timing` branch: frames the player never sees, and
runs that behave differently from one launch to the next. It ends with the cause identified and one
candidate fix parked. Everything here was measured on one machine — Windows 11, RTX 4070, a panel
switchable between 59.959 Hz and 119.961 Hz — running Snes9x (core rate 60.099 fps) under Vulkan and
Direct3D 11.

## Why 60 Hz and nothing else

The display supplies one pass slot per refresh. The core needs 60.099 frames a second.

| display | slots/sec | slots per core frame |
|---|---|---|
| 59.959 Hz | 59.959 | **0.998 — a deficit** |
| 119.961 Hz | 119.961 | 1.99 |
| 174.962 Hz | 174.962 | 2.91 |

Only at 60 Hz is there no headroom, so only at 60 Hz does anything have to give. Every symptom below
disappears at 120 Hz and above, which is why the whole area looked healthy for a long time.

The unavoidable cost at 60 Hz is 0.14 frames/sec — one every seven seconds. That is inherent to
`native`, not a defect. Everything this document is about is the excess over that.

## The instrument

`src/app/emulation/pace_probe.hpp`, behind `FL_PACE_PROBE=1`. One line a second:

```
pace: wakes=120 ticks=60 req=60 presents=60 syncs=60 renders=60 idle=0 frames=60 | pass 1x60
```

| counter | site | meaning |
|---|---|---|
| `wakes` | loop | every return from the loop's wait |
| `ticks` | loop | iterations that asked for at least one frame |
| `req` | `handleCommand` | RunFrame commands that reached the renderer |
| `presents` | `frameSwapped` | present submissions |
| `syncs` | `synchronize()` | the only event that drains commands |
| `renders` | `render()` | every call, including ones with nothing to draw |
| `idle` | `render()` | calls that found nothing owed |
| `frames` | `render()` | frames the core advanced through |
| `pass NxM` | `render()` | M passes carried N frames each |

Counting all of these separately is what settled the investigation. The performance overlay could not:
an empty pass returns before `recordFrame`, so "58 presents, 31 frame-carrying passes" reads the same
whether syncs are being wasted or never happening.

## Three instruments that lied

Each of these produced a confident wrong conclusion before being caught.

**`framesNotShown` was never reset.** `PerformanceStats::reset()` cleared `framesRun` and `framesLost`
but not `framesNotShown`, and `reset()` fires from `setSystemAVInfo` — reached mid-game on every
`RETRO_ENVIRONMENT_SET_GEOMETRY`, which SNES cores send constantly. The numerator was
process-lifetime, the denominator geometry-epoch. A reading of "1507 not shown out of 4293" is
arithmetically impossible in one epoch, which is what gave it away. Fixed.

**`Pacing:` shows the request, not the result.** The row is `getSyncMethod()` — the setting string —
not `getResolvedMode()`. A stored `sync-method` of `"display"` (not a valid value; the options are
`auto`/`audio`/`monitor`/`native`/`fixed`) falls through `syncMethodFromString`'s trailing
`return SyncMethod::Native` **silently**. The overlay said "display" while the pacer ran native, and a
whole conclusion about Display mode was built on it. Still unfixed: unknown sync methods should warn,
and that row should show the resolved mode.

**`frameRate` is cumulative since epoch.** It crawls toward the truth over minutes and reads
identically whether a run is recovering or broken. It cannot show a transient, which is exactly what
was being looked for.

## What was actually wrong

### The cost gate was inert, and it ratcheted

```cpp
const auto canAffordTwo = m_framesToRun > 1 && m_lastPresentRefreshes <= 1;
```

`<= 1` was written from `refresh_counter.hpp`'s `@return At least one`, which is **stale** —
`observe()` deliberately floors at 0 for a present queued inside one refresh. So the gate accepted
the pipelined case it existed to reject.

Worse, it made `m_framesToRun` rise unconditionally and fall conditionally: a pure ratchet to
`MAX_FRAMES_PER_PASS`, after which frames were genuinely lost and the rate sat under target.

Replaced with `owedThisPass = std::min(m_framesToRun, 2)` — no noisy input, cannot ratchet, bounded so
it cannot burst. `Lost` went to 0 and the rate holds. **The stale doc comment is still there.**

### Monitor mode's wake predicate was inert

```cpp
m_loopWake.wait_for(lock, 1ms, [this] { return m_emulationStopping.load(); });
```

A predicated `wait_for` re-evaluates on each wake and goes back to sleep for the remaining timeout
when the predicate is still false. So `frameSwapped`'s `notify_one()` woke the thread, was asked "are
we stopping?", and was put straight back down. The comment above it claimed "Display is woken by a
refresh rather than the timeout"; that could not happen.

The predicate now also reads `FramePacer::hasPendingPresents()`. Monitor at 60 Hz went from a best of
`ticks=57 | pass 1x54 2x3` to holding `ticks=60 idle=0 | pass 1x60` for 35 seconds unbroken.

## The 7-second beat, measured

In `native` at 60 Hz the doubled frames cluster, and the clusters land at `:00 :07 :14 :21 :29 :36
:43 :50` — 7.1 seconds apart, against a predicted 7.14 s for 60.099 vs 59.959. Each cluster costs 4-5
doubles, then it returns to 1. That is the mode working as designed, not a fault.

## What remains: the present train is either even or paired

`native` at 60 Hz is now clean — `idle=0 | pass 1x59 2x1`. `monitor` is bistable, and lands in one of
two states per launch:

```
wakes=120 ticks=60 req=60 presents=60 syncs=60 renders=60 idle=0  frames=60 | pass 1x60
wakes=120 ticks=30 req=60 presents=60 syncs=60 renders=60 idle=30 frames=60 | pass 2x30
```

`presents` is 60 in both. So is `wakes`.

**`wakes` is about 120 in every run, good and bad** — roughly 60 notify-driven plus 64 from the 1 ms
timeout firing at the 15.6 ms Windows granularity. The loop is wide awake throughout and is never the
bottleneck. In a bad stretch, 90 wakes saw `submitCount == 0` and 30 saw **2**, never 1.

`RefreshCounter::observe` accumulates `gap / period` and returns the integer part, and that reproduces
both states exactly:

| present gaps (periods) | remainder | returns |
|---|---|---|
| 1.0, 1.0, 1.0, 1.0 | 1.0, 0, 1.0, 0 | **1, 1, 1, 1** |
| 0.5, 1.5, 0.5, 1.5 | 0.5, 2.0, 0.5, 2.0 | **0, 2, 0, 2** |

Same correct code, distinguished only by whether presents arrive evenly or in pairs. The count is
conserved perfectly — it is delivered lumpily, and at `refreshesPerFrame = 1` each lump becomes two
frames in one pass.

Under D3D11 `Frame Time` and `Submit Time` read the same number and passes track presents 1:1. Under
Vulkan the pass rate can sit at half the present rate. The 3-image FIFO swapchain is the difference,
and it is the same chain depth already confirmed to cost one refresh of input lag.

The state is usually decided within the first second and can persist for a whole run, but it is
**metastable, not absorbing** — one run drifted 56, 51, 38, 30, sat at `2x30` for two seconds, then
climbed back to `1x58 2x1` unaided.

## Ruled out — do not re-derive

**The pass source.** `FL_PASS_SOURCE=present|render|both` switched which side asks for the next pass,
including the pre-`frameSwapped` design of a self-sustaining chain from `render()`. Both states appear
under every setting; one `render` run locked at `2x30` from its first report. This eliminates the
whole request path — coalescing, queue hops, `frameSwapped` vs `render()`. The switch has been
removed.

**Qt's missing vblank service on Vulkan.** `QWindowsWindow::requestUpdate()` branches on
`QDxgiVSyncService::supportsWindow()`; `qrhid3d11.cpp` and `qrhid3d12.cpp` call `registerWindow`,
`qrhivulkan.cpp` never does, so Vulkan falls back to `QPlatformWindow::requestUpdate()` and a 5 ms
timer — scaled down only when `refreshRate > 60.0`, so 59.959 pays the full 5 ms. All confirmed in
`C:/Qt/6.11.1/Src`. **It is not the cause.** `QT_QPA_UPDATE_IDLE_TIME=0` on Vulkan did not help, and
`QT_D3D_NO_VBLANK_THREAD=1` failed to reproduce the fault on D3D11 in ten tries.

**Audio backpressure.** The sink write is a non-blocking push; overflow goes to `m_pendingBytes` and
is discarded past capacity. Nothing waits. `Blocking: N%` on the overlay is `closeToBlockingPercent`,
a fullness indicator, not a record of anything blocking. Audio is downstream of irregular frame
delivery, not a cause of it.

**Core work time.** Sub-millisecond. A pass is never overrunning its slot on cost.

**Clock-only production.** A no-op in `native`, which resolves to `SyncMode::Fixed` where
`framesDueOnPresent()` returns 0 and the `RefreshCounter` path is discarded entirely. That mode
already is clock-only.

## Parked: count presents, not converted refreshes

In Display mode, drive `framesDueOnPresent()` once per **present** rather than once per counted
refresh, and let `refreshesPerFrame` do the division. With vsync on, presents equal refreshes over any
real interval — the pairing is short-term pipelining — so production becomes immune to gap jitter by
construction.

What it gives up is what `RefreshCounter` exists for: a present genuinely held for two refreshes (a
real dropped frame) would stop being noticed, so the core would not catch up. That floor-at-zero
change is on record as having taken monitor mode from 28% to 4.20% frame-time deviation, so the
counter is load-bearing and this trade needs thinking through rather than typing in.

## Open

- The stale `@return At least one` on `RefreshCounter::observe`, which is what made `<= 1` look right.
- An unrecognised `sync-method` silently becomes `Native`; `Pacing:` shows the request, not the result.
- `MAX_CORRECTION = 0.02` is the DRC's ceiling **and** its proportional gain (`-MAX_CORRECTION * error`).
  The comment above it describes 0.5%; RetroArch's `audio_rate_control_delta` defaults to 0.005.
- `m_renderContinuously` is written at `emulator_item.cpp:410` and read nowhere.
- Nothing measures the work *inside* a pass — `m_measureTime`, `m_averageEmulationTime` and
  `m_emulationWorkTimeBuffer` are declared and referenced by no `.cpp`.
- No test constructs `EmulatorItemRenderer`. `tests/app/emulation/` covers `FramePacer`,
  `RefreshCounter` and `EmulationRateController` only, and those test conservation over a sequence,
  never a single return value — which is why the stale doc survived.

## Method note

Six wrong conclusions were reached and discarded here. Every one came from reasoning past the point
where a measurement was available; every one was killed by a counter, not by an argument. The probe's
`wakes` counter in particular refuted a diagnosis that had already been written up as fact. When a
theory and a number disagree on this branch, the number has won every time.

# pacing-sim

`FramePacer`, `RefreshCounter` and `EmulationRateController` hold no thread and read no clock of
their own, so the whole pacing policy builds and runs without Qt, Windows or a GPU. These programs
drive it against synthetic present trains and modelled displays.

    ./build.sh && ./out/noise_sweep

| program | links production code | what it answers |
|---|---|---|
| `noise_sweep` | yes | how the current design loses frames as present timestamps get noisy |
| `policies` | no | pass counting vs. a display-matched clock, across refresh rates |
| `phase` | no | what clock drift costs, and what phase-locking buys back |
| `modes` | no | what native / display / auto resolve to on each panel, and the pitch cost |
| `latepoll` | no | how late input can be polled before frames miss their refresh |

## What is modelled and what is not

`noise_sweep` calls the real classes. The other three model the policy rather than linking it,
because the policies they compare do not exist in the tree yet.

Every program models the render side: a pass carries `min(owed, 2)` frames and shows only the last,
matching `render()` in `emulator_item_renderer.cpp`. Qt's `update()` coalescing is **not** modelled,
so loss figures for the current design are a floor rather than an estimate.

Findings are written up in `docs/emulation-loop-design.md`.

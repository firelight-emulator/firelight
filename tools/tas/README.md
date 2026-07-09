# TAS tooling (`tools/tas`)

Experimental Tool-Assisted Speedrun (TAS) tooling for Firelight. Standalone — no
Qt dependency; builds against the repo's `include/libretro/libretro.h` and drives
a bundled libretro core directly.

## `determinism_test` — Gate 1a (internal determinism)

Proves a libretro core (the bundled **gambatte**) replays **identically** given the
same inputs and savestates — the prerequisite for TAS re-recording, RNG-manipulation
search, and playback verification. It checks the *self-consistency* of this core
build; it does **not** check console/BizHawk accuracy (Risk 1b, deferred).

Passes (driven by a deterministic, purely-frame-derived input stream):

- **A / B** — two power-on runs with an identical input stream must produce
  identical per-frame serialized-state hashes.
- **C** — a savestate captured mid-run, restored into a fresh load, must reproduce
  the same hashes forward (serialize/unserialize is frame-exact).

### Build

```sh
sh tools/tas/build.sh
```

Uses the MSYS2 mingw64 `g++` (override with `CXX=...`).

### Run

```sh
tools/tas/determinism_test.exe _cores/windows/gambatte_libretro.dll <rom.gbc> [frames=4000] [seed=1]
```

e.g. against Pokémon Yellow (USA/EU, SHA-1 `CC7D0326…`):

```sh
tools/tas/determinism_test.exe _cores/windows/gambatte_libretro.dll pokemon_yellow.gbc 4000 1
```

The tool **sweeps several input seeds** (savestate reliability is seed-dependent).
Exit `0` / **`GATE 1a PASS`** requires power-on determinism **and** same-instance
savestate reliability across all seeds. Any GB ROM exercises the core; Pokémon
Yellow is used because it is the POC target.

## Result — bundled stock gambatte v0.5.0 (2026-07-09, Pokémon Yellow, 12 seeds)

| Check | Result |
|---|---|
| power-on determinism | **12 / 12 PASS** |
| savestate, same instance | **10 / 12 PASS** |
| savestate, fresh instance | **3 / 12 PASS** |

**Verdict: GATE 1a FAIL for savestate-based authoring.** Power-on determinism is
solid (full power-on replays are reproducible), but the bundled stock
`gambatte v0.5.0` core has **incomplete savestates**: restoring a savestate and
continuing diverges intermittently — ~17% of input sequences even within a live
session, and most across fresh instances. Re-recording and RNG-manipulation search
require reliable savestate branching, so **TAS authoring needs a newer /
accuracy-lineage gambatte core** (see Risk 1b in the scoping doc), not the bundled
v0.5.0. Confirming *which* build fixes savestates — a newer RetroArch-buildbot
gambatte, or the `gambatte-speedrun`/GSR lineage — is the next step.

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

Exit code `0` and **`GATE 1a PASS`** ⇒ the stock core is internally deterministic
and TAS authoring is sound on it. Any GB ROM exercises the core; Pokémon Yellow is
used because it is the POC target.

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

## Result — bundled core comparison (2026-07-09, Pokémon Yellow, seed sweep)

| Bundled core | power-on | savestate (same) | savestate (fresh) | Verdict |
|---|---|---|---|---|
| stock `gambatte v0.5.0` | 12/12 | 10/12 | 3/12 | **FAIL** — incomplete savestates |
| **`mGBA 0.11-dev`** | 16/16 | **16/16** | **16/16** | **PASS** — use this |

**Conclusion.** The bundled stock `gambatte v0.5.0` has **incomplete savestates**
(restoring and continuing diverges intermittently — ~17% of input sequences even
within a live session, most across fresh instances), so it is **not** usable for
savestate-based TAS authoring. The bundled **mGBA** core is power-on-deterministic
with **fully reliable savestates** (same- *and* fresh-instance), so it is the core
to author on — selectable in Firelight via `--core mgba_libretro`.

gambatte is the TAS community's *console-accuracy* standard, so a
`gambatte-speedrun` / GSR accuracy-lineage core (Risk 1b in the scoping doc)
remains the eventual upgrade **if** console-verified runs are wanted; it is **not**
required for in-app authoring.

Compare cores yourself:

```sh
determinism_test.exe _cores/windows/mgba_libretro.dll     <rom.gbc> 8000 16   # PASS
determinism_test.exe _cores/windows/gambatte_libretro.dll <rom.gbc> 8000 16   # FAIL
```

## Movie engine — `tas_movie`

A headless `.fltm` movie engine on a libretro core (use **mGBA** for Game Boy, per
gate 1a). Subcommands:

- `gen     <core> <rom> <out.fltm> [frames] [seed]` — scripted-input test movie.
- `compile <core> <rom> <script.txt> <out.fltm>` — author from an input script.
- `play    <core> <rom> <movie.fltm>` — deterministic replay + checkpoint report.
- `verify  <core> <rom> <movie.fltm>` — replay-twice determinism + checkpoint sync.
- `shot    <core> <rom> <movie.fltm> <frame> <out.ppm>` — dump a frame (binary PPM).
- `ram     <core> <rom> <movie.fltm> <frame> <hexAddr> <len>` — hexdump SYSTEM_RAM.
- `read    <core> <rom> <movie.fltm> <frame> <gbAddr> <len>` — read any Game Boy address (WRAM / I-O / **HRAM**) via the core's memory-map descriptors.
- `sweep   <core> <rom> <movie.fltm> <atFrame> <maxDelay> <gbAddr> [len]` — **luck manipulation**: savestate at a frame, then for each idle-frame delay 0..N show how the target (e.g. the RNG bytes) shifts — the search picks the delay that hits the wanted value.
- `watch   <core> <rom> <movie.fltm> <frame>` — decode named Pokémon Yellow state (RNG/DSum, party, battle you/foe, position, trainer ID) — see `yellow_ram.hpp`.
- `maps    <core> <rom> _` — dump the core's memory-map descriptors.

### Gen-1 assist (Phase 3)

`yellow_ram.hpp` holds the authoritative Pokémon Yellow RAM/HRAM addresses (from
the `pokeyellow.sym` symfile — Yellow's WRAM is shifted vs Red/Blue). Reads go
through the core's `RETRO_ENVIRONMENT_SET_MEMORY_MAPS` descriptors (`readGB`),
which reach WRAM banks, `rDIV`, **and** the HRAM RNG bytes (`0xFFD3/0xFFD4`) that
are not in `SYSTEM_RAM`. `sweep` demonstrates the manipulation lever (each idle
delay → a distinct RNG state); a targeted search over `sweep` against a `watch`
oracle (crit / forced miss / wild-encounter DVs) is the "assist". Verified
in-game on mGBA: a compiled script reaches the overworld and `watch` reports real
`map`/position/`trainerID`.

**Input-script format** (one directive per line; `#` = comment):

```
<frameCount> <buttons>   # buttons: - (none) or +-joined A B START SELECT UP DOWN LEFT RIGHT
60 -                     # wait 60 frames
2  START                 # tap Start
16 RIGHT                 # walk right
2  RIGHT+A               # right + A
```

`.fltm` = FLTM header (core+ROM identity, rerecord count) + per-frame `InputFrame`
log (10 bytes, mirroring Firelight's record) + optional frame→framebuffer-hash
checkpoints (used by `verify`). Verified end-to-end on mGBA + Pokémon Yellow:
`examples/yellow_boot.txt` compiles, replays deterministically, and `shot` at frame
935 renders the **Pokémon Yellow title screen** (correct RGB565 colours).

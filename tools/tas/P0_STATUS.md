# TAS harness — Phase 0 (de-fork & CI gate) status

Phase 0 from the roadmap: make the POC assets first-class Firelight citizens and
lock determinism into CI, before building any UI on top. No user-visible features.

## Done + verified in this change

| Item | What | Verified |
|---|---|---|
| **Multi-byte poke** | `poke` now parses each hex token MSB-first into whole bytes (`0300` / `0x0300` → `03 00`), instead of truncating a multi-digit token to one byte. Single-byte tokens are unchanged (back-compatible). | Built; `poke 0xD16B: 0300` writes 2 bytes; `read` confirms `03 00`. |
| **One RAM address model** | Retired the raw `SYSTEM_RAM` buffer-index `ram` command; `ram` is now a deprecated alias of `read` (emulated GB addresses via the memory map), so `read` / `poke` / `sweep` / `watch` all share one address space. | Built; `ram … 0xD16B 2` prints the same GB bytes as `read` + a deprecation note. |
| **InputFrame drift guard** | `fltm.hpp`'s `InputFrame` documents that it mirrors `firelight::input::InputFrame` and adds `static_assert`s pinning the 10-byte layout (`buttons`@0, axes @2/4/6/8, `sizeof==10`). | Compiles (asserts pass). |
| **CMake target** | `tools/tas/CMakeLists.txt` builds `tas_movie` + `tas_determinism_test` as CMake targets sharing `include/libretro`, `include`, and the `firelight/input` header tree. Root `CMakeLists.txt` gains `option(FL_BUILD_TAS OFF)` + a gated `add_subdirectory` — inert by default. | `cmake -S tools/tas -B build && cmake --build build` produces the binaries. |
| **Determinism gate — WIRED** | `add_test(tas_verify)` replays a checkpoint-free reference movie (`tests/reference.fltm`) over a self-authored test ROM (`tests/testrom.gb`, built by `tests/make_testrom.py`) and the committed per-platform mGBA core, asserting `verify`'s replay-×2 self-check passes (portable — not pinned to a core build). `-DFL_BUILD_TAS=ON` is now set in both CI workflows' Configure step, so it runs under the existing `ctest`. | `cmake -S tools/tas -B build && cmake --build build && ctest` → `tas_verify … Passed` (100%). |

## Remaining in Phase 0 (need a full build)

- **Finish the InputFrame de-dup.** With the CMake target in place, `tools/tas` can `target_link_libraries(... firelight_input)` (or just add its include dir, as here) and replace the `fltm.hpp` mirror with `firelight::input::InputFrame` directly. Deferred out of the standalone `build.sh` path because the canonical header pulls a transitive include tree.
- **App-vs-CLI equivalence test.** Prove `EmulatorInstance::runFrame()` from a fixed input log is framebuffer-bit-identical to this CLI's core wrapper — the gate that lets the verify oracle protect the *real* pipeline. Needs the Firelight emulation lib linked (a small gtest under `fl_test`), so it lands with the Phase 1 integration.

## Build

- Standalone (unchanged): `sh tools/tas/build.sh`
- Via CMake: `cmake -S tools/tas -B build/tas && cmake --build build/tas`
- In-tree: `cmake -B build -DFL_BUILD_TAS=ON …` then `ctest` runs `tas_verify`.

## The determinism gate

`tests/` holds a **self-authored** minimal Game Boy ROM (`make_testrom.py` → `testrom.gb`:
a tiny deterministic loop; only the required 48-byte boot logo is non-original) and a
**checkpoint-free** reference movie (`reference.fltm`). The gate asserts `verify`'s
replay-×2 self-check — replaying identical input twice reproduces identical framebuffer
hashes — which is portable across core builds and platforms. It deliberately does *not*
compare stored checkpoints, so a Windows-generated reference can't false-fail on the
Linux runner. Regenerate after a tooling change with:

```
python tools/tas/tests/make_testrom.py tools/tas/tests/testrom.gb
tas_movie gen  <core> tools/tas/tests/testrom.gb ref.fltm 600 1   # then strip checkpoints
tas_movie verify <core> tools/tas/tests/testrom.gb tools/tas/tests/reference.fltm
```

**Verified locally:** the ROM boots + runs deterministically in mGBA; `ctest` builds and
runs `tas_verify` → Passed (Windows/g++). **Not exercised locally** (no CI runner / clang /
Linux here): the actual CI `clang` build of the target and the Linux `.so` path. The code
is standard C++20 with a `#ifdef _WIN32`/`dlopen` split and links `${CMAKE_DL_LIBS}`, so the
risk is low; the first CI run on `tas-testing` is the confirmation.

## To strengthen later

A second, core-**pinned** gate (keep the stored checkpoints, hash the core `.dll` into the
movie's sync-manifest per roadmap Phase 1) would also catch *emulation-behavior* changes,
not just non-determinism — at the cost of re-baselining on every core bump.

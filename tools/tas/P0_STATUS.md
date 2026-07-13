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
| **Determinism gate** | `add_test(tas_verify)` runs the reference movie through `tas_movie verify` (double-replay + framebuffer-hash compare). It auto-registers under the CI workflows' existing `ctest --output-on-failure` step once `FL_BUILD_TAS=ON` — no workflow edit needed. Skips cleanly when a core/ROM/movie aren't provided. | `cmake -P tests/run_verify.cmake` PASSes over the E4 reference movie and SKIPs with no inputs. |

## Remaining in Phase 0 (need a full build / more assets)

- **Activate the gate in CI.** The `ctest` step already exists; it needs (a) `FL_BUILD_TAS=ON` in the two workflows and (b) a **redistributable** GB test ROM + a reference movie recorded over it, set via `FL_TAS_TEST_CORE` / `FL_TAS_TEST_ROM` / `FL_TAS_TEST_MOVIE`. Copyrighted ROMs can't be committed, so the current reference run (a Pokémon Yellow movie) verifies locally but not on CI runners — hence the skip-if-absent design. Source a homebrew/public-domain ROM, `tas_movie gen` a movie over it, commit both.
- **Finish the InputFrame de-dup.** With the CMake target in place, `tools/tas` can `target_link_libraries(... firelight_input)` (or just add its include dir, as here) and replace the `fltm.hpp` mirror with `firelight::input::InputFrame` directly. Deferred out of the standalone `build.sh` path because the canonical header pulls a transitive include tree.
- **App-vs-CLI equivalence test.** Prove `EmulatorInstance::runFrame()` from a fixed input log is framebuffer-bit-identical to this CLI's core wrapper — the gate that lets the verify oracle protect the *real* pipeline. Needs the Firelight emulation lib linked (a small gtest under `fl_test`), so it lands with the Phase 1 integration.

## Build

- Standalone (unchanged): `sh tools/tas/build.sh`
- Via CMake: `cmake -S tools/tas -B build/tas && cmake --build build/tas`
- In-tree: `cmake -B build -DFL_BUILD_TAS=ON …` then `ctest` runs `tas_verify`.

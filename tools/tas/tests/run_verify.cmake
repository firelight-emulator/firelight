# Portable CTest runner for the TAS determinism gate.
#
# Replays a reference .fltm movie twice through `tas_movie verify` and asserts the
# two runs produce identical framebuffer-hash checkpoints (i.e. the movie is
# deterministic and in sync). This is the POC's genuine strength, promoted to a
# merge gate.
#
# Invoked by CMake as:
#   cmake -DTAS_MOVIE_EXE=... -DTAS_CORE=... -DTAS_ROM=... -DTAS_MOVIE=... -P run_verify.cmake
#
# The core .dll, the ROM, and the reference movie are supplied via the
# FL_TAS_TEST_CORE / FL_TAS_TEST_ROM / FL_TAS_TEST_MOVIE environment variables
# (see tools/tas/CMakeLists.txt). When they are unset or missing the test SKIPS
# rather than fails -- CI runners don't ship copyrighted ROMs. Point them at a
# redistributable GB test ROM + a movie recorded over it to make this a hard gate
# everywhere.

if(NOT TAS_CORE OR NOT TAS_ROM OR NOT TAS_MOVIE
   OR NOT EXISTS "${TAS_CORE}" OR NOT EXISTS "${TAS_ROM}" OR NOT EXISTS "${TAS_MOVIE}")
  message(STATUS "tas_verify: SKIP — set FL_TAS_TEST_CORE / FL_TAS_TEST_ROM / "
                 "FL_TAS_TEST_MOVIE to a core, ROM, and reference movie to enable "
                 "the determinism gate.")
  return()
endif()

execute_process(
  COMMAND "${TAS_MOVIE_EXE}" verify "${TAS_CORE}" "${TAS_ROM}" "${TAS_MOVIE}"
  RESULT_VARIABLE rc
  OUTPUT_VARIABLE out
  ERROR_VARIABLE  err)
message(STATUS "${out}${err}")
if(NOT rc EQUAL 0)
  message(FATAL_ERROR
    "tas_verify FAILED (rc=${rc}) — the reference movie desynced. Either the core "
    "build changed its savestate/emulation, or a change broke determinism.")
endif()
message(STATUS "tas_verify: PASS — reference movie is deterministic + in sync.")

# TAS harness tooling (roadmap Phase 0: "de-fork & CI gate").
#
# Makes the standalone tools/tas binaries first-class CMake targets so they build
# and CI-verify against the same tree as Firelight, instead of only via build.sh.
# Enabled with -DFL_BUILD_TAS=ON (OFF by default, so the normal app build is
# untouched); also configurable standalone for quick dev builds:
#   cmake -S tools/tas -B build/tas && cmake --build build/tas
#
# These are plain libretro CLI tools (no Qt), matching build.sh. They additionally
# see the canonical firelight::input::InputFrame header tree so Phase 1 can retire
# the byte-identical mirror in fltm.hpp for the real type (see the static_assert
# guard there); no Qt link is required for the header alone.

cmake_minimum_required(VERSION 3.22)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  project(firelight_tas LANGUAGES CXX)
endif()
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(FL_REPO_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../..)

add_executable(tas_movie tas_movie.cpp)
target_include_directories(tas_movie PRIVATE
  ${FL_REPO_ROOT}/include/libretro
  ${FL_REPO_ROOT}/include
  ${FL_REPO_ROOT}/libs/firelight/input/include)

add_executable(tas_determinism_test determinism_test.cpp)
target_include_directories(tas_determinism_test PRIVATE ${FL_REPO_ROOT}/include/libretro)

if(NOT MSVC)
  target_compile_options(tas_movie PRIVATE -Wall -Wextra)
  target_compile_options(tas_determinism_test PRIVATE -Wall -Wextra)
endif()

# Determinism merge gate. Registered under the project's existing `ctest` step
# (the CI workflows already run `ctest --output-on-failure`), so no workflow edit
# is needed to activate it once FL_BUILD_TAS=ON. Skips when the core/ROM/movie
# env vars are absent (CI runners have no ROM) — see tests/run_verify.cmake.
enable_testing()
add_test(NAME tas_verify COMMAND ${CMAKE_COMMAND}
  -DTAS_MOVIE_EXE=$<TARGET_FILE:tas_movie>
  -DTAS_CORE=$ENV{FL_TAS_TEST_CORE}
  -DTAS_ROM=$ENV{FL_TAS_TEST_ROM}
  -DTAS_MOVIE=$ENV{FL_TAS_TEST_MOVIE}
  -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/run_verify.cmake)

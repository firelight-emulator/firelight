#!/usr/bin/env bash
# Build the standalone libretro determinism tester (gate 1a) with MSYS2 mingw64 g++.
# No Qt deps -- the libretro.h header, the C++ stdlib, and the header-only canonical
# input type (firelight::input::InputFrame + its IRetroPad interface, both Qt-free).
set -euo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO="$(cd "$HERE/../.." && pwd)"
CXX="${CXX:-C:/msys64/mingw64/bin/g++.exe}"
# g++ spawns cc1plus/as, which need the mingw64 runtime DLLs on PATH. Under a
# POSIX shell (Git Bash / MSYS2) the PATH entry must be in Unix form (/c/...)
# or the child process can't resolve its DLLs.
CXX_DIR="$(dirname "$CXX")"
if command -v cygpath >/dev/null 2>&1; then
  CXX_DIR="$(cygpath -u "$CXX_DIR")"
fi
export PATH="$CXX_DIR:$PATH"

build() { # <src> <out>
  # Include roots (all header-only, no Qt): libretro.h; the repo include/ tree
  # (firelight/libretro/retropad.hpp, which #includes "libretro/libretro.h"); and
  # the canonical input header tree (firelight/input/input_frame.hpp).
  "$CXX" -std=c++20 -O2 -Wall -Wextra \
    -I"$REPO/include/libretro" \
    -I"$REPO/include" \
    -I"$REPO/libs/firelight/input/include" \
    "$HERE/$1" -o "$HERE/$2"
  echo "built: $HERE/$2"
}

build determinism_test.cpp determinism_test.exe
build tas_movie.cpp tas_movie.exe

echo
echo "run e.g. (Game Boy -> use the mGBA core; gate 1a: gambatte savestates unreliable):"
echo "  \"$HERE/determinism_test.exe\" \"$REPO/_cores/windows/mgba_libretro.dll\" <rom.gbc> 4000 12"
echo "  \"$HERE/tas_movie.exe\" gen    \"$REPO/_cores/windows/mgba_libretro.dll\" <rom.gbc> out.fltm 4000 1"
echo "  \"$HERE/tas_movie.exe\" verify \"$REPO/_cores/windows/mgba_libretro.dll\" <rom.gbc> out.fltm"

#!/usr/bin/env bash
# Build the standalone libretro determinism tester (gate 1a) with MSYS2 mingw64 g++.
# No Qt/Firelight deps -- just the libretro.h header and the C++ stdlib.
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

"$CXX" -std=c++20 -O2 -Wall -Wextra \
  -I"$REPO/include/libretro" \
  "$HERE/determinism_test.cpp" \
  -o "$HERE/determinism_test.exe"

echo "built: $HERE/determinism_test.exe"
echo
echo "run e.g.:"
echo "  \"$HERE/determinism_test.exe\" \"$REPO/_cores/windows/gambatte_libretro.dll\" /path/to/pokemon_yellow.gbc 4000 1"

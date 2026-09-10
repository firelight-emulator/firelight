#!/usr/bin/env bash
# Builds every simulator into ./out. Programs that link production pacing code are compiled
# straight from src/app/emulation; nothing here needs Qt.
set -euo pipefail

cd "$(dirname "$0")"
root=../..
emulation=$root/src/app/emulation
mkdir -p out

production="$emulation/frame_pacer.cpp $emulation/refresh_counter.cpp $emulation/emulation_rate_controller.cpp"

# shellcheck disable=SC2086
g++ -std=c++20 -O2 -I "$emulation" noise_sweep.cpp $production -o out/noise_sweep
g++ -std=c++20 -O2 policies.cpp -o out/policies
g++ -std=c++20 -O2 phase.cpp -o out/phase
g++ -std=c++20 -O2 modes.cpp -o out/modes

echo "built: out/noise_sweep out/policies out/phase out/modes"

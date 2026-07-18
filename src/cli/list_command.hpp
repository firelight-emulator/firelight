#pragma once

namespace firelight::cli {

// TODO
// `--list-settings`: prints every emulation setting key usable with `--set`
// (common frontend settings + each core's friendly settings), with type,
// default, and allowed values. Returns a process exit code
int runListSettings(int argc, char **argv);

// `--list-cores`: prints the known libretro cores and the platforms each runs
int runListCores(int argc, char **argv);

} // namespace firelight::cli

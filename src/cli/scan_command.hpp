#pragma once

#include "cli/cli_app.hpp"

namespace firelight::cli {

// Runs a one-shot library scan headlessly (no GUI / QML engine): resolves the
// same data directories the GUI uses, opens the library store, scans the
// content directories, prints a summary, and returns a process exit code.
int runScan(int argc, char **argv, const CliOptions &opts);

} // namespace firelight::cli

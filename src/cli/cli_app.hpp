#pragma once

#include <string>

namespace firelight::cli {

// What the parsed command line asks the app to do.
enum class CliAction {
  RunGui,  // launch the normal GUI (optionally auto-launching romPath)
  RunScan, // run the headless `scan` subcommand, then exit
  Exit,    // CLI already handled output (--help/--version) or errored; use exitCode
};

// The result of parsing argv. A plain struct (no Qt, no CLI11 in the header) so
// it's trivially unit-testable.
struct CliOptions {
  CliAction action = CliAction::RunGui;
  int exitCode = 0; // meaningful only when action == Exit

  bool fullscreen = false;
  bool verbose = false;
  bool portable = false;
  std::string configDir; // --config-dir; empty = platform default
  std::string romPath;   // positional content path; empty = none
};

// Parses argv with CLI11. On --help/--version or a parse error, prints the
// appropriate output and returns action == Exit with the process exit code.
CliOptions parseCli(int argc, char **argv);

} // namespace firelight::cli

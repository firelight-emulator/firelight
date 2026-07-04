#pragma once

#include <string>
#include <utility>
#include <vector>

namespace firelight::cli {

// What the parsed command line asks the app to do.
enum class CliAction {
  RunGui,       // launch the normal GUI (optionally auto-launching romPath)
  RunScan,      // run the headless `scan` subcommand, then exit
  Login,        // run the headless `login` subcommand, then exit
  ListSettings, // print the emulation setting catalog, then exit
  ListCores,    // print the known cores, then exit
  Exit,         // CLI already handled output (--help/--version) or errored
};

// The result of parsing argv. A plain struct (no Qt, no CLI11 in the header) so
// it's trivially unit-testable.
struct CliOptions {
  CliAction action = CliAction::RunGui;
  int exitCode = 0; // meaningful only when action == Exit

  bool fullscreen = false;
  bool windowed = false; // --windowed (overrides a saved fullscreen)
  bool verbose = false;
  bool portable = false;
  bool muted = false;  // --muted (start with audio muted)
  bool paused = false; // --paused (start the game paused)
  bool exitOnClose = false;    // --exit-on-close (quit when the game closes)
  bool singleInstance = false; // --single-instance (forward to a running app)
  int saveSlot = -1;  // --save-slot N; -1 = use the entry's active slot
  std::string configDir; // --config-dir; empty = platform default
  std::string romPath;   // positional content path; empty = none
  std::string settingsFile; // --settings-file; bulk emulation overrides
  std::string controller;   // --controller NAME; per-launch preferred type
  std::string core;         // --core NAME; per-launch core override

  // Inline `--set key=value` emulation overrides (already split on the first
  // '='), applied for this launch only.
  std::vector<std::pair<std::string, std::string>> sets;

  // RetroAchievements credentials, from the startup `--ra-*` flags or the
  // `login` subcommand's `--username/--password/--token`.
  std::string raUsername;
  std::string raPassword;
  std::string raToken;
};

// Parses argv with CLI11. On --help/--version or a parse error, prints the
// appropriate output and returns action == Exit with the process exit code.
CliOptions parseCli(int argc, char **argv);

} // namespace firelight::cli

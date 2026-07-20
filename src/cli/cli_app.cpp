#include "cli/cli_app.hpp"

#include <CLI11.hpp>
#include <iostream>

namespace firelight::cli {

CliOptions parseCli(int argc, char **argv) {
  CliOptions opts;

  CLI::App app{"Firelight - a libretro emulation frontend"};
  // Only recognize --long / -short options; otherwise CLI11 treats a leading
  // "/" as a Windows-style option flag and mis-parses POSIX-style ROM paths
  // (e.g. "/roms/game.gba")
  app.allow_windows_style_options(false);
#ifndef FL_VERSION
#define FL_VERSION "dev"
#endif
  app.set_version_flag("--version", std::string("Firelight ") + FL_VERSION);

  auto *fullscreenFlag = app.add_flag("-f,--fullscreen", opts.fullscreen, "Start in fullscreen");
  app.add_flag("--windowed", opts.windowed, "Start windowed (overrides a saved fullscreen preference)")
      ->excludes(fullscreenFlag);
  app.add_flag("-v,--verbose", opts.verbose, "Enable debug logging");
  app.add_flag("--portable", opts.portable, "Store all data next to the executable");
  app.add_flag("--muted", opts.muted, "Start with audio muted");
  app.add_flag("--paused", opts.paused, "Start the game paused");
  app.add_flag("--exit-on-close", opts.exitOnClose, "Quit Firelight when the launched game closes");
  app.add_flag("--single-instance", opts.singleInstance,
               "Forward the launch to a running Firelight instead of opening a "
               "second window");
  app.add_flag("--fatal-warnings", opts.fatalWarnings, "Exit non-zero if any QML warning or error is logged");
  app.add_option("--route", opts.startupRoute, "Open this route on startup instead of /library")->type_name("PATH");
  app.add_option("--save-slot", opts.saveSlot, "Save slot to load for the launched game")->type_name("N");
  app.add_option("--controller", opts.controller, "Preferred controller type for the launched game")->type_name("NAME");
  app.add_option("--core", opts.core, "Force a specific libretro core for the launched game")->type_name("NAME");
  app.add_option("--config-dir", opts.configDir, "Use this directory for all app data")->type_name("DIR");
  app.add_option("--settings-file", opts.settingsFile,
                 "Load emulation setting overrides from a properties or JSON "
                 "file")
      ->type_name("FILE");

  // Repeatable inline emulation overrides, collected raw then split below
  std::vector<std::string> rawSets;
  app.add_option("--set", rawSets,
                 "Override an emulation setting for this launch (key=value; "
                 "repeatable)")
      ->type_name("KEY=VALUE");

  // RetroAchievements startup login flags (a GUI launch logs in on start)
  app.add_option("--ra-username", opts.raUsername, "RetroAchievements username to log in at startup")
      ->type_name("USER");
  app.add_option("--ra-password", opts.raPassword, "RetroAchievements password")->type_name("PASS");
  app.add_option("--ra-token", opts.raToken, "RetroAchievements login token")->type_name("TOKEN");

  bool listSettings = false;
  bool listCores = false;
  app.add_flag("--list-settings", listSettings, "List every emulation setting key usable with --set, then exit");
  app.add_flag("--list-cores", listCores, "List the known libretro cores, then exit");

  app.add_option("rom", opts.romPath, "Content file to launch")->type_name("ROM");

  auto *scan = app.add_subcommand("scan", "Rescan content directories, update the library, then exit");

  auto *verifyUi = app.add_subcommand("verify-ui", "Mount every registered route, report failures, then exit");
  // So `verify-ui --fatal-warnings` works, not just `--fatal-warnings verify-ui`
  verifyUi->fallthrough();
  verifyUi->add_option("--route", opts.verifyRoutes, "Sweep only this route (repeatable)")->type_name("PATH");
  verifyUi->add_option("--settle-ms", opts.settleMs, "How long to wait for a route to finish building")->type_name("N");
  verifyUi->add_flag("--json", opts.json, "Emit the report as JSON");

  auto *login = app.add_subcommand("login", "Log in to RetroAchievements (persists a token), then exit");
  login->add_option("--username", opts.raUsername, "RetroAchievements username")->type_name("USER");
  login->add_option("--password", opts.raPassword, "RetroAchievements password")->type_name("PASS");
  login->add_option("--token", opts.raToken, "RetroAchievements login token")->type_name("TOKEN");

  try {
    app.parse(argc, argv);
  } catch (const CLI::ParseError &e) {
    opts.action = CliAction::Exit;
    opts.exitCode = app.exit(e);
    return opts;
  }

  // Split inline `--set` items into key/value pairs (a missing '=' or empty key
  // is a usage error)
  for (const auto &item : rawSets) {
    const auto eq = item.find('=');
    if (eq == std::string::npos || eq == 0) {
      std::cerr << "Error: --set expects KEY=VALUE, got: " << item << "\n";
      opts.action = CliAction::Exit;
      opts.exitCode = 1;
      return opts;
    }
    opts.sets.emplace_back(item.substr(0, eq), item.substr(eq + 1));
  }

  if (scan->parsed()) {
    opts.action = CliAction::RunScan;
  } else if (verifyUi->parsed()) {
    opts.action = CliAction::VerifyUi;
  } else if (login->parsed()) {
    opts.action = CliAction::Login;
  } else if (listSettings) {
    opts.action = CliAction::ListSettings;
  } else if (listCores) {
    opts.action = CliAction::ListCores;
  }
  return opts;
}

} // namespace firelight::cli

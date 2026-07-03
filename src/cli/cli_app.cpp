#include "cli/cli_app.hpp"

#include <CLI11.hpp>

namespace firelight::cli {

CliOptions parseCli(int argc, char **argv) {
  CliOptions opts;

  CLI::App app{"Firelight - a libretro emulation frontend"};
  // Only recognize --long / -short options; otherwise CLI11 treats a leading
  // "/" as a Windows-style option flag and mis-parses POSIX-style ROM paths
  // (e.g. "/roms/game.gba").
  app.allow_windows_style_options(false);
  app.set_version_flag("--version", std::string("Firelight ") + "dev");

  app.add_flag("-f,--fullscreen", opts.fullscreen, "Start in fullscreen");
  app.add_flag("-v,--verbose", opts.verbose, "Enable debug logging");
  app.add_flag("--portable", opts.portable,
               "Store all data next to the executable");
  app.add_option("--config-dir", opts.configDir,
                 "Use this directory for all app data")
      ->type_name("DIR");
  app.add_option("rom", opts.romPath, "Content file to launch")
      ->type_name("ROM");

  auto *scan = app.add_subcommand(
      "scan", "Rescan content directories, update the library, then exit");

  try {
    app.parse(argc, argv);
  } catch (const CLI::ParseError &e) {
    // Prints help/version/error text and yields the process exit code.
    opts.action = CliAction::Exit;
    opts.exitCode = app.exit(e);
    return opts;
  }

  if (scan->parsed()) {
    opts.action = CliAction::RunScan;
  }
  return opts;
}

} // namespace firelight::cli

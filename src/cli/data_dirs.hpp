#pragma once

#include "cli/cli_app.hpp"

#include <QString>

namespace firelight::cli {
  // The app's resolved data directories. `appDataPath` holds config + databases;
  // `docsPath` holds user content (roms/saves live under it)
  struct DataDirs {
    QString docsPath;
    QString appDataPath;
    QString romsPath; // the default content directory
    QString savesPath;
    QString capturesPath; // gameplay captures: screenshots/ + clips/ subdirs
    QString coreSystemPath; // shared core-system directory (PPSSPP, etc.)
  };

  // Resolves the data directories, honoring --config-dir / --portable (and the
  // legacy portable.txt marker next to the executable), else the platform
  // defaults. Requires a live QCoreApplication (uses applicationDirPath())
  DataDirs resolveDataDirs(const CliOptions &opts);
} // namespace firelight::cli

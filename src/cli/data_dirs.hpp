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
  QString capturesPath;   // gameplay captures: screenshots/ + clips/ subdirs
  QString coreSystemPath; // shared core-system directory (PPSSPP, etc.)
};

// Resolves the data directories, honoring --config-dir / --portable (and the
// legacy portable.txt marker next to the executable), else the platform
// defaults. Requires a live QCoreApplication (uses applicationDirPath())
DataDirs resolveDataDirs(const CliOptions &opts);

// Seeds core runtime assets bundled with the install (PPSSPP fonts, VFPU tables,
// atlases) into the writable core-system directory. Idempotent: keyed on a
// marker asset so it only copies when the destination is unseeded. Any path that
// can launch a game calls this before doing so. Requires a live QCoreApplication
void provisionCoreAssets(const DataDirs &dirs);
} // namespace firelight::cli

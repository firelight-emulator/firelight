#include "cli/data_dirs.hpp"

#include <QCoreApplication>
#include <QFileInfo>
#include <QStandardPaths>
#include <filesystem>
#include <spdlog/spdlog.h>

namespace firelight::cli {
DataDirs resolveDataDirs(const CliOptions &opts) {
  DataDirs dirs;

  const bool portableMarker = QFileInfo(QCoreApplication::applicationDirPath() + "/portable.txt").exists();

  if (!opts.configDir.empty()) {
    dirs.docsPath = QString::fromStdString(opts.configDir);
    dirs.appDataPath = dirs.docsPath + "/appdata";
  } else if (opts.portable || portableMarker) {
    dirs.docsPath = QCoreApplication::applicationDirPath();
    dirs.appDataPath = dirs.docsPath + "/appdata";
  } else {
    dirs.docsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/Firelight";
    dirs.appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  }

  dirs.coreSystemPath = dirs.appDataPath + "/core-system";

  dirs.romsPath = dirs.docsPath + "/roms";
  dirs.savesPath = dirs.docsPath + "/saves";
  dirs.capturesPath = dirs.docsPath + "/captures";
  return dirs;
}

void provisionCoreAssets(const DataDirs &dirs) {
  namespace fs = std::filesystem;

  // PPSSPP loads a runtime asset tree (fonts, VFPU tables, texture atlases,
  // compat db) from <system>/PPSSPP. These aren't part of the core DLL, so PSP
  // games can't run without them
  const fs::path dest = fs::path(dirs.coreSystemPath.toStdString()) / "PPSSPP";
  const fs::path src = fs::path(QCoreApplication::applicationDirPath().toStdString()) / "system" / "PPSSPP";

  std::error_code ec;
  if (fs::exists(dest / "ppge_atlas.zim", ec) || !fs::exists(src / "ppge_atlas.zim", ec)) {
    return;
  }

  spdlog::info("Seeding PPSSPP assets: {} -> {}", src.string(), dest.string());
  fs::copy(src, dest, fs::copy_options::recursive | fs::copy_options::overwrite_existing, ec);
  if (ec) {
    spdlog::warn("Failed to seed PPSSPP assets: {}", ec.message());
  }
}
} // namespace firelight::cli

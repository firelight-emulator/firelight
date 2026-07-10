#include "cli/data_dirs.hpp"

#include <QCoreApplication>
#include <QFileInfo>
#include <QStandardPaths>

namespace firelight::cli {
  DataDirs resolveDataDirs(const CliOptions &opts) {
    DataDirs dirs;

    const bool portableMarker =
        QFileInfo(QCoreApplication::applicationDirPath() + "/portable.txt")
        .exists();

    if (!opts.configDir.empty()) {
      dirs.docsPath = QString::fromStdString(opts.configDir);
      dirs.appDataPath = dirs.docsPath + "/appdata";
    } else if (opts.portable || portableMarker) {
      dirs.docsPath = QCoreApplication::applicationDirPath();
      dirs.appDataPath = dirs.docsPath + "/appdata";
    } else {
      dirs.docsPath =
          QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) +
          "/Firelight";
      dirs.appDataPath =
          QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    }

    dirs.coreSystemPath = dirs.appDataPath + "/core-system";

    dirs.romsPath = dirs.docsPath + "/roms";
    dirs.savesPath = dirs.docsPath + "/saves";
    dirs.capturesPath = dirs.docsPath + "/captures";
    dirs.shadersPath = dirs.docsPath + "/shaders";
    dirs.bordersPath = dirs.docsPath + "/borders";
    return dirs;
  }
} // namespace firelight::cli

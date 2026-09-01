// TODO: NEEDS REVIEW
#include "cli/scan_command.hpp"

#include "cli/data_dirs.hpp"

#include <firelight/library/disc_set_service.hpp>
#include <firelight/library/library_ingest_service.hpp>
#include <firelight/library/library_scanner2.hpp>
#include <firelight/library/sqlite_user_library.hpp>
#include <firelight/library/user_library_service.hpp>
#include <firelight/platforms/platform_service.hpp>

#include <QCoreApplication>
#include <QDir>
#include <QEventLoop>
#include <cstdio>

namespace firelight::cli {

int runScan(int argc, char **argv, const CliOptions &opts) {
  // A Core (non-GUI) application: the scanner uses QtConcurrent + signals, so we
  // need an event loop, but no window/QML
  QCoreApplication app(argc, argv);

  const auto dirs = resolveDataDirs(opts);
  QDir().mkpath(dirs.appDataPath);
  QDir().mkpath(dirs.romsPath);

  library::SqliteUserLibraryRepository repository(dirs.appDataPath + "/library.db");
  // Turns scanned content files into run configurations + entries; must outlive
  // the scan
  library::DiscSetService discSets(repository, "");
  library::LibraryIngestService ingest(repository, discSets);
  platforms::PlatformService platformService;
  library::LibraryScanner2 scanner(repository, platformService, nullptr, &discSets);
  // Guarantees the default content directory exists and is registered before we
  // scan (a fresh install has nothing to scan otherwise)
  library::UserLibraryService libraryService(repository, dirs.romsPath.toStdString());

  QEventLoop loop;
  // TODO
  // scanFinished is only emitted when the scan changed something, so waiting on it never returns
  // for a rescan that finds nothing new. The scanning flag going back to false is true either way
  QObject::connect(&scanner, &library::LibraryScanner2::scanningChanged, &loop, [&] {
    if (!scanner.isScanning()) {
      loop.quit();
    }
  });

  std::printf("Scanning content directories...\n");
  std::fflush(stdout);
  scanner.scanAll();
  loop.exec();

  const auto entries = repository.getEntries();
  const auto contentFiles = repository.getPresentContentFiles();
  std::printf("Scan complete: %zu content file(s), %zu library entr%s.\n", contentFiles.size(), entries.size(),
              entries.size() == 1 ? "y" : "ies");
  return 0;
}

} // namespace firelight::cli

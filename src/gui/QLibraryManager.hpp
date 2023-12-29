//
// Created by alexs on 12/22/2023.
//

#ifndef FIRELIGHT_QLIBRARYMANAGER_HPP
#define FIRELIGHT_QLIBRARYMANAGER_HPP

#include "QLibEntryModelShort.hpp"
#include "src/app/db/library_database.hpp"
#include "src/app/library/library_manager.hpp"

#include <QObject>
class QLibraryManager final : public QObject {
  Q_OBJECT

public:
  struct ScanResults {
    std::vector<std::string> all_md5s;
    std::vector<LibEntry> existing_entries;
    std::vector<LibEntry> new_entries;
  };

  explicit QLibraryManager(LibraryDatabase *lib_database,
                           std::filesystem::path default_rom_path,
                           ContentDatabase *content_database,
                           QLibEntryModelShort *shortModel);

public slots:
  void startScan();

signals:
  void entryAdded(int entryId);
  void entryRemoved(int entryId);
  void entryModified(int entryId);

  void scanStarted();
  void scanFinished();

private:
  std::filesystem::path default_rom_path_;
  const int thread_pool_size_ = 1;
  LibraryDatabase *library_database_;
  ContentDatabase *content_database_;
  QLibEntryModelShort *m_shortModel;

  std::unique_ptr<QThreadPool> scanner_thread_pool_ = nullptr;
};

#endif // FIRELIGHT_QLIBRARYMANAGER_HPP

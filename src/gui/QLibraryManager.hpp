//
// Created by alexs on 12/22/2023.
//

#ifndef FIRELIGHT_QLIBRARYMANAGER_HPP
#define FIRELIGHT_QLIBRARYMANAGER_HPP

#include "../app/db/content_database.hpp"
#include "../app/db/library_database.hpp"
#include "QLibraryViewModel.hpp"
#include <QFileSystemWatcher>
#include <filesystem>

#include <QObject>
#include <QThreadPool>
class QLibraryManager final : public QObject {
  Q_OBJECT
  Q_PROPERTY(bool scanning READ scanning NOTIFY scanningChanged)

public:
  struct ScanResults {
    std::vector<std::string> all_md5s;
    std::vector<LibEntry> existing_entries;
    std::vector<LibEntry> new_entries;
  };

  explicit QLibraryManager(LibraryDatabase *lib_database,
                           std::filesystem::path default_rom_path,
                           ContentDatabase *content_database,
                           QLibraryViewModel *model);

  std::optional<LibEntry> get_by_id(int id) const;

public slots:
  void startScan();
  bool scanning();

signals:
  void entryAdded(int entryId);
  void entryRemoved(int entryId);
  void entryModified(int entryId);

  void scanStarted();
  void scanFinished();

  void scanningChanged();

private:
  bool scanning_ = false;
  std::filesystem::path default_rom_path_;
  const int thread_pool_size_ = 1;
  LibraryDatabase *library_database_;
  ContentDatabase *content_database_;
  QFileSystemWatcher directory_watcher_;
  QLibraryViewModel *model_;

  std::vector<QLibraryViewModel::Item> get_model_items_() const;

  std::unique_ptr<QThreadPool> scanner_thread_pool_ = nullptr;
};

#endif // FIRELIGHT_QLIBRARYMANAGER_HPP

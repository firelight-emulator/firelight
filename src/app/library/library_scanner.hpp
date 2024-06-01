#pragma once

#include "firelight/content_database.hpp"
#include "firelight/library_database.hpp"
#include <QFileSystemWatcher>
#include <QObject>
#include <QThreadPool>
#include <filesystem>

#include "../../gui/library_path_model.hpp"

class LibraryScanner final : public QObject {
  Q_OBJECT
  Q_PROPERTY(bool scanning READ scanning NOTIFY scanningChanged)

public:
  struct ScanResults {
    std::vector<std::string> all_md5s;
    std::vector<std::string> all_filenames;
    std::vector<firelight::db::LibraryEntry> existing_entries;
    std::vector<firelight::db::LibraryEntry> new_entries;
  };

  explicit LibraryScanner(firelight::db::ILibraryDatabase *lib_database,
                          firelight::db::IContentDatabase *content_database);

  QAbstractListModel *scanDirectoryModel() const;

public slots:
  void startScan();

  bool scanning() const;

signals:
  void entryAdded(int entryId);

  void entryRemoved(int entryId);

  void entryModified(int entryId);

  void scanStarted();

  void scanFinished();

  void scanningChanged();

private:
  bool scanning_ = false;
  const int thread_pool_size_ = 1;
  firelight::db::ILibraryDatabase *library_database_;
  firelight::db::IContentDatabase *content_database_;
  QFileSystemWatcher directory_watcher_;

  firelight::gui::LibraryPathModel *m_scanDirectoryModel = nullptr;
  std::unique_ptr<QThreadPool> scanner_thread_pool_ = nullptr;

  void refreshDirectories();

  void handleScannedPatchFile(const std::filesystem::directory_entry &entry,
                              ScanResults &scan_results) const;
};

#pragma once

#include "../app/db/content_database.hpp"
#include "../app/db/library_database.hpp"
#include <QFileSystemWatcher>
#include <QObject>
#include <QThreadPool>
#include <filesystem>

class QLibraryManager final : public QObject {
  Q_OBJECT
  Q_PROPERTY(bool scanning READ scanning NOTIFY scanningChanged)

public:
  struct ScanResults {
    std::vector<std::string> all_md5s;
    std::vector<firelight::db::LibraryEntry> existing_entries;
    std::vector<firelight::db::LibraryEntry> new_entries;
  };

  explicit QLibraryManager(firelight::db::ILibraryDatabase *lib_database,
                           std::filesystem::path default_rom_path,
                           IContentDatabase *content_database);

  [[nodiscard]] std::optional<LibEntry> get_by_id(int id) const;
  [[nodiscard]] std::optional<LibEntry> getByRomId(int id) const;

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
  std::filesystem::path default_rom_path_;
  const int thread_pool_size_ = 1;
  firelight::db::ILibraryDatabase *library_database_;
  IContentDatabase *content_database_;
  QFileSystemWatcher directory_watcher_;

  std::unique_ptr<QThreadPool> scanner_thread_pool_ = nullptr;
  void handleScannedPatchFile(const std::filesystem::directory_entry &entry,
                              ScanResults &scan_results) const;
  void handleScannedRomFile(const std::filesystem::directory_entry &entry,
                            ScanResults &scan_results) const;
};

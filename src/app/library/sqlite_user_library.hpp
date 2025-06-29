#pragma once
#include "rom_file.hpp"
#include "user_library.hpp"
#include <QSqlDatabase>
#include <QString>
#include <qsettings.h>

namespace firelight::library {
class SqliteUserLibrary final : public QObject, public IUserLibrary {
  Q_OBJECT
  Q_PROPERTY(QString mainGameDirectory READ getMainGameDirectory WRITE
                 setMainGameDirectory NOTIFY mainGameDirectoryChanged)

public:
  explicit SqliteUserLibrary(QString path, QString mainGameDirectory);

  ~SqliteUserLibrary() override;

  bool create(FolderInfo &folder) override;
  bool create(FolderEntryInfo &folderEntry) override;
  std::vector<FolderInfo> listFolders(FolderInfo filter) override;
  bool deleteFolder(int folderId) override;

  bool update(FolderInfo &folder) override;
  bool deleteFolderEntry(FolderEntryInfo &info) override;

  bool update(Entry &entry) override;

  void setMainGameDirectory(const QString &directory);

  QString getMainGameDirectory();

  void create(RomFile &romFile) override;

  std::optional<RomFile> getRomFileWithPathAndSize(const QString &filePath,
                                                   size_t fileSizeBytes,
                                                   bool inArchive) override;

  std::vector<Entry> getEntries(int offset, int limit) override;

  std::optional<Entry> getEntry(int entryId) override;

  std::vector<RunConfiguration>
  getRunConfigurations(const QString &contentHash) override;

  void doStuffWithRunConfigurations() override;

  std::vector<RomFile>
  getRomFilesWithContentHash(const QString &contentHash) override;

  std::vector<RomFile> getRomFiles() override;

  std::optional<RomFile> getRomFile(int id) override;

  std::optional<PatchFile> getPatchFile(int id) override;

  void create(PatchFile &file);

  std::optional<PatchFile> getPatchFileWithPathAndSize(const QString &filePath,
                                                       size_t fileSizeBytes,
                                                       bool inArchive);

  std::vector<PatchFile> getPatchFiles();

  std::vector<WatchedDirectory> getWatchedDirectories() override;

  bool addWatchedDirectory(const WatchedDirectory &directory) override;

  bool updateWatchedDirectory(const WatchedDirectory &directory) override;

signals:
  void mainGameDirectoryChanged(const QString &newDirectory);

  void entryAddedToFolder(int folderId, int entryId);

  void romFileAdded(int id, const QString &contentHash);

  void entryCreated(int id, const QString &contentHash);

  void watchedDirectoryAdded(int id, const QString &path);

private:
  static constexpr auto DATABASE_PREFIX = "userlibrary_";

  QString m_mainGameDirectory;

  QSettings m_settings;

  [[nodiscard]] QSqlDatabase getDatabase() const;

  QString m_databasePath;
};
} // namespace firelight::library

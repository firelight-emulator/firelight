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
  bool deleteContentDirectory(int id) override;

  QString getMainGameDirectory();

  bool create(RomFileInfo &romFile) override;

  std::optional<RomFileInfo> getRomFileWithPathAndSize(const QString &filePath,
                                                       size_t fileSizeBytes,
                                                       bool inArchive) override;

  bool deleteRomFile(int id) override;

  std::vector<Entry> getEntries(int offset, int limit) override;

  std::optional<Entry> getEntry(int entryId) override;

  std::optional<Entry>
  getEntryWithContentHash(const QString &contentHash) override;

  std::vector<RunConfiguration>
  getRunConfigurations(const QString &contentHash) override;

  void doStuffWithRunConfigurations() override;

  std::vector<RomFileInfo>
  getRomFilesWithContentHash(const QString &contentHash) override;

  std::vector<RomFileInfo> getRomFiles() override;

  std::optional<RomFileInfo> getRomFile(int id) override;

  std::optional<PatchFile> getPatchFile(int id) override;

  void create(PatchFile &file) override;

  std::optional<PatchFile> getPatchFileWithPathAndSize(const QString &filePath,
                                                       size_t fileSizeBytes,
                                                       bool inArchive) override;

  std::vector<PatchFile> getPatchFiles() override;

  std::vector<WatchedDirectory> getWatchedDirectories() override;

  bool create(WatchedDirectory &directory) override;

  bool update(const WatchedDirectory &directory) override;
  bool createEntry(Entry &entry);

signals:
  void mainGameDirectoryChanged(const QString &newDirectory);

  void entryAddedToFolder(int folderId, int entryId);

  void romFileAdded(int id, QString path, int platformId, QString contentHash);
  void romFileDeleted(int id);

  void entryCreated(int id, const QString &contentHash);

  void watchedDirectoryAdded(int id, const QString &path);
  void watchedDirectoryUpdated(int id, const QString &oldPath,
                               const QString &newPath);
  void watchedDirectoryRemoved(int id, const QString &path);

  void romRunConfigurationCreated(int id, QString path, int platformId,
                                  QString contentHash);
  void romRunConfigurationDeleted(QString contentHash);

  void entryHidden(int entryId);

private:
  static constexpr auto DATABASE_PREFIX = "userlibrary_";

  QString m_mainGameDirectory;

  QSettings m_settings;
  void createRomRunConfiguration(int romId, const QString &path, int platformId,
                                 const QString &contentHash);

  [[nodiscard]] QSqlDatabase getDatabase() const;

  QString m_databasePath;
};
} // namespace firelight::library

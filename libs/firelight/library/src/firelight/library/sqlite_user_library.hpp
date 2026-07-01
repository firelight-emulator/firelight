#pragma once

#include <firelight/library/content_file.hpp>
#include <firelight/library/user_library_repository.hpp>
#include <QSqlDatabase>
#include <QString>

namespace firelight::library {
// Persistence for the user library: owns the schema and implements all CRUD over
// its tables. Announces changes through the EventDispatcher (see
// library_events.hpp) rather than Qt signals, so it is a plain class, not a
// QObject. Its QSqlDatabase/QString internals are Qt Core value types.
class SqliteUserLibraryRepository final : public IUserLibraryRepository {
public:
  explicit SqliteUserLibraryRepository(QString path);

  ~SqliteUserLibraryRepository() override;

  bool create(FolderInfo &folder) override;
  bool create(FolderEntryInfo &folderEntry) override;
  std::vector<FolderInfo> listFolders() override;
  bool deleteFolder(int folderId) override;

  bool update(FolderInfo &folder) override;
  bool deleteFolderEntry(FolderEntryInfo &info) override;

  bool update(Entry &entry) override;

  bool deleteContentDirectory(int id) override;

  bool create(ContentFile &romFile) override;

  std::optional<ContentFile>
  getContentFileWithPathAndSize(const QString &filePath, size_t fileSizeBytes,
                                bool inArchive) override;

  bool deleteContentFile(int id) override;

  std::vector<Entry> getEntries(int offset, int limit) override;

  std::optional<Entry> getEntry(int entryId) override;

  std::optional<Entry>
  getEntryWithContentHash(const QString &contentHash) override;

  std::vector<RunConfiguration>
  getRunConfigurations(const QString &contentHash) override;

  std::vector<ContentFile> getContentFiles() override;

  std::optional<ContentFile> getContentFile(int id) override;

  std::optional<PatchFile> getPatchFile(int id) override;

  bool create(DiscMember &member) override;

  void create(PatchFile &file) override;

  std::vector<WatchedDirectory> getWatchedDirectories() override;

  bool create(WatchedDirectory &directory) override;

  bool update(const WatchedDirectory &directory) override;

  bool createEntry(Entry &entry) override;

  void createRunConfiguration(int contentFileId, const QString &path,
                              int platformId,
                              const QString &contentHash) override;

private:
  static constexpr auto DATABASE_PREFIX = "userlibrary_";

  [[nodiscard]] QSqlDatabase getDatabase() const;

  QString m_databasePath;
};
} // namespace firelight::library

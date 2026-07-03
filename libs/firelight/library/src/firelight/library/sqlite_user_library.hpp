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
  bool reorderFolders(int parentId,
                      const std::vector<int> &orderedFolderIds) override;
  bool setFolderParent(int folderId, int newParentId) override;
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

  std::vector<ContentDirectory> getContentDirectories() override;

  bool create(ContentDirectory &directory) override;

  bool update(const ContentDirectory &directory) override;

  bool createEntry(Entry &entry) override;

  void createRunConfiguration(int contentFileId, const QString &path,
                              int platformId,
                              const QString &contentHash) override;

private:
  static constexpr auto DATABASE_PREFIX = "userlibrary_";

  [[nodiscard]] QSqlDatabase getDatabase() const;

  // Adds `column` (with the given SQL type/constraints) to `table` if it does
  // not already exist, so databases created before a column was introduced pick
  // it up. No-op on fresh databases where CREATE TABLE already added it.
  void ensureColumn(const QString &table, const QString &column,
                    const QString &definition) const;

  // Resolves the content directory (content_directoriesv1.id) an on-disk path
  // belongs to by longest matching path prefix, or -1 if none matches.
  [[nodiscard]] int resolveContentDirectoryId(const QString &onDiskPath);

  // Populates entry.contentDirectoryIds + entry.contentPaths from the entry's
  // content_files (joined by content_hash). Shared by all three entry loaders.
  void populateEntryProvenance(Entry &entry);

  // One-time backfill of content_files.content_directory_id for rows added
  // before provenance tracking existed (longest path-prefix wins).
  void backfillContentDirectoryIds();

  // The next free ordering position within a parent scope (max + 1).
  int nextFolderPosition(int parentId);

  QString m_databasePath;
};
} // namespace firelight::library

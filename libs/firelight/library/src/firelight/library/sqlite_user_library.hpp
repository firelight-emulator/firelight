#pragma once

#include <firelight/library/content_file.hpp>
#include <firelight/library/user_library_repository.hpp>

#include <QString>
#include <memory>
#include <mutex>
#include <string>

namespace SQLite {
class Database;
}

namespace firelight::library {
class SqliteUserLibraryRepository final : public IUserLibraryRepository {
public:
  explicit SqliteUserLibraryRepository(QString path);

  ~SqliteUserLibraryRepository() override;

  bool create(FolderInfo &folder) override;

  bool create(FolderEntryInfo &folderEntry) override;

  std::vector<FolderInfo> listFolders() override;

  bool deleteFolder(int folderId) override;

  bool update(FolderInfo &folder) override;

  bool reorderFolders(int parentId, const std::vector<int> &orderedFolderIds) override;

  bool setFolderParent(int folderId, int newParentId) override;

  bool deleteFolderEntry(FolderEntryInfo &info) override;

  bool update(Entry &entry) override;

  bool updateEntryMetadata(const Entry &entry) override;

  bool deleteContentDirectory(int id) override;

  bool create(ContentFile &romFile) override;

  std::optional<ContentFile> getContentFileWithPathAndSize(const std::string &filePath, size_t fileSizeBytes,
                                                           bool inArchive) override;

  bool deleteContentFile(int id) override;

  std::vector<Entry> getEntries(int offset, int limit) override;

  std::optional<Entry> getEntry(int entryId) override;

  std::optional<Entry> getEntryWithContentHash(const std::string &contentHash) override;

  std::vector<RunConfiguration> getRunConfigurations(const std::string &contentHash) override;

  std::vector<ContentFile> getContentFiles() override;

  std::optional<ContentFile> getContentFile(int id) override;

  std::optional<PatchFile> getPatchFile(int id) override;

  bool create(DiscMember &member) override;

  void create(PatchFile &file) override;

  std::vector<ContentDirectory> getContentDirectories() override;

  bool create(ContentDirectory &directory) override;

  bool update(const ContentDirectory &directory) override;

  bool createEntry(Entry &entry) override;

  void createRunConfiguration(int contentFileId, const std::string &path, int platformId,
                              const std::string &contentHash) override;

private:
  void ensureColumnExists(const std::string &table, const std::string &column, const std::string &definition);

  [[nodiscard]] int resolveContentDirectoryId(const std::string &onDiskPath);

  void populateEntrySource(Entry &entry);

  void backfillContentDirectoryIds();

  int nextFolderPosition(int parentId);

  std::string m_databasePath;
  std::unique_ptr<SQLite::Database> m_db;
  mutable std::recursive_mutex m_mutex;
};
} // namespace firelight::library

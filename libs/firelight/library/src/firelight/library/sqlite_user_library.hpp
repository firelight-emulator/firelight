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

  bool applyEntryMetadata(int entryId, const GameMetadata &incoming, const std::set<std::string> &changedFields,
                          bool isUserEdit) override;

  bool markArtFetched(int entryId, uint64_t whenMillis) override;

  std::vector<int> getEntryIdsMissingArt(int limit) override;

  bool createVariantGroup(VariantGroup &group) override;

  bool updateVariantGroup(const VariantGroup &group) override;

  bool deleteVariantGroup(int groupId) override;

  std::vector<VariantGroup> getVariantGroups() override;

  std::optional<VariantGroup> getVariantGroup(int groupId) override;

  bool setEntryVariantGroup(int entryId, std::optional<int> groupId, bool isUserChoice) override;

  std::vector<int> getEntryIdsWithNormalizedTitle(unsigned platformId, const std::string &normalizedTitle) override;

  std::vector<Entry> getEntriesInVariantGroup(int groupId) override;

  bool createTag(Tag &tag) override;

  bool renameTag(int tagId, const std::string &name) override;

  bool mergeTags(int sourceTagId, int targetTagId) override;

  bool deleteTag(int tagId) override;

  std::vector<Tag> getTags() override;

  bool setEntryTags(int entryId, const std::vector<int> &tagIds) override;

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

#pragma once

#include <firelight/library/content_file.hpp>
#include <firelight/library/disc_member.hpp>
#include <firelight/library/entry.hpp>
#include <firelight/library/folder_entry_info.hpp>
#include <firelight/library/folder_info.hpp>
#include <firelight/library/patch_file.hpp>
#include <firelight/library/run_configuration.hpp>
#include <firelight/library/content_directory.hpp>

#include <optional>
#include <vector>

namespace firelight::library {

struct EntryUpdatedEvent {
  int entryId;
};

// The user library's persistence contract: all CRUD over the library's tables.
// Implemented by SqliteUserLibraryRepository and consumed by the library's own
// collaborators (scanner, ingest, resolver) and the UserLibraryService.
class IUserLibraryRepository {
public:
  virtual ~IUserLibraryRepository() = default;

  virtual bool create(ContentFile &contentFile) = 0;
  virtual void create(PatchFile &file) = 0;
  virtual bool create(FolderInfo &folder) = 0;
  virtual bool create(FolderEntryInfo &folderEntry) = 0;
  virtual bool create(ContentDirectory &directory) = 0;
  virtual bool create(DiscMember &member) = 0;

  // Inserts an entry row (setting entry.id). Distinct from update(Entry&), which
  // mutates an existing row.
  virtual bool createEntry(Entry &entry) = 0;
  // Inserts a run configuration linking a content file (and optional patch) to a
  // content hash.
  virtual void createRunConfiguration(int contentFileId, const QString &path,
                                      int platformId,
                                      const QString &contentHash) = 0;

  virtual bool update(FolderInfo &folder) = 0;
  virtual bool update(Entry &entry) = 0;
  virtual bool update(const ContentDirectory &directory) = 0;

  virtual std::vector<FolderInfo> listFolders() = 0;
  virtual bool deleteFolder(int folderId) = 0;
  virtual bool deleteFolderEntry(FolderEntryInfo &info) = 0;

  // Reassigns manual ordering positions (0..n-1) to the given folders within a
  // parent scope (parentId -1 = root). Ids not under parentId are ignored.
  virtual bool reorderFolders(int parentId,
                              const std::vector<int> &orderedFolderIds) = 0;
  // Moves a folder under a new parent (-1 = root), appended at the end.
  virtual bool setFolderParent(int folderId, int newParentId) = 0;

  virtual bool deleteContentDirectory(int id) = 0;

  virtual std::optional<ContentFile>
  getContentFileWithPathAndSize(const QString &filePath, size_t fileSizeBytes,
                                bool inArchive) = 0;
  virtual std::vector<ContentFile> getContentFiles() = 0;
  virtual std::optional<ContentFile> getContentFile(int id) = 0;
  virtual bool deleteContentFile(int id) = 0;

  virtual std::optional<PatchFile> getPatchFile(int id) = 0;

  virtual std::vector<Entry> getEntries(int offset, int limit) = 0;
  virtual std::optional<Entry> getEntry(int entryId) = 0;
  virtual std::optional<Entry>
  getEntryWithContentHash(const QString &contentHash) = 0;

  virtual std::vector<RunConfiguration>
  getRunConfigurations(const QString &contentHash) = 0;

  virtual std::vector<ContentDirectory> getContentDirectories() = 0;
};
} // namespace firelight::library

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
  struct EntryCreatedEvent {
    int entryId;
  };

  struct EntryUpdatedEvent {
    int entryId;
  };


  class IUserLibraryRepository {
  public:
    virtual ~IUserLibraryRepository() = default;

    virtual bool create(ContentFile &contentFile) = 0;

    virtual void create(PatchFile &file) = 0;

    virtual bool create(FolderInfo &folder) = 0;

    virtual bool create(FolderEntryInfo &folderEntry) = 0;

    virtual bool create(ContentDirectory &directory) = 0;

    virtual bool create(DiscMember &member) = 0;


    virtual bool createEntry(Entry &entry) = 0;

    virtual void createRunConfiguration(int contentFileId, const std::string &path,
                                        int platformId,
                                        const std::string &contentHash) = 0;

    virtual bool update(FolderInfo &folder) = 0;

    virtual bool update(Entry &entry) = 0;

    virtual bool update(const ContentDirectory &directory) = 0;

    virtual bool updateEntryMetadata(const Entry &entry) = 0;

    virtual std::vector<FolderInfo> listFolders() = 0;

    virtual bool deleteFolder(int folderId) = 0;

    virtual bool deleteFolderEntry(FolderEntryInfo &info) = 0;

    virtual bool reorderFolders(int parentId,
                                const std::vector<int> &orderedFolderIds) = 0;

    // Moves a folder under a new parent (-1 = root), appended at the end
    virtual bool setFolderParent(int folderId, int newParentId) = 0;

    virtual bool deleteContentDirectory(int id) = 0;

    virtual std::optional<ContentFile>
    getContentFileWithPathAndSize(const std::string &filePath, size_t fileSizeBytes,
                                  bool inArchive) = 0;

    virtual std::vector<ContentFile> getContentFiles() = 0;

    virtual std::optional<ContentFile> getContentFile(int id) = 0;

    virtual bool deleteContentFile(int id) = 0;

    virtual std::optional<PatchFile> getPatchFile(int id) = 0;

    virtual std::vector<Entry> getEntries(int offset, int limit) = 0;

    virtual std::optional<Entry> getEntry(int entryId) = 0;

    virtual std::optional<Entry>
    getEntryWithContentHash(const std::string &contentHash) = 0;

    virtual std::vector<RunConfiguration>
    getRunConfigurations(const std::string &contentHash) = 0;

    virtual std::vector<ContentDirectory> getContentDirectories() = 0;
  };
} // namespace firelight::library

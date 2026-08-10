#pragma once

#include <firelight/library/content_directory.hpp>
#include <firelight/library/content_file.hpp>
#include <firelight/library/disc_member.hpp>
#include <firelight/library/entry.hpp>
#include <firelight/library/folder_entry_info.hpp>
#include <firelight/library/folder_info.hpp>
#include <firelight/library/patch_file.hpp>
#include <firelight/library/run_configuration.hpp>
#include <firelight/library/tag.hpp>
#include <firelight/library/variant_group.hpp>

#include <optional>
#include <set>
#include <vector>

namespace firelight::library {
struct EntryCreatedEvent {
  int entryId;
};

struct EntryUpdatedEvent {
  int entryId;
};

// TODO
// Published when a group's title, pin, or the entry standing for it changes, and when the
// group is dissolved. The id outlives the group, so a listener must tolerate it being gone
struct VariantGroupUpdatedEvent {
  int groupId;
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

  virtual void createRunConfiguration(int contentFileId, const std::string &path, int platformId,
                                      const std::string &contentHash) = 0;

  virtual bool update(FolderInfo &folder) = 0;

  virtual bool update(Entry &entry) = 0;

  virtual bool update(const ContentDirectory &directory) = 0;

  /**
   * Writes an entry's art projections. The descriptive document is not written here — see
   * applyEntryMetadata
   */
  virtual bool updateEntryMetadata(const Entry &entry) = 0;

  /**
   * Merges metadata into an entry's stored document.
   *
   * Fields the user pinned are skipped unless isUserEdit says this change is theirs, in which case
   * every field in changedFields is written and pinned. The read, merge and write happen in one
   * locked step, so a scrape on a worker cannot lose an edit made while it was running
   *
   * @param changedFields The metadata_fields names incoming carries a value for
   */
  virtual bool applyEntryMetadata(int entryId, const GameMetadata &incoming, const std::set<std::string> &changedFields,
                                  bool isUserEdit) = 0;

  /**
   * Records that art was looked up for an entry, whether or not any was found.
   *
   * Stamped even on an empty result, so a game the provider has nothing for is not
   * asked about again on every startup
   */
  virtual bool markArtFetched(int entryId, uint64_t whenMillis) = 0;

  /**
   * The ids of visible entries art has never been looked up for, oldest first.
   *
   * A query rather than a filter over getEntries, because the caller wants a handful at a time out of
   * a library of thousands
   */
  virtual std::vector<int> getEntryIdsMissingArt(int limit) = 0;

  //****************
  // variant groups
  //****************

  virtual bool createVariantGroup(VariantGroup &group) = 0;

  virtual bool updateVariantGroup(const VariantGroup &group) = 0;

  /**
   * Removes a group and clears it from every entry that was in it
   */
  virtual bool deleteVariantGroup(int groupId) = 0;

  virtual std::vector<VariantGroup> getVariantGroups() = 0;

  virtual std::optional<VariantGroup> getVariantGroup(int groupId) = 0;

  /**
   * Puts an entry in a group, or takes it out of whichever one it was in
   */
  virtual bool setEntryVariantGroup(int entryId, std::optional<int> groupId, bool isUserChoice) = 0;

  /**
   * The entries on a platform whose derived title folds to the same string, in id order
   */
  virtual std::vector<int> getEntryIdsWithNormalizedTitle(unsigned platformId, const std::string &normalizedTitle) = 0;

  virtual std::vector<Entry> getEntriesInVariantGroup(int groupId) = 0;

  //****************
  // tags
  //****************

  /**
   * Creates a tag, or fills in the existing one when the name is already taken. Names collide case
   * insensitively, so "sci-fi" and "Sci-Fi" are the same tag
   */
  virtual bool createTag(Tag &tag) = 0;

  virtual bool renameTag(int tagId, const std::string &name) = 0;

  /**
   * Moves every entry carrying sourceTagId onto targetTagId and removes the source. An entry that
   * carried both keeps one
   */
  virtual bool mergeTags(int sourceTagId, int targetTagId) = 0;

  /**
   * Removes a tag and takes it off every entry carrying it
   */
  virtual bool deleteTag(int tagId) = 0;

  /**
   * Every tag, with how many entries carry each
   */
  virtual std::vector<Tag> getTags() = 0;

  /**
   * Replaces an entry's tags with exactly the ones given
   */
  virtual bool setEntryTags(int entryId, const std::vector<int> &tagIds) = 0;

  virtual std::vector<FolderInfo> listFolders() = 0;

  virtual bool deleteFolder(int folderId) = 0;

  virtual bool deleteFolderEntry(FolderEntryInfo &info) = 0;

  virtual bool reorderFolders(int parentId, const std::vector<int> &orderedFolderIds) = 0;

  // Moves a folder under a new parent (-1 = root), appended at the end
  virtual bool setFolderParent(int folderId, int newParentId) = 0;

  virtual bool deleteContentDirectory(int id) = 0;

  virtual std::optional<ContentFile> getContentFileWithPathAndSize(const std::string &filePath, size_t fileSizeBytes,
                                                                   bool inArchive) = 0;

  virtual std::vector<ContentFile> getContentFiles() = 0;

  virtual std::optional<ContentFile> getContentFile(int id) = 0;

  virtual bool deleteContentFile(int id) = 0;

  virtual std::optional<PatchFile> getPatchFile(int id) = 0;

  virtual std::vector<Entry> getEntries(int offset, int limit) = 0;

  virtual std::optional<Entry> getEntry(int entryId) = 0;

  virtual std::optional<Entry> getEntryWithContentHash(const std::string &contentHash) = 0;

  virtual std::vector<RunConfiguration> getRunConfigurations(const std::string &contentHash) = 0;

  virtual std::vector<ContentDirectory> getContentDirectories() = 0;
};
} // namespace firelight::library

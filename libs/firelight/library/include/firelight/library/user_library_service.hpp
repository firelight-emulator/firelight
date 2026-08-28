// TODO: NEEDS REVIEW
#pragma once

#include <firelight/library/content_directory.hpp>
#include <firelight/library/content_file.hpp>
#include <firelight/library/disc_set.hpp>
#include <firelight/library/entry.hpp>
#include <firelight/library/folder_entry.hpp>
#include <firelight/library/folder_info.hpp>
#include <firelight/library/game_identity.hpp>
#include <firelight/library/tag.hpp>
#include <firelight/library/variant_group.hpp>
#include <firelight/util/game_metadata.hpp>

#include <cstdint>
#include <optional>
#include <set>
#include <string>
#include <vector>

namespace firelight::library {
class IUserLibraryRepository;

/**
 * Provides the library service's operations on a user library, delegating to a repository for the actual storage
 */
class UserLibraryService {
public:
  UserLibraryService(IUserLibraryRepository &repository, const std::string &defaultContentDirectory);

  // Entries
  std::vector<Entry> getEntries();
  std::optional<Entry> getEntry(int entryId);
  std::optional<Entry> getEntryWithContentHash(const std::string &contentHash);
  bool update(Entry &entry);

  // Writes an entry's name and artwork projections. Separate from update() so a
  // user's own edits and the metadata pipeline share one write path
  bool updateEntryMetadata(const Entry &entry);

  // Merges metadata into an entry's document, leaving fields the user pinned alone
  // unless the change is theirs
  bool applyEntryMetadata(int entryId, const GameMetadata &incoming, const std::set<std::string> &changedFields,
                          bool isUserEdit);

  // Records that art was looked up, found or not
  bool markArtFetched(int entryId, uint64_t whenMillis);

  // A handful of entries art has never been looked up for
  std::vector<int> getEntryIdsMissingArt(int limit);

  // Variant groups
  bool createVariantGroup(VariantGroup &group);
  bool updateVariantGroup(const VariantGroup &group);
  bool deleteVariantGroup(int groupId);
  std::vector<VariantGroup> getVariantGroups();
  std::optional<VariantGroup> getVariantGroup(int groupId);
  bool setEntryVariantGroup(int entryId, std::optional<int> groupId, bool isUserChoice);
  std::vector<int> getCandidateEntryIds(const GameIdentity &identity);
  std::vector<Entry> getEntriesInVariantGroup(int groupId);

  // Disc sets
  std::optional<DiscSet> getDiscSet(int setId);
  std::vector<ContentFile> getDiscsInSet(int setId);

  // Only the discs of the set whose bytes are on disk
  std::vector<ContentFile> getPresentDiscsInSet(int setId);

  // Pins a group's primary, so the preference ordering stops moving it
  bool setVariantGroupPrimary(int groupId, int entryId);

  // Hands the primary back to the preference ordering
  bool clearVariantGroupPrimary(int groupId);

  // Tags
  bool createTag(Tag &tag);
  bool renameTag(int tagId, const std::string &name);
  bool mergeTags(int sourceTagId, int targetTagId);
  bool deleteTag(int tagId);
  std::vector<Tag> getTags();
  bool setEntryTags(int entryId, const std::vector<int> &tagIds);

  // Folders
  std::vector<FolderInfo> listFolders();
  bool create(FolderInfo &folder);
  bool update(FolderInfo &folder);
  bool deleteFolder(int folderId);
  bool reorderFolders(int parentId, const std::vector<int> &orderedFolderIds);
  bool setFolderParent(int folderId, int newParentId);
  bool create(FolderEntry &folderEntry);
  bool deleteFolderEntry(FolderEntry &info);

  // Watched directories
  std::vector<ContentDirectory> getContentDirectories();
  bool create(ContentDirectory &directory);
  bool update(const ContentDirectory &directory);
  bool deleteContentDirectory(int id);

private:
  IUserLibraryRepository &m_repository;
};

} // namespace firelight::library

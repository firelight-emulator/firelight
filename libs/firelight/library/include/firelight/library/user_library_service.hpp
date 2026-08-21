#pragma once

#include <firelight/library/user_library_repository.hpp>

#include <string>

namespace firelight::library {

// The app-facing curation surface for the user library. A thin, concrete facade
// over the repository (no Qt, no QObject): it exposes only the operations the GUI
// and emulation need — browsing/updating entries, managing folders, and managing
// content directories — and keeps the full CRUD contract out of the app
//
// It also owns the guarantee that a default content directory always exists: on
// construction it creates that directory on disk and ensures it is watched, so
// the user never has to pick or configure a primary games folder
class UserLibraryService {
public:
  UserLibraryService(IUserLibraryRepository &repository, const std::string &defaultContentDirectory);

  // Entries
  std::vector<Entry> getEntries(int offset = 0, int limit = -1);
  std::optional<Entry> getEntry(int entryId);
  std::optional<Entry> getEntryWithContentHash(const std::string &contentHash);
  bool update(Entry &entry);

  // Writes an entry's name and artwork projections. Separate from update() so a
  // user's own edits and the metadata pipeline share one write path
  bool updateEntryMetadata(const Entry &entry);

  // TODO
  // Merges metadata into an entry's document, leaving fields the user pinned alone
  // unless the change is theirs
  bool applyEntryMetadata(int entryId, const GameMetadata &incoming, const std::set<std::string> &changedFields,
                          bool isUserEdit);

  // TODO
  // Records that art was looked up, found or not
  bool markArtFetched(int entryId, uint64_t whenMillis);

  // TODO
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

  // TODO
  // Only the discs of the set whose bytes are on disk
  std::vector<ContentFile> getPresentDiscsInSet(int setId);

  // TODO
  // Pins a group's primary, so the preference ordering stops moving it
  bool setVariantGroupPrimary(int groupId, int entryId);

  // TODO
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
  bool create(FolderEntryInfo &folderEntry);
  bool deleteFolderEntry(FolderEntryInfo &info);

  // Watched directories
  std::vector<ContentDirectory> getContentDirectories();
  bool create(ContentDirectory &directory);
  bool update(const ContentDirectory &directory);
  bool deleteContentDirectory(int id);

private:
  IUserLibraryRepository &m_repository;
};

} // namespace firelight::library

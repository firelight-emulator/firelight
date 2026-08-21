#include <firelight/library/user_library_service.hpp>

#include <filesystem>
#include <spdlog/spdlog.h>

namespace firelight::library {

UserLibraryService::UserLibraryService(IUserLibraryRepository &repository, const std::string &defaultContentDirectory)
    : m_repository(repository) {
  // Guarantee the default content directory: the user never picks or changes the
  // primary games folder, so make sure it exists on disk and is watched. Both
  // steps are idempotent, so this is safe on every startup (and re-seeds a reset
  // database or a library the user emptied of all directories)
  std::error_code ec;
  std::filesystem::create_directories(defaultContentDirectory, ec);
  if (ec) {
    spdlog::warn("Failed to create default content directory {}: {}", defaultContentDirectory, ec.message());
  }

  for (const auto &dir : m_repository.getContentDirectories()) {
    if (dir.path == defaultContentDirectory) {
      return;
    }
  }

  ContentDirectory defaultDir{.path = defaultContentDirectory};
  m_repository.create(defaultDir);
}

std::vector<Entry> UserLibraryService::getEntries(int offset, int limit) {
  return m_repository.getEntries(offset, limit);
}

std::optional<Entry> UserLibraryService::getEntry(int entryId) { return m_repository.getEntry(entryId); }

std::optional<Entry> UserLibraryService::getEntryWithContentHash(const std::string &contentHash) {
  return m_repository.getEntryWithContentHash(contentHash);
}

bool UserLibraryService::update(Entry &entry) { return m_repository.update(entry); }

bool UserLibraryService::updateEntryMetadata(const Entry &entry) { return m_repository.updateEntryMetadata(entry); }

bool UserLibraryService::applyEntryMetadata(const int entryId, const GameMetadata &incoming,
                                            const std::set<std::string> &changedFields, const bool isUserEdit) {
  return m_repository.applyEntryMetadata(entryId, incoming, changedFields, isUserEdit);
}

bool UserLibraryService::markArtFetched(const int entryId, const uint64_t whenMillis) {
  return m_repository.markArtFetched(entryId, whenMillis);
}

std::vector<int> UserLibraryService::getEntryIdsMissingArt(const int limit) {
  return m_repository.getEntryIdsMissingArt(limit);
}

bool UserLibraryService::createTag(Tag &tag) { return m_repository.createTag(tag); }

bool UserLibraryService::renameTag(const int tagId, const std::string &name) {
  return m_repository.renameTag(tagId, name);
}

bool UserLibraryService::mergeTags(const int sourceTagId, const int targetTagId) {
  return m_repository.mergeTags(sourceTagId, targetTagId);
}

bool UserLibraryService::deleteTag(const int tagId) { return m_repository.deleteTag(tagId); }

std::vector<Tag> UserLibraryService::getTags() { return m_repository.getTags(); }

bool UserLibraryService::setEntryTags(const int entryId, const std::vector<int> &tagIds) {
  return m_repository.setEntryTags(entryId, tagIds);
}

bool UserLibraryService::createVariantGroup(VariantGroup &group) { return m_repository.createVariantGroup(group); }

bool UserLibraryService::updateVariantGroup(const VariantGroup &group) {
  return m_repository.updateVariantGroup(group);
}

bool UserLibraryService::deleteVariantGroup(const int groupId) { return m_repository.deleteVariantGroup(groupId); }

std::vector<VariantGroup> UserLibraryService::getVariantGroups() { return m_repository.getVariantGroups(); }

std::optional<VariantGroup> UserLibraryService::getVariantGroup(const int groupId) {
  return m_repository.getVariantGroup(groupId);
}

bool UserLibraryService::setEntryVariantGroup(const int entryId, const std::optional<int> groupId,
                                              const bool isUserChoice) {
  return m_repository.setEntryVariantGroup(entryId, groupId, isUserChoice);
}

std::vector<int> UserLibraryService::getCandidateEntryIds(const GameIdentity &identity) {
  return m_repository.getCandidateEntryIds(identity);
}

std::vector<Entry> UserLibraryService::getEntriesInVariantGroup(const int groupId) {
  return m_repository.getEntriesInVariantGroup(groupId);
}

std::optional<DiscSet> UserLibraryService::getDiscSet(const int setId) { return m_repository.getDiscSet(setId); }

std::vector<ContentFile> UserLibraryService::getPresentDiscsInSet(const int setId) {
  return m_repository.getPresentDiscsInSet(setId);
}

std::vector<ContentFile> UserLibraryService::getDiscsInSet(const int setId) {
  return m_repository.getDiscsInSet(setId);
}

bool UserLibraryService::setVariantGroupPrimary(const int groupId, const int entryId) {
  auto group = m_repository.getVariantGroup(groupId);

  if (!group.has_value()) {
    return false;
  }

  group->primaryEntryId = entryId;
  group->primaryUserSet = true;

  return m_repository.updateVariantGroup(*group);
}

bool UserLibraryService::clearVariantGroupPrimary(const int groupId) {
  auto group = m_repository.getVariantGroup(groupId);

  if (!group.has_value()) {
    return false;
  }

  group->primaryUserSet = false;

  return m_repository.updateVariantGroup(*group);
}

std::vector<FolderInfo> UserLibraryService::listFolders() { return m_repository.listFolders(); }

bool UserLibraryService::create(FolderInfo &folder) { return m_repository.create(folder); }

bool UserLibraryService::update(FolderInfo &folder) { return m_repository.update(folder); }

bool UserLibraryService::deleteFolder(int folderId) { return m_repository.deleteFolder(folderId); }

bool UserLibraryService::reorderFolders(int parentId, const std::vector<int> &orderedFolderIds) {
  return m_repository.reorderFolders(parentId, orderedFolderIds);
}

bool UserLibraryService::setFolderParent(int folderId, int newParentId) {
  return m_repository.setFolderParent(folderId, newParentId);
}

bool UserLibraryService::create(FolderEntryInfo &folderEntry) { return m_repository.create(folderEntry); }

bool UserLibraryService::deleteFolderEntry(FolderEntryInfo &info) { return m_repository.deleteFolderEntry(info); }

std::vector<ContentDirectory> UserLibraryService::getContentDirectories() {
  return m_repository.getContentDirectories();
}

bool UserLibraryService::create(ContentDirectory &directory) { return m_repository.create(directory); }

bool UserLibraryService::update(const ContentDirectory &directory) { return m_repository.update(directory); }

bool UserLibraryService::deleteContentDirectory(int id) { return m_repository.deleteContentDirectory(id); }

} // namespace firelight::library

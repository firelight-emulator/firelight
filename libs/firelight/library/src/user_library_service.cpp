#include <firelight/library/user_library_service.hpp>

#include <filesystem>
#include <spdlog/spdlog.h>

namespace firelight::library {

UserLibraryService::UserLibraryService(IUserLibraryRepository &repository,
                                       const QString &defaultContentDirectory)
    : m_repository(repository) {
  // Guarantee the default content directory: the user never picks or changes the
  // primary games folder, so make sure it exists on disk and is watched. Both
  // steps are idempotent, so this is safe on every startup (and re-seeds a reset
  // database or a library the user emptied of all directories).
  const auto path = defaultContentDirectory.toStdString();
  std::error_code ec;
  std::filesystem::create_directories(path, ec);
  if (ec) {
    spdlog::warn("Failed to create default content directory {}: {}", path,
                 ec.message());
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

std::optional<Entry> UserLibraryService::getEntry(int entryId) {
  return m_repository.getEntry(entryId);
}

std::optional<Entry>
UserLibraryService::getEntryWithContentHash(const std::string &contentHash) {
  return m_repository.getEntryWithContentHash(contentHash);
}

bool UserLibraryService::update(Entry &entry) {
  return m_repository.update(entry);
}

std::vector<FolderInfo> UserLibraryService::listFolders() {
  return m_repository.listFolders();
}

bool UserLibraryService::create(FolderInfo &folder) {
  return m_repository.create(folder);
}

bool UserLibraryService::update(FolderInfo &folder) {
  return m_repository.update(folder);
}

bool UserLibraryService::deleteFolder(int folderId) {
  return m_repository.deleteFolder(folderId);
}

bool UserLibraryService::reorderFolders(
    int parentId, const std::vector<int> &orderedFolderIds) {
  return m_repository.reorderFolders(parentId, orderedFolderIds);
}

bool UserLibraryService::setFolderParent(int folderId, int newParentId) {
  return m_repository.setFolderParent(folderId, newParentId);
}

bool UserLibraryService::create(FolderEntryInfo &folderEntry) {
  return m_repository.create(folderEntry);
}

bool UserLibraryService::deleteFolderEntry(FolderEntryInfo &info) {
  return m_repository.deleteFolderEntry(info);
}

std::vector<ContentDirectory> UserLibraryService::getContentDirectories() {
  return m_repository.getContentDirectories();
}

bool UserLibraryService::create(ContentDirectory &directory) {
  return m_repository.create(directory);
}

bool UserLibraryService::update(const ContentDirectory &directory) {
  return m_repository.update(directory);
}

bool UserLibraryService::deleteContentDirectory(int id) {
  return m_repository.deleteContentDirectory(id);
}

} // namespace firelight::library

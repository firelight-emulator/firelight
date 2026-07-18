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
  UserLibraryService(IUserLibraryRepository &repository,
                     const std::string &defaultContentDirectory);

  // Entries
  std::vector<Entry> getEntries(int offset = 0, int limit = -1);
  std::optional<Entry> getEntry(int entryId);
  std::optional<Entry> getEntryWithContentHash(const std::string &contentHash);
  bool update(Entry &entry);

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

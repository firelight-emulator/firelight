#pragma once

#include "entry.hpp"
#include "folder_entry_info.hpp"
#include "folder_info.hpp"
#include "rom_file.hpp"
#include "run_configuration.hpp"
#include "watched_directory.hpp"

namespace firelight::library {
class IUserLibrary {
public:
  virtual ~IUserLibrary() = default;

  virtual void create(RomFile &path) = 0;
  virtual void create(PatchFile &file) = 0;
  virtual bool create(FolderInfo &folder) = 0;
  virtual bool create(FolderEntryInfo &folderEntry) = 0;

  virtual bool update(FolderInfo &folder) = 0;
  virtual std::vector<FolderInfo> listFolders(FolderInfo filter) = 0;
  virtual bool deleteFolder(int folderId) = 0;

  virtual bool deleteFolderEntry(FolderEntryInfo &info) = 0;

  virtual bool update(Entry &entry) = 0;

  virtual std::optional<RomFile>
  getRomFileWithPathAndSize(const QString &filePath, size_t fileSizeBytes,
                            bool inArchive) = 0;

  virtual std::vector<RomFile> getRomFiles() = 0;

  virtual std::optional<RomFile> getRomFile(int id) = 0;

  virtual std::optional<PatchFile>
  getPatchFileWithPathAndSize(const QString &filePath, size_t fileSizeBytes,
                              bool inArchive) = 0;

  virtual std::optional<PatchFile> getPatchFile(int id) = 0;

  virtual std::vector<PatchFile> getPatchFiles() = 0;

  virtual std::vector<Entry> getEntries(int offset, int limit) = 0;

  virtual std::optional<Entry> getEntry(int entryId) = 0;

  virtual std::vector<RunConfiguration>
  getRunConfigurations(const QString &contentHash) = 0;

  virtual void doStuffWithRunConfigurations() = 0;

  virtual std::vector<RomFile>
  getRomFilesWithContentHash(const QString &contentHash) = 0;

  virtual std::vector<WatchedDirectory> getWatchedDirectories() = 0;

  virtual bool addWatchedDirectory(const WatchedDirectory &directory) = 0;

  virtual bool updateWatchedDirectory(const WatchedDirectory &directory) = 0;
};
} // namespace firelight::library

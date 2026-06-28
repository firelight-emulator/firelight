#pragma once

#include <firelight/library/entry.hpp>
#include <firelight/library/folder_entry_info.hpp>
#include <firelight/library/folder_info.hpp>
#include <firelight/library/patch_file.hpp>
#include <firelight/library/rom_file_info.hpp>
#include <firelight/library/run_configuration.hpp>
#include <firelight/library/watched_directory.hpp>

#include <optional>
#include <vector>

namespace firelight::library {

struct EntryUpdatedEvent {
  int entryId;
};

class IUserLibrary {
public:
  virtual ~IUserLibrary() = default;

  virtual bool create(RomFileInfo &romFileInfo) = 0;
  virtual void create(PatchFile &file) = 0;
  virtual bool create(FolderInfo &folder) = 0;
  virtual bool create(FolderEntryInfo &folderEntry) = 0;
  virtual bool create(WatchedDirectory &directory) = 0;

  virtual bool update(FolderInfo &folder) = 0;
  virtual std::vector<FolderInfo> listFolders(FolderInfo filter) = 0;
  virtual bool deleteFolder(int folderId) = 0;

  virtual bool deleteFolderEntry(FolderEntryInfo &info) = 0;

  virtual bool update(Entry &entry) = 0;
  virtual bool update(const WatchedDirectory &directory) = 0;

  virtual bool deleteContentDirectory(int id) = 0;

  virtual std::optional<RomFileInfo>
  getRomFileWithPathAndSize(const QString &filePath, size_t fileSizeBytes,
                            bool inArchive) = 0;

  virtual std::vector<RomFileInfo> getRomFiles() = 0;

  virtual std::optional<RomFileInfo> getRomFile(int id) = 0;
  virtual bool deleteRomFile(int id) = 0;

  virtual std::optional<PatchFile> getPatchFile(int id) = 0;

  virtual std::vector<Entry> getEntries(int offset, int limit) = 0;

  virtual std::optional<Entry> getEntry(int entryId) = 0;

  virtual std::optional<Entry>
  getEntryWithContentHash(const QString &contentHash) = 0;

  virtual std::vector<RunConfiguration>
  getRunConfigurations(const QString &contentHash) = 0;

  virtual void doStuffWithRunConfigurations() = 0;

  virtual std::vector<RomFileInfo>
  getRomFilesWithContentHash(const QString &contentHash) = 0;

  virtual std::vector<WatchedDirectory> getWatchedDirectories() = 0;
};
} // namespace firelight::library

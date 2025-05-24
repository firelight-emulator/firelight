#pragma once

#include "entry.hpp"
#include "entry_folder_info.hpp"
#include "rom_file.hpp"
#include "run_configuration.hpp"
#include "watched_directory.hpp"

namespace firelight::library {
class IUserLibrary {
public:
  virtual ~IUserLibrary() = default;

  virtual bool create(EntryFolderInfo &folder) = 0;
  virtual std::vector<EntryFolderInfo> getFolders(EntryFolderInfo filter) = 0;

  virtual void addRomFile(RomFile &path) = 0;

  virtual std::optional<RomFile>
  getRomFileWithPathAndSize(const QString &filePath, size_t fileSizeBytes,
                            bool inArchive) = 0;

  virtual std::vector<RomFile> getRomFiles() = 0;

  virtual std::optional<RomFile> getRomFile(int id) = 0;

  virtual void addPatchFile(PatchFile &file) = 0;

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

#pragma once

#include "daos/lib_entry.hpp"
#include "daos/library_entry.hpp"
#include "daos/playlist.hpp"

#include <vector>

namespace Firelight::Databases {

class ILibraryDatabase {
public:
  struct Filter {
    int id = -1;
    std::string display_name;
    bool verified;
    std::string md5;
    int platform;
    int game = -1;
    int rom = -1;
    int parent_entry = -1;
    int romhack = -1;
    std::string source_directory;
    std::string content_path;
  };

  virtual ~ILibraryDatabase() = default;
  virtual void addOrRenameEntry(LibEntry entry) = 0;
  virtual std::vector<LibEntry> getAllEntries() = 0;
  virtual void match_md5s(std::string t_sourceDirectory,
                          std::vector<std::string> t_md5List) = 0;
  virtual void updateEntryContentPath(int entryId, std::string sourceDirectory,
                                      std::string contentPath) = 0;
  virtual std::vector<LibEntry> getMatching(Filter filter) = 0;

  virtual bool createPlaylist(Firelight::Databases::Playlist &playlist) = 0;
  virtual bool addEntryToPlaylist(int playlistId, int entryId) = 0;
  virtual bool tableExists(const std::string &tableName) = 0;
  [[nodiscard]] virtual std::vector<Firelight::Databases::Playlist>
  getAllPlaylists() const = 0;
};
} // namespace Firelight::Databases

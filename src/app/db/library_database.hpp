#pragma once

#include "daos/lib_entry.hpp"
#include "daos/library_entry.hpp"
#include "daos/playlist.hpp"

#include <optional>
#include <vector>

namespace firelight::db {

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

  /**
   * @param tableName The name of the table to check for.
   * @return true if the table exists, false otherwise.
   */
  virtual bool tableExists(const std::string &tableName) = 0;

  /**
   * @brief Creates a new Library Entry.
   *
   * @param entry The entry object to use when creating the entry.
   *
   * @note The entry object will be updated with the new entry's ID.
   */
  virtual bool createLibraryEntry(LibraryEntry &entry) = 0;
  virtual std::optional<LibraryEntry> getLibraryEntry(int entryId) = 0;
  virtual bool deleteLibraryEntry(int entryId) = 0;
  virtual std::vector<LibraryEntry> getAllLibraryEntries() = 0;

  /**
   * @brief Creates a new Playlist.
   *
   * @param playlist The playlist object to use when creating the playlist.
   *
   * @note The playlist object will be updated with the new playlist's ID.
   */
  virtual bool createPlaylist(Playlist &playlist) = 0;
  virtual bool deletePlaylist(int playlistId) = 0;
  virtual bool renamePlaylist(int playlistId, std::string newName) = 0;
  virtual bool addEntryToPlaylist(int playlistId, int entryId) = 0;
  [[nodiscard]] virtual std::vector<Playlist> getAllPlaylists() = 0;
};
} // namespace firelight::db

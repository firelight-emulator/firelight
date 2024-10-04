#pragma once

#include "library_entry.hpp"
#include "playlist.hpp"
#include "library/entry.hpp"

#include <optional>
#include <string>
#include <vector>

#include "library_content_directory.hpp"

namespace firelight::db {
 class ILibraryDatabase {
 public:
  virtual ~ILibraryDatabase() = default;

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

  virtual std::vector<LibraryEntry>
  getMatchingLibraryEntries(const LibraryEntry &entry) = 0;

  virtual std::vector<std::string> getAllContentPaths() = 0;

  virtual bool
  createLibraryContentDirectory(LibraryContentDirectory &directory) = 0;

  virtual bool
  updateLibraryContentDirectory(LibraryContentDirectory &directory) = 0;

  virtual std::vector<LibraryContentDirectory>
  getAllLibraryContentDirectories() const = 0;

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

  virtual std::vector<Playlist> getPlaylistsForEntry(int entryId) = 0;
 };
} // namespace firelight::db

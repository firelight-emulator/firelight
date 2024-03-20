#pragma once

#include "library_database.hpp"
#include <QSqlDatabase>
#include <QThreadStorage>
#include <filesystem>

namespace firelight::db {

class SqliteLibraryDatabase final : public ILibraryDatabase {
public:
  explicit SqliteLibraryDatabase(const std::filesystem::path &db_file_path);
  ~SqliteLibraryDatabase() override;

  // General stuff
  bool tableExists(const std::string &tableName) override;

  // Library Entries
  bool createLibraryEntry(LibraryEntry &entry) override;
  std::optional<LibraryEntry> getLibraryEntry(int entryId) override;
  bool deleteLibraryEntry(int entryId) override;
  std::vector<LibraryEntry> getAllLibraryEntries() override;
  std::vector<LibraryEntry>
  getMatchingLibraryEntries(const LibraryEntry &entry) override;

  // Playlists
  bool createPlaylist(Playlist &playlist) override;
  bool deletePlaylist(int playlistId) override;
  bool renamePlaylist(int playlistId, std::string newName) override;
  [[nodiscard]] std::vector<Playlist> getAllPlaylists() override;
  std::vector<Playlist> getPlaylistsForEntry(int entryId) override;

  // Playlist Entries
  bool addEntryToPlaylist(int playlistId, int entryId) override;

private:
  std::filesystem::path m_dbFilePath;
  static LibraryEntry createLibraryEntryFromQuery(const QSqlQuery &query);
  QSqlDatabase getDatabase() const;
};

} // namespace firelight::db

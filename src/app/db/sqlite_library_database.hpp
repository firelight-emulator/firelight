#pragma once

#include "library_database.hpp"
#include <QSqlDatabase>
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

  // Playlists
  bool createPlaylist(Playlist &playlist) override;
  bool deletePlaylist(int playlistId) override;
  bool renamePlaylist(int playlistId, std::string newName) override;
  [[nodiscard]] std::vector<Playlist> getAllPlaylists() override;

  // Playlist Entries
  bool addEntryToPlaylist(int playlistId, int entryId) override;

  // Old stuff
  void updateEntryContentPath(int entryId, std::string sourceDirectory,
                              std::string contentPath) override;

  void addOrRenameEntry(LibEntry entry) override;
  std::vector<LibEntry> getAllEntries() override;
  void match_md5s(std::string source_directory,
                  std::vector<std::string> md5s) override;
  std::vector<LibEntry> getMatching(Filter filter) override;

private:
  void insert_entry_into_db(LibEntry entry) const;
  QSqlDatabase m_database;
};

} // namespace firelight::db

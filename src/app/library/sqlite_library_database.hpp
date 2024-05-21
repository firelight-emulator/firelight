#pragma once

#include "firelight/library_database.hpp"

#include <QObject>
#include <QSqlDatabase>
#include <QThreadStorage>
#include <filesystem>

namespace firelight::db {

class SqliteLibraryDatabase final : public QObject, public ILibraryDatabase {
  Q_OBJECT

public:
  explicit SqliteLibraryDatabase(std::filesystem::path db_file_path);
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
  std::vector<std::string> getAllContentPaths() override;

  // Playlists
  bool createPlaylist(Playlist &playlist) override;
  bool deletePlaylist(int playlistId) override;
  bool renamePlaylist(int playlistId, std::string newName) override;
  [[nodiscard]] std::vector<Playlist> getAllPlaylists() override;
  std::vector<Playlist> getPlaylistsForEntry(int entryId) override;

  // Playlist Entries
  bool addEntryToPlaylist(int playlistId, int entryId) override;

signals:
  void libraryEntryCreated(const firelight::db::LibraryEntry &entry);

private:
  std::filesystem::path m_dbFilePath;
  static LibraryEntry createLibraryEntryFromQuery(const QSqlQuery &query);
  [[nodiscard]] QSqlDatabase getDatabase() const;
};

} // namespace firelight::db

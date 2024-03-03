//
// Created by alexs on 12/29/2023.
//

#pragma once

#include "library_database.hpp"

#include <QSqlDatabase>
#include <filesystem>

namespace Firelight::Databases {

class SqliteLibraryDatabase final : public ILibraryDatabase {
public:
  bool createPlaylist(Playlist &playlist) override;
  bool addEntryToPlaylist(int playlistId, int entryId) override;
  void updateEntryContentPath(int entryId, std::string sourceDirectory,
                              std::string contentPath) override;
  explicit SqliteLibraryDatabase(const std::filesystem::path &db_file_path);
  ~SqliteLibraryDatabase() override;

  void addOrRenameEntry(LibEntry entry) override;
  std::vector<LibEntry> getAllEntries() override;
  void match_md5s(std::string source_directory,
                  std::vector<std::string> md5s) override;
  std::vector<LibEntry> getMatching(Filter filter) override;
  bool tableExists(const std::string &tableName) override;
  [[nodiscard]] std::vector<Playlist> getAllPlaylists() const override;

private:
  void insert_entry_into_db(LibEntry entry) const;
  QSqlDatabase m_database;
};
} // namespace Firelight::Databases

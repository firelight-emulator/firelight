#pragma once

#include "../../src/app/db/library_database.hpp"
#include <gmock/gmock.h>

namespace firelight::db {
class MockLibraryDatabase final : public ILibraryDatabase {
public:
  MOCK_METHOD(bool, tableExists, (const std::string &tableName), (override));
  MOCK_METHOD(bool, createLibraryEntry, (LibraryEntry & entry), (override));
  MOCK_METHOD(bool, deleteLibraryEntry, (int entryId), (override));
  MOCK_METHOD(std::vector<LibraryEntry>, getAllLibraryEntries, (), (override));
  MOCK_METHOD(bool, createPlaylist, (Playlist & playlist), (override));
  MOCK_METHOD(bool, deletePlaylist, (int playlistId), (override));
  MOCK_METHOD(std::vector<Playlist>, getAllPlaylists, (), (override));
  MOCK_METHOD(bool, addEntryToPlaylist, (int playlistId, int entryId),
              (override));
  MOCK_METHOD(void, addOrRenameEntry, (LibEntry entry), (override));
  MOCK_METHOD(std::vector<LibEntry>, getAllEntries, (), (override));
  MOCK_METHOD(void, match_md5s,
              (std::string t_sourceDirectory,
               std::vector<std::string> t_md5List),
              (override));
  MOCK_METHOD(void, updateEntryContentPath,
              (int entryId, std::string sourceDirectory,
               std::string contentPath),
              (override));
  MOCK_METHOD(std::vector<LibEntry>, getMatching, (Filter filter), (override));
};
} // namespace firelight::db

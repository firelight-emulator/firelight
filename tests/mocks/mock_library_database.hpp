#pragma once

#include "firelight/library_database.hpp"
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
};
} // namespace firelight::db

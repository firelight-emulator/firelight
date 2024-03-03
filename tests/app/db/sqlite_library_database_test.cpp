#include "../../../src/app/db/sqlite_library_database.hpp"
#include "filesystem"
#include "gtest/gtest.h"

namespace firelight::db {

class SqliteLibraryDatabaseTest : public testing::Test {
protected:
  std::filesystem::path temp_file_path;

  void SetUp() override {
    temp_file_path = std::filesystem::temp_directory_path() / "temp.db";
  }

  void TearDown() override { std::filesystem::remove(temp_file_path); }
};

TEST_F(SqliteLibraryDatabaseTest, ConstructorTest) {
  SqliteLibraryDatabase db(temp_file_path.string());

  ASSERT_TRUE(db.tableExists("library_entries"));
  ASSERT_TRUE(db.tableExists("playlists"));
  ASSERT_TRUE(db.tableExists("playlist_entries"));
}

TEST_F(SqliteLibraryDatabaseTest, CreatePlaylistSetsId) {
  SqliteLibraryDatabase db(temp_file_path.string());
  Playlist playlist{.id = -1, .displayName = "Test Playlist"};

  ASSERT_TRUE(db.createPlaylist(playlist));
  ASSERT_NE(playlist.id, -1);

  const auto playlists = db.getAllPlaylists();
  ASSERT_EQ(playlists.size(), 1);
  ASSERT_EQ(playlists[0].id, playlist.id);
  ASSERT_EQ(playlists[0].displayName, playlist.displayName);
}
} // namespace firelight::db
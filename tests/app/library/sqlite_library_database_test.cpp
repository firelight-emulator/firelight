#include "../../../src/app/library/sqlite_library_database.hpp"
#include "filesystem"
#include "gtest/gtest.h"

namespace firelight::db {

class SqliteLibraryDatabaseTest : public testing::Test {
protected:
  std::filesystem::path temp_file_path;

  void SetUp() override {
    temp_file_path =
        std::filesystem::temp_directory_path() / "firelightlibrary.db";
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

TEST_F(SqliteLibraryDatabaseTest, CreatePlaylistFailsOnDuplicate) {
  SqliteLibraryDatabase db(temp_file_path.string());
  Playlist playlist{.id = -1, .displayName = "Test Playlist"};

  ASSERT_TRUE(db.createPlaylist(playlist));
  ASSERT_EQ(db.getAllPlaylists().size(), 1);
  ASSERT_FALSE(db.createPlaylist(playlist));
  ASSERT_EQ(db.getAllPlaylists().size(), 1);
}

TEST_F(SqliteLibraryDatabaseTest, DeletePlaylist) {
  SqliteLibraryDatabase db(temp_file_path.string());
  Playlist playlist{.id = -1, .displayName = "Test Playlist"};

  ASSERT_TRUE(db.createPlaylist(playlist));
  ASSERT_EQ(db.getAllPlaylists().size(), 1);
  ASSERT_TRUE(db.deletePlaylist(playlist.id));
  ASSERT_TRUE(db.getAllPlaylists().empty());
}

TEST_F(SqliteLibraryDatabaseTest, RenamePlaylist) {
  SqliteLibraryDatabase db(temp_file_path.string());
  Playlist playlist{.id = -1, .displayName = "Test Playlist"};

  ASSERT_TRUE(db.createPlaylist(playlist));
  ASSERT_EQ(db.getAllPlaylists().size(), 1);

  ASSERT_TRUE(db.renamePlaylist(playlist.id, "New Name"));

  const auto playlists = db.getAllPlaylists();
  ASSERT_EQ(playlists.size(), 1);
  ASSERT_EQ(playlists[0].id, playlist.id);
  ASSERT_EQ(playlists[0].displayName, "New Name");
}

TEST_F(SqliteLibraryDatabaseTest, RenamePlaylistWhenDoesntExist) {
  SqliteLibraryDatabase db(temp_file_path.string());

  ASSERT_TRUE(db.getAllPlaylists().empty());
  ASSERT_FALSE(db.renamePlaylist(1, "New Name"));
}

TEST_F(SqliteLibraryDatabaseTest, CreateLibraryEntrySetsId) {
  SqliteLibraryDatabase db(temp_file_path.string());
  LibraryEntry entry{.id = -1,
                     .displayName = "Test Playlist",
                     .contentMd5 = "1234567890",
                     .platformId = 1,
                     .type = LibraryEntry::EntryType::ROM,
                     .sourceDirectory = "source",
                     .contentPath = "content"};

  ASSERT_TRUE(db.createLibraryEntry(entry));
  ASSERT_NE(entry.id, -1);
  ASSERT_EQ(db.getAllLibraryEntries().size(), 1);
}

TEST_F(SqliteLibraryDatabaseTest, CreateLibraryEntryFailsOnDuplicate) {
  SqliteLibraryDatabase db(temp_file_path.string());
  LibraryEntry entry{.id = -1,
                     .displayName = "Test Playlist",
                     .contentMd5 = "1234567890",
                     .platformId = 1,
                     .type = LibraryEntry::EntryType::ROM,
                     .sourceDirectory = "source",
                     .contentPath = "content"};

  ASSERT_TRUE(db.createLibraryEntry(entry));
  ASSERT_EQ(db.getAllLibraryEntries().size(), 1);
  ASSERT_FALSE(db.createLibraryEntry(entry));
  ASSERT_EQ(db.getAllLibraryEntries().size(), 1);
}

TEST_F(SqliteLibraryDatabaseTest, GetLibraryEntry) {
  SqliteLibraryDatabase db(temp_file_path.string());
  LibraryEntry entry{.id = -1,
                     .displayName = "Test Playlist",
                     .contentMd5 = "1234567890",
                     .platformId = 1,
                     .type = LibraryEntry::EntryType::ROM,
                     .sourceDirectory = "source",
                     .contentPath = "content"};

  ASSERT_TRUE(db.createLibraryEntry(entry));
  ASSERT_EQ(db.getAllLibraryEntries().size(), 1);

  auto result = db.getLibraryEntry(entry.id);
  ASSERT_TRUE(result.has_value());
  ASSERT_EQ(result->id, entry.id);
  ASSERT_EQ(result->displayName, entry.displayName);
  ASSERT_EQ(result->contentMd5, entry.contentMd5);
  ASSERT_EQ(result->platformId, entry.platformId);
  ASSERT_EQ(result->type, entry.type);
  ASSERT_EQ(result->sourceDirectory, entry.sourceDirectory);
  ASSERT_EQ(result->contentPath, entry.contentPath);
}

TEST_F(SqliteLibraryDatabaseTest, GetLibraryEntryWhenDoesntExist) {
  SqliteLibraryDatabase db(temp_file_path.string());

  ASSERT_TRUE(db.getAllLibraryEntries().empty());
  ASSERT_FALSE(db.getLibraryEntry(1).has_value());
}

TEST_F(SqliteLibraryDatabaseTest, AddEntryToPlaylist) {
  SqliteLibraryDatabase db(temp_file_path.string());
  Playlist playlist{.id = -1, .displayName = "Test Playlist"};
  LibraryEntry entry{.id = -1,
                     .displayName = "Test Playlist",
                     .contentMd5 = "1234567890",
                     .platformId = 1,
                     .type = LibraryEntry::EntryType::ROM,
                     .sourceDirectory = "source",
                     .contentPath = "content"};

  ASSERT_TRUE(db.createPlaylist(playlist));
  ASSERT_TRUE(db.createLibraryEntry(entry));
  ASSERT_TRUE(db.addEntryToPlaylist(playlist.id, entry.id));
}

TEST_F(SqliteLibraryDatabaseTest, AddEntryToPlaylistFailsOnDuplicate) {
  SqliteLibraryDatabase db(temp_file_path.string());
  Playlist playlist{.id = -1, .displayName = "Test Playlist"};
  LibraryEntry entry{.id = -1,
                     .displayName = "Test Playlist",
                     .contentMd5 = "1234567890",
                     .platformId = 1,
                     .type = LibraryEntry::EntryType::ROM,
                     .sourceDirectory = "source",
                     .contentPath = "content"};

  ASSERT_TRUE(db.createPlaylist(playlist));
  ASSERT_TRUE(db.createLibraryEntry(entry));
  ASSERT_TRUE(db.addEntryToPlaylist(playlist.id, entry.id));
  ASSERT_FALSE(db.addEntryToPlaylist(playlist.id, entry.id));
}

TEST_F(SqliteLibraryDatabaseTest, GetPlaylistsForEntry) {
  SqliteLibraryDatabase db(temp_file_path.string());
  Playlist playlist{.id = -1, .displayName = "Test Playlist"};
  LibraryEntry entry{.id = -1,
                     .displayName = "Test Playlist",
                     .contentMd5 = "1234567890",
                     .platformId = 1,
                     .type = LibraryEntry::EntryType::ROM,
                     .sourceDirectory = "source",
                     .contentPath = "content"};

  ASSERT_TRUE(db.createPlaylist(playlist));
  ASSERT_TRUE(db.createLibraryEntry(entry));
  ASSERT_TRUE(db.addEntryToPlaylist(playlist.id, entry.id));

  const auto playlists = db.getPlaylistsForEntry(entry.id);
  ASSERT_EQ(playlists.size(), 1);
  ASSERT_EQ(playlists[0].id, playlist.id);
  ASSERT_EQ(playlists[0].displayName, playlist.displayName);
}

TEST_F(SqliteLibraryDatabaseTest, GetPlaylistsForEntryWhenDoesntExist) {
  SqliteLibraryDatabase db(temp_file_path.string());

  ASSERT_TRUE(db.getAllPlaylists().empty());
  ASSERT_TRUE(db.getPlaylistsForEntry(1).empty());
}

TEST_F(SqliteLibraryDatabaseTest, DeleteLibraryEntry) {
  SqliteLibraryDatabase db(temp_file_path.string());
  LibraryEntry entry{.id = -1,
                     .displayName = "Test Playlist",
                     .contentMd5 = "1234567890",
                     .platformId = 1,
                     .type = LibraryEntry::EntryType::ROM,
                     .sourceDirectory = "source",
                     .contentPath = "content"};

  ASSERT_TRUE(db.createLibraryEntry(entry));
  ASSERT_EQ(db.getAllLibraryEntries().size(), 1);
  ASSERT_TRUE(db.deleteLibraryEntry(entry.id));
  ASSERT_TRUE(db.getAllLibraryEntries().empty());
}

} // namespace firelight::db
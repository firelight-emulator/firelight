#include <gtest/gtest.h>
#include <library/sqlite_user_library.hpp>

namespace firelight::db {
class SqliteUserLibraryTest : public testing::Test {};

TEST_F(SqliteUserLibraryTest, CreateFolderSetsIdTest) {
  auto library = library::SqliteUserLibrary(":memory:", "file.txt");
  auto info = library::FolderInfo{.displayName = "test"};

  ASSERT_TRUE(library.create(info));
  ASSERT_NE(info.id, -1);
}

TEST_F(SqliteUserLibraryTest, CreateFolderWithExistingNameTest) {
  auto library = library::SqliteUserLibrary(":memory:", "file.txt");
  auto info = library::FolderInfo{.displayName = "test"};

  ASSERT_TRUE(library.create(info));
  ASSERT_NE(info.id, -1);

  auto info2 = library::FolderInfo{.displayName = "test"};
  ASSERT_FALSE(library.create(info2));
  ASSERT_EQ(info2.id, -1);
}

TEST_F(SqliteUserLibraryTest, ListFoldersTest) {
  auto library = library::SqliteUserLibrary(":memory:", "file.txt");

  ASSERT_TRUE(library.listFolders({}).empty());

  auto info = library::FolderInfo{.displayName = "test",
                                  .description = "test description",
                                  .iconSourceUrl = "testurl"};
  ASSERT_TRUE(library.create(info));

  const auto folders = library.listFolders({});
  ASSERT_EQ(folders.size(), 1);
  ASSERT_EQ(folders[0].id, info.id);
  ASSERT_EQ(folders[0].displayName, info.displayName);
  ASSERT_EQ(folders[0].description, info.description);
  ASSERT_EQ(folders[0].iconSourceUrl, info.iconSourceUrl);
}

TEST_F(SqliteUserLibraryTest, UpdateFolderTest) {
  auto library = library::SqliteUserLibrary(":memory:", "file.txt");

  auto info = library::FolderInfo{.displayName = "test"};
  ASSERT_TRUE(library.create(info));

  auto folderEntry =
      library::FolderEntryInfo{.folderId = info.id, .entryId = 1};
  ASSERT_TRUE(library.create(folderEntry));

  auto folders = library.listFolders({});
  ASSERT_EQ(folders.size(), 1);
  ASSERT_EQ(folders[0].id, info.id);
  ASSERT_EQ(folders[0].displayName, info.displayName);
  ASSERT_EQ(folders[0].description, info.description);

  info.description = "test description";
  info.iconSourceUrl = "testurl";

  ASSERT_TRUE(library.update(info));

  folders = library.listFolders({});
  ASSERT_EQ(folders.size(), 1);
  ASSERT_EQ(folders[0].id, info.id);
  ASSERT_EQ(folders[0].displayName, info.displayName);
  ASSERT_EQ(folders[0].description, info.description);
}

TEST_F(SqliteUserLibraryTest, UpdateFolderInvalidIdTest) {
  auto library = library::SqliteUserLibrary(":memory:", "file.txt");

  auto info = library::FolderInfo{.displayName = "test"};
  ASSERT_TRUE(library.create(info));

  auto folderEntry =
      library::FolderEntryInfo{.folderId = info.id, .entryId = 1};
  ASSERT_TRUE(library.create(folderEntry));

  auto folders = library.listFolders({});
  ASSERT_EQ(folders.size(), 1);

  info.id = -1; // Set to an invalid ID
  ASSERT_FALSE(library.update(info));
}

TEST_F(SqliteUserLibraryTest, UpdateFolderThatDoesntExistTest) {
  auto library = library::SqliteUserLibrary(":memory:", "file.txt");

  ASSERT_EQ(0, library.listFolders({}).size());

  auto info = library::FolderInfo{.displayName = "test"};
  info.id = 1; // Set to a invalid ID
  ASSERT_FALSE(library.update(info));
}

TEST_F(SqliteUserLibraryTest, DeleteFolderTest) {
  auto library = library::SqliteUserLibrary(":memory:", "file.txt");

  ASSERT_TRUE(library.listFolders({}).empty());

  auto info = library::FolderInfo{.displayName = "test"};
  ASSERT_TRUE(library.create(info));

  const auto folders = library.listFolders({});
  ASSERT_EQ(folders.size(), 1);
  ASSERT_EQ(folders[0].id, info.id);

  ASSERT_TRUE(library.deleteFolder(info.id));
  ASSERT_TRUE(library.listFolders({}).empty());
}

TEST_F(SqliteUserLibraryTest, CreateFolderEntryTest) {
  auto library = library::SqliteUserLibrary(":memory:", "file.txt");

  auto info = library::FolderInfo{.displayName = "test"};
  ASSERT_TRUE(library.create(info));

  auto folderEntry =
      library::FolderEntryInfo{.folderId = info.id, .entryId = 1};
  ASSERT_TRUE(library.create(folderEntry));

  // TODO: Get entry and make sure the folder is associated with it
}

} // namespace firelight::db

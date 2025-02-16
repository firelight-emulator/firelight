#include "../../../src/app/db/sqlite_content_database.hpp"
#include <gtest/gtest.h>

namespace firelight::db {

class SqliteContentDatabaseTest : public testing::Test {
protected:
  std::filesystem::path db_path = "test_resources/test-content.db";
};

TEST_F(SqliteContentDatabaseTest, ConstructorTest) {
  SqliteContentDatabase db(QString::fromStdString(db_path.string()));

  ASSERT_FALSE(db.tableExists("ouipahedslifuhasdf"));

  ASSERT_TRUE(db.tableExists("games"));
  ASSERT_TRUE(db.tableExists("game_external_ids"));
  ASSERT_TRUE(db.tableExists("roms"));
  ASSERT_TRUE(db.tableExists("rom_external_ids"));
  ASSERT_TRUE(db.tableExists("rom_regions"));
  ASSERT_TRUE(db.tableExists("patches"));
  ASSERT_TRUE(db.tableExists("platforms"));
  ASSERT_TRUE(db.tableExists("platform_external_ids"));
  ASSERT_TRUE(db.tableExists("regions"));
  ASSERT_TRUE(db.tableExists("region_external_ids"));
}

TEST_F(SqliteContentDatabaseTest, GetGameTest) {}
TEST_F(SqliteContentDatabaseTest, GetRomTest) {}
TEST_F(SqliteContentDatabaseTest, GetMatchingRomsTest) {}
TEST_F(SqliteContentDatabaseTest, GetModTest) {}
TEST_F(SqliteContentDatabaseTest, GetAllModsTest) {}
TEST_F(SqliteContentDatabaseTest, GetPatchTest) {}
TEST_F(SqliteContentDatabaseTest, GetMatchingPatchesTest) {}
TEST_F(SqliteContentDatabaseTest, GetPlatformTest) {}
TEST_F(SqliteContentDatabaseTest, GetMatchingPlatformsTest) {}
TEST_F(SqliteContentDatabaseTest, GetRegion) {}

} // namespace firelight::db
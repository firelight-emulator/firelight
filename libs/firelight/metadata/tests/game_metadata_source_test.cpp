#include <firelight/metadata/sqlite_game_metadata_source.hpp>

#include <SQLiteCpp/Database.h>
#include <gtest/gtest.h>

#include <atomic>
#include <filesystem>
#include <string>

namespace firelight::metadata {
  namespace {
    std::string makeFixtureDb() {
      static std::atomic counter{0};
      const auto path =
          (std::filesystem::temp_directory_path() /
           ("fl_meta_fixture_" + std::to_string(counter.fetch_add(1)) + ".db"))
          .string();
      std::filesystem::remove(path);

      SQLite::Database db(path, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
      db.exec("CREATE TABLE games(id INTEGER PRIMARY KEY, name TEXT, description "
        "TEXT, developer TEXT, publisher TEXT, genre TEXT, release_year "
        "INTEGER, release_date TEXT, region TEXT, players TEXT, ra_game_id "
        "INTEGER, platform_id INTEGER);");
      db.exec("CREATE TABLE game_hashes(content_hash TEXT PRIMARY KEY, game_id "
        "INTEGER);");
      db.exec("CREATE TABLE game_media(game_id INTEGER, media_type INTEGER, url "
        "TEXT);");
      db.exec("INSERT INTO games VALUES(1, 'Super Metroid', 'A game', 'Nintendo "
        "R&D1', 'Nintendo', 'Action', 1994, '1994-03-19', 'US', '1', 10003, "
        "6);");
      db.exec("INSERT INTO game_hashes VALUES('hashA', 1);");
      db.exec("INSERT INTO game_media VALUES(1, 0, 'https://cdn/icon.png');");
      db.exec("INSERT INTO game_media VALUES(1, 1, 'https://cdn/box.png');");
      return path;
    }
  } // namespace

  TEST(GameMetadataSourceTest, LooksUpKnownHash) {
    const auto path = makeFixtureDb(); {
      // scope the source so it releases the file before cleanup
      SqliteGameMetadataSource source(path);

      const auto md = source.lookup("hashA");
      ASSERT_TRUE(md.has_value());
      EXPECT_EQ(md->name, "Super Metroid");
      EXPECT_EQ(md->developer, "Nintendo R&D1");
      EXPECT_EQ(md->publisher, "Nintendo");
      EXPECT_EQ(md->genre, "Action");
      EXPECT_EQ(md->releaseYear, 1994u);
      EXPECT_EQ(md->retroAchievementsId, 10003u);
      EXPECT_EQ(md->platformId, 6);

      ASSERT_EQ(md->media.size(), 2u);
      bool hasIcon = false, hasBox = false;
      for (const auto &m: md->media) {
        if (m.type == MediaType::Icon && m.url == "https://cdn/icon.png") {
          hasIcon = true;
        }
        if (m.type == MediaType::BoxartFront && m.url == "https://cdn/box.png") {
          hasBox = true;
        }
      }
      EXPECT_TRUE(hasIcon);
      EXPECT_TRUE(hasBox);
    }
    std::filesystem::remove(path);
  }

  TEST(GameMetadataSourceTest, UnknownHashReturnsNullopt) {
    const auto path = makeFixtureDb(); {
      SqliteGameMetadataSource source(path);
      EXPECT_FALSE(source.lookup("not-a-hash").has_value());
    }
    std::filesystem::remove(path);
  }

  TEST(GameMetadataSourceTest, MissingDatabaseIsTolerated) {
    SqliteGameMetadataSource source(
      (std::filesystem::temp_directory_path() / "fl_meta_absent_zzz.db")
      .string());
    EXPECT_FALSE(source.lookup("hashA").has_value());
  }
} // namespace firelight::metadata

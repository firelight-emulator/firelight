#include "app/metadata/metadata_service.hpp"

#include <firelight/library/entry.hpp>
#include <firelight/library/sqlite_user_library.hpp>
#include <firelight/metadata/art_provider.hpp>
#include <firelight/metadata/sqlite_game_metadata_source.hpp>
#include <firelight/metadata/sqlite_media_asset_repository.hpp>

#include <QThread>
#include <SQLiteCpp/Database.h>
#include <atomic>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

// Verifies MetadataService auto-population: the shipped-source name/metadata/art
// is written onto the entry and seeded into the media store, a user-renamed
// entry keeps its name, and the EntryCreatedEvent path triggers population
namespace firelight::metadata {
namespace {

std::string makeFixtureDb() {
  static std::atomic<int> counter{0};
  const auto path =
      (std::filesystem::temp_directory_path() / ("fl_metasvc_fixture_" + std::to_string(counter.fetch_add(1)) + ".db"))
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
  db.exec("INSERT INTO games VALUES(1, 'Super Metroid', 'desc', 'Nintendo "
          "R&D1', 'Nintendo', 'Action', 1994, '1994-03-19', 'US', '1', 10003, "
          "6);");
  db.exec("INSERT INTO game_hashes VALUES('hashA', 1);");
  db.exec("INSERT INTO game_media VALUES(1, 0, 'https://cdn/icon.png');");
  db.exec("INSERT INTO game_media VALUES(1, 1, 'https://cdn/box.png');");
  return path;
}

} // namespace

class MetadataServiceTest : public testing::Test {
protected:
  std::string metadataDbPath = makeFixtureDb();
  std::string mediaDir = (std::filesystem::temp_directory_path() / "fl_metasvc_media").string();
  library::SqliteUserLibraryRepository library{":memory:"};
  SqliteMediaAssetRepository media{":memory:"};
  SqliteGameMetadataSource source{metadataDbPath};

  void TearDown() override {
    // The source member still holds the file open here; best-effort cleanup
    std::error_code ec;
    std::filesystem::remove(metadataDbPath, ec);
  }

  // Creates an entry. EntryCreatedEvent fires here; construct the service after
  // this call to avoid a concurrent async populate in the deterministic tests
  int makeEntry(const std::string &name, const std::string &hash, unsigned platformId) {
    library::Entry entry;
    entry.displayName = name;
    entry.contentHash = hash;
    entry.platformId = platformId;
    library.createEntry(entry);
    return entry.id;
  }
};

TEST_F(MetadataServiceTest, PopulatesNameArtAndMetadata) {
  const int id = makeEntry("mario.sfc", "hashA", 6);
  MetadataService service(library, source, media, mediaDir);
  service.populate(id);

  const auto entry = library.getEntry(id);
  ASSERT_TRUE(entry.has_value());
  EXPECT_EQ(entry->displayName, "Super Metroid");
  EXPECT_EQ(entry->developer, "Nintendo R&D1");
  EXPECT_EQ(entry->genres, "Action");
  EXPECT_EQ(entry->releaseYear, 1994u);
  EXPECT_EQ(entry->retroachievementsSetId, 10003u);
  EXPECT_EQ(entry->icon1x1SourceUrl, "https://cdn/icon.png");
  EXPECT_EQ(entry->boxartFrontSourceUrl, "https://cdn/box.png");

  // Media store seeded with the RA defaults, icon selected
  const auto icon = media.selectedFor("hashA", MediaType::Icon);
  ASSERT_TRUE(icon.has_value());
  EXPECT_EQ(icon->remoteUrl, "https://cdn/icon.png");
  EXPECT_EQ(icon->source, MediaSource::RetroAchievements);
}

TEST_F(MetadataServiceTest, DoesNotOverrideUserRenamedName) {
  const int id = makeEntry("mario.sfc", "hashA", 6);
  auto entry = library.getEntry(id);
  ASSERT_TRUE(entry.has_value());
  entry->displayName = "My Cool Name";
  entry->nameUserSet = true;
  ASSERT_TRUE(library.update(*entry));

  MetadataService service(library, source, media, mediaDir);
  service.populate(id);

  const auto after = library.getEntry(id);
  ASSERT_TRUE(after.has_value());
  EXPECT_EQ(after->displayName, "My Cool Name"); // not overridden
  // ...but the rest of the metadata + art still fills in
  EXPECT_EQ(after->developer, "Nintendo R&D1");
  EXPECT_EQ(after->icon1x1SourceUrl, "https://cdn/icon.png");
}

TEST_F(MetadataServiceTest, UnknownHashLeavesEntryUntouched) {
  const int id = makeEntry("homebrew.sfc", "no-metadata", 6);
  MetadataService service(library, source, media, mediaDir);
  service.populate(id);

  const auto entry = library.getEntry(id);
  ASSERT_TRUE(entry.has_value());
  EXPECT_EQ(entry->displayName, "homebrew.sfc"); // kept the filename
  EXPECT_TRUE(entry->icon1x1SourceUrl.empty());
}

TEST_F(MetadataServiceTest, EntryCreatedEventTriggersPopulation) {
  MetadataService service(library, source, media, mediaDir); // subscribes first
  const int id = makeEntry("metroid.sfc", "hashA", 6);       // fires the event

  bool populated = false;
  for (int i = 0; i < 300; ++i) { // ~3s budget for the background worker
    const auto entry = library.getEntry(id);
    if (entry && entry->displayName == "Super Metroid") {
      populated = true;
      break;
    }
    QThread::msleep(10);
  }
  EXPECT_TRUE(populated);
}

// Picking an online candidate stores it (selected) and reprojects onto the
// entry's icon column
TEST_F(MetadataServiceTest, ApplyCandidateSelectsAndReprojects) {
  const int id = makeEntry("mario.sfc", "hashA", 6);
  MetadataService service(library, source, media, mediaDir);
  service.populate(id); // RA icon selected first

  ArtCandidate candidate;
  candidate.type = MediaType::Icon;
  candidate.url = "https://sgdb/custom-icon.png";
  candidate.thumbUrl = "https://sgdb/custom-icon_thumb.png";
  candidate.externalId = "999";
  service.applyCandidate("hashA", candidate);

  const auto selected = media.selectedFor("hashA", MediaType::Icon);
  ASSERT_TRUE(selected.has_value());
  EXPECT_EQ(selected->source, MediaSource::SteamGridDb);
  EXPECT_EQ(selected->remoteUrl, "https://sgdb/custom-icon.png"); // full kept
  EXPECT_EQ(selected->thumbUrl, "https://sgdb/custom-icon_thumb.png");

  // The entry's grid icon uses the thumbnail (crisp at tile size), not the
  // possibly-low-res full original
  const auto entry = library.getEntry(id);
  ASSERT_TRUE(entry.has_value());
  EXPECT_EQ(entry->icon1x1SourceUrl, "https://sgdb/custom-icon_thumb.png");
}

// Selecting a previously-stored asset flips the selection back to it
TEST_F(MetadataServiceTest, SelectAssetSwitchesBack) {
  const int id = makeEntry("mario.sfc", "hashA", 6);
  MetadataService service(library, source, media, mediaDir);
  service.populate(id);
  const auto raIcon = media.selectedFor("hashA", MediaType::Icon);
  ASSERT_TRUE(raIcon.has_value());

  ArtCandidate candidate;
  candidate.type = MediaType::Icon;
  candidate.url = "https://sgdb/custom-icon.png";
  service.applyCandidate("hashA", candidate);
  ASSERT_NE(media.selectedFor("hashA", MediaType::Icon)->id, raIcon->id);

  service.selectAsset("hashA", raIcon->id);
  const auto entry = library.getEntry(id);
  ASSERT_TRUE(entry.has_value());
  EXPECT_EQ(entry->icon1x1SourceUrl, raIcon->remoteUrl);
}

// Importing a local image copies it into the media dir and selects it
TEST_F(MetadataServiceTest, ImportLocalImageCopiesAndSelects) {
  const int id = makeEntry("mario.sfc", "hashA", 6);

  // A throwaway source image on disk
  std::error_code ec;
  std::filesystem::create_directories(mediaDir, ec);
  const auto src = (std::filesystem::path(mediaDir) / "src.png").string();
  {
    std::ofstream f(src, std::ios::binary);
    f << "PNGDATA";
  }

  MetadataService service(library, source, media, mediaDir);
  ASSERT_TRUE(service.importLocalImage("hashA", MediaType::Icon, src));

  const auto selected = media.selectedFor("hashA", MediaType::Icon);
  ASSERT_TRUE(selected.has_value());
  EXPECT_EQ(selected->source, MediaSource::User);
  EXPECT_FALSE(selected->localPath.empty());
  EXPECT_TRUE(std::filesystem::exists(selected->localPath));

  const auto entry = library.getEntry(id);
  ASSERT_TRUE(entry.has_value());
  EXPECT_EQ(entry->icon1x1SourceUrl, selected->localPath);

  std::filesystem::remove_all(mediaDir, ec);
}

} // namespace firelight::metadata

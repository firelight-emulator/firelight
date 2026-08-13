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
  db.exec("INSERT INTO games VALUES(2, 'Multi Genre', '', '', '', 'Action, Adventure', 1996, '', "
          "'Japan', '1', 0, 6);");
  db.exec("INSERT INTO game_hashes VALUES('hashA', 1);");
  db.exec("INSERT INTO game_hashes VALUES('multiHash', 2);");
  db.exec("INSERT INTO game_media VALUES(1, 0, 'https://cdn/icon.png');");
  db.exec("INSERT INTO game_media VALUES(1, 1, 'https://cdn/box.png');");
  return path;
}

// Stands in for SteamGridDB: records what it was asked and answers with whatever the
// test set up
class FakeArtProvider final : public IArtProvider {
public:
  bool configured = true;
  ArtSearchStatus status = ArtSearchStatus::Ok;
  std::vector<ArtCandidate> results;
  std::vector<std::string> searchedTitles;

  [[nodiscard]] std::string name() const override { return "Fake"; }

  [[nodiscard]] bool isConfigured() const override { return configured; }

  [[nodiscard]] ArtSearchResult search(const std::string &gameName, int, MediaType) override {
    searchedTitles.push_back(gameName);
    return ArtSearchResult{.status = status, .candidates = results};
  }
};

// Stands in for a source that knows more than the shipped one does
class RichSource final : public IGameMetadataSource {
public:
  [[nodiscard]] std::optional<MetadataLookup> lookup(const std::string &) override {
    MetadataLookup result;
    result.metadata.revision = "2";
    result.metadata.languages = {"en", "ja"};
    result.metadata.flags = {"verified-dump"};
    return result;
  }
};

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

  // Gives an entry an on-disk path, which is where the filename layer reads its tags
  void giveContentFile(const std::string &hash, const std::string &path) {
    library::ContentFile file;
    file.m_filePath = path;
    file.m_fileSizeBytes = 1;
    file.m_fileMd5 = hash;
    file.m_fileCrc32 = "0";
    file.m_platformId = 6;
    file.m_contentHash = hash;
    library.create(file);
  }
};

// The shipped database ships empty, so an unmatched hash is the common case rather
// than the exception: the filename still has to yield regions and languages
TEST_F(MetadataServiceTest, FilenameTagsPopulateWhenTheDatabaseMisses) {
  const int id = makeEntry("Some Game", "unknownHash", 6);
  giveContentFile("unknownHash", "C:/roms/Some Game (Japan) (En,Fr) (Rev 1).sfc");

  MetadataService service(library, source, media, mediaDir);
  service.populate(id);

  const auto entry = library.getEntry(id);
  ASSERT_TRUE(entry.has_value());
  EXPECT_EQ(entry->metadata.regions, (std::vector<std::string>{"JP"}));
  EXPECT_EQ(entry->metadata.languages, (std::vector<std::string>{"en", "fr"}));
  EXPECT_EQ(entry->metadata.revision, "1");
}

// The tile falls down the ladder rather than staying blank: a game whose only art
// is a screenshot still shows something in the grid
TEST_F(MetadataServiceTest, TheTileFallsBackToArtOfAnotherType) {
  const int id = makeEntry("Some Game", "unknownHash", 6);

  MediaAsset screenshot;
  screenshot.contentHash = "unknownHash";
  screenshot.type = MediaType::TitleScreen;
  screenshot.source = MediaSource::SteamGridDb;
  screenshot.remoteUrl = "https://cdn/title.png";
  ASSERT_TRUE(media.add(screenshot));

  MetadataService service(library, source, media, mediaDir);
  service.populate(id);

  const auto entry = library.getEntry(id);
  ASSERT_TRUE(entry.has_value());
  EXPECT_EQ(entry->icon1x1SourceUrl, "https://cdn/title.png");
}

// Art applied from a title search is a guess, and has to stay identifiable as one
TEST_F(MetadataServiceTest, FetchedArtIsSelectedAndFlaggedUnconfirmed) {
  const int id = makeEntry("Chrono Trigger (USA) (Rev 1).sfc", "unknownHash", 6);

  FakeArtProvider provider;
  provider.results = {ArtCandidate{.type = MediaType::GridSquare,
                                   .url = "https://sgdb/grid.png",
                                   .thumbUrl = "https://sgdb/grid_thumb.png",
                                   .externalId = "42"}};

  MetadataService service(library, source, media, mediaDir, &provider);
  service.fetchArt(id);

  const auto selected = media.selectedFor("unknownHash", MediaType::GridSquare);
  ASSERT_TRUE(selected.has_value());
  EXPECT_TRUE(selected->unconfirmed);
  EXPECT_EQ(selected->source, MediaSource::SteamGridDb);

  const auto entry = library.getEntry(id);
  ASSERT_TRUE(entry.has_value());
  EXPECT_EQ(entry->icon1x1SourceUrl, "https://sgdb/grid_thumb.png");

  // Searched by the title with its tags stripped, not the raw filename
  ASSERT_EQ(provider.searchedTitles.size(), 1u);
  EXPECT_EQ(provider.searchedTitles.front(), "Chrono Trigger");
}

// A poor match is only findable later if what it matched, and how well, is kept
TEST_F(MetadataServiceTest, AReviewItemPairsThePoorMatchWithTheGameThatGotIt) {
  const int id = makeEntry("La Wares (Japan).sfc", "unknownHash", 6);
  giveContentFile("unknownHash", "C:/roms/La Wares (Japan).sfc");

  FakeArtProvider provider;
  provider.results = {ArtCandidate{.type = MediaType::GridSquare,
                                   .url = "https://sgdb/grid.png",
                                   .thumbUrl = "https://sgdb/grid_thumb.png",
                                   .externalId = "42",
                                   .gameName = "La-Mulana",
                                   .matchScore = 1}};

  MetadataService service(library, source, media, mediaDir, &provider);
  service.populate(id);
  service.fetchArt(id);

  const auto review = service.getArtReviewItems();
  ASSERT_EQ(review.size(), 1u);
  EXPECT_EQ(review.front().entryId, id);
  EXPECT_EQ(review.front().displayName, "La Wares");
  EXPECT_EQ(review.front().matchedName, "La-Mulana");
  EXPECT_EQ(review.front().matchScore, 1);
  EXPECT_EQ(review.front().thumbUrl, "https://sgdb/grid_thumb.png");
}

// The source and the entry hold the same shape now, so a source can answer for fields that
// used to be reachable only by parsing a filename
TEST_F(MetadataServiceTest, ASourceCanSupplyRevisionAndLanguages) {
  const int id = makeEntry("Whatever.sfc", "richHash", 6);

  RichSource rich;
  MetadataService service(library, rich, media, mediaDir);
  service.populate(id);

  const auto entry = library.getEntry(id);
  ASSERT_TRUE(entry.has_value());
  EXPECT_EQ(entry->metadata.revision, "2");
  EXPECT_EQ(entry->metadata.languages, (std::vector<std::string>{"en", "ja"}));
  EXPECT_EQ(entry->metadata.flags, (std::vector<std::string>{"verified-dump"}));
}

// A row naming several genres reaches the entry as several genres, not as one long one
TEST_F(MetadataServiceTest, SeveralGenresArriveSeparately) {
  const int id = makeEntry("Multi.sfc", "multiHash", 6);

  MetadataService service(library, source, media, mediaDir);
  service.populate(id);

  const auto entry = library.getEntry(id);
  ASSERT_TRUE(entry.has_value());
  EXPECT_EQ(entry->metadata.genres, (std::vector<std::string>{"Action", "Adventure"}));
}

// The grouping key comes off the parsed title, so two dumps of one game agree on it
TEST_F(MetadataServiceTest, PopulateStoresTheGroupingTitleFromTheFilename) {
  const int usa = makeEntry("Zelda (USA).sfc", "hashUsa", 6);
  giveContentFile("hashUsa", "C:/roms/Zelda (USA).sfc");
  const int japan = makeEntry("Zelda (Japan) (Rev 1).sfc", "hashJapan", 6);
  giveContentFile("hashJapan", "C:/roms/Zelda (Japan) (Rev 1).sfc");

  MetadataService service(library, source, media, mediaDir);
  service.populate(usa);
  service.populate(japan);

  const auto first = library.getEntry(usa);
  const auto second = library.getEntry(japan);
  ASSERT_TRUE(first.has_value());
  ASSERT_TRUE(second.has_value());
  EXPECT_EQ(first->normalizedTitle, "zelda");
  EXPECT_EQ(second->normalizedTitle, "zelda");
}

// Renaming a game must not move it out of the set it belongs to
TEST_F(MetadataServiceTest, AUserRenameDoesNotChangeTheGroupingTitle) {
  const int id = makeEntry("Zelda (USA).sfc", "hashUsa", 6);
  giveContentFile("hashUsa", "C:/roms/Zelda (USA).sfc");

  auto entry = library.getEntry(id);
  ASSERT_TRUE(entry.has_value());
  entry->displayName = "My Favourite Game";
  entry->nameUserSet = true;
  ASSERT_TRUE(library.update(*entry));

  MetadataService service(library, source, media, mediaDir);
  service.populate(id);

  const auto after = library.getEntry(id);
  ASSERT_TRUE(after.has_value());
  EXPECT_EQ(after->displayName, "My Favourite Game");
  EXPECT_EQ(after->normalizedTitle, "zelda");
}

// Otherwise a game the provider has nothing for is looked up again on every startup
TEST_F(MetadataServiceTest, AnEmptyResultStillCountsAsTried) {
  const int id = makeEntry("Obscure Homebrew", "unknownHash", 6);

  FakeArtProvider provider;

  MetadataService service(library, source, media, mediaDir, &provider);
  service.fetchArt(id);

  auto entry = library.getEntry(id);
  ASSERT_TRUE(entry.has_value());
  EXPECT_TRUE(entry->artFetchedAt.has_value());

  // A second pass asks nothing, because the first already answered
  service.fetchArt(id);
  EXPECT_EQ(provider.searchedTitles.size(), 1u);
}

// No key means not yet attempted rather than attempted and failed, so supplying one
// later must not find the whole library already accounted for
TEST_F(MetadataServiceTest, AnUnconfiguredProviderLeavesTheEntryUnmarked) {
  const int id = makeEntry("Some Game", "unknownHash", 6);

  FakeArtProvider provider;
  provider.configured = false;

  MetadataService service(library, source, media, mediaDir, &provider);
  service.fetchArt(id);

  const auto entry = library.getEntry(id);
  ASSERT_TRUE(entry.has_value());
  EXPECT_FALSE(entry->artFetchedAt.has_value());
  EXPECT_TRUE(provider.searchedTitles.empty());
}

TEST_F(MetadataServiceTest, ArtIsNotFetchedForAGameThatAlreadyHasSome) {
  const int id = makeEntry("mario.sfc", "hashA", 6);

  FakeArtProvider provider;
  MetadataService service(library, source, media, mediaDir, &provider);

  // hashA resolves in the shipped fixture, so this seeds art from there
  service.populate(id);
  service.fetchArt(id);

  EXPECT_TRUE(provider.searchedTitles.empty());

  const auto entry = library.getEntry(id);
  ASSERT_TRUE(entry.has_value());
  EXPECT_TRUE(entry->artFetchedAt.has_value());
}

// The sweep works a queue rather than the whole library, so a tick asks for one
// entry at a time and the marker is what advances it
TEST_F(MetadataServiceTest, TheSweepWorksThroughEveryEntryAndThenStops) {
  const int first = makeEntry("Alpha", "hashOne", 6);
  const int second = makeEntry("Bravo", "hashTwo", 6);

  // Art is only looked up for a game there is something to launch
  giveContentFile("hashOne", "/roms/Alpha.sfc");
  giveContentFile("hashTwo", "/roms/Bravo.sfc");

  ASSERT_EQ(library.getEntryIdsMissingArt(10).size(), 2u);

  FakeArtProvider provider;
  MetadataService service(library, source, media, mediaDir, &provider);

  service.fetchArt(first);
  EXPECT_EQ(library.getEntryIdsMissingArt(10), (std::vector<int>{second}));

  service.fetchArt(second);
  EXPECT_TRUE(library.getEntryIdsMissingArt(10).empty());
}

// A hidden entry has no file to play, so spending a request on its art is waste
TEST_F(MetadataServiceTest, TheSweepSkipsHiddenEntries) {
  const int id = makeEntry("Missing", "hashHidden", 6);

  auto entry = library.getEntry(id);
  ASSERT_TRUE(entry.has_value());
  ASSERT_TRUE(library.setEntryHidden(entry->id, true));

  EXPECT_TRUE(library.getEntryIdsMissingArt(10).empty());
}

// A failed request answers nothing about the game, so recording it as "no art"
// would give up on that entry permanently
TEST_F(MetadataServiceTest, AFailedLookupIsNotRecordedAsTried) {
  const int id = makeEntry("Some Game", "unknownHash", 6);

  FakeArtProvider provider;
  provider.status = ArtSearchStatus::RateLimited;

  MetadataService service(library, source, media, mediaDir, &provider);
  EXPECT_EQ(service.fetchArt(id), ArtSearchStatus::RateLimited);

  auto entry = library.getEntry(id);
  ASSERT_TRUE(entry.has_value());
  EXPECT_FALSE(entry->artFetchedAt.has_value());

  // ...and once the provider recovers, the entry is still waiting to be asked about
  provider.status = ArtSearchStatus::Ok;
  EXPECT_EQ(service.fetchArt(id), ArtSearchStatus::Ok);

  entry = library.getEntry(id);
  ASSERT_TRUE(entry.has_value());
  EXPECT_TRUE(entry->artFetchedAt.has_value());
}

TEST_F(MetadataServiceTest, DatabaseRegionOverridesTheFilename) {
  const int id = makeEntry("mario.sfc", "hashA", 6);
  giveContentFile("hashA", "C:/roms/Super Metroid (Japan).sfc");

  MetadataService service(library, source, media, mediaDir);
  service.populate(id);

  const auto entry = library.getEntry(id);
  ASSERT_TRUE(entry.has_value());
  EXPECT_EQ(entry->metadata.regions, (std::vector<std::string>{"US"}));
}

// The whole point of the override set: a scrape must not undo a user's typing
TEST_F(MetadataServiceTest, AUserEditSurvivesARescrape) {
  const int id = makeEntry("mario.sfc", "hashA", 6);

  GameMetadata edit;
  edit.description = "My own words";
  ASSERT_TRUE(library.applyEntryMetadata(id, edit, {metadata_fields::DESCRIPTION}, true));

  MetadataService service(library, source, media, mediaDir);
  service.populate(id);

  const auto entry = library.getEntry(id);
  ASSERT_TRUE(entry.has_value());
  EXPECT_EQ(entry->metadata.description, "My own words");
  // Everything the user did not pin still fills in
  EXPECT_EQ(entry->metadata.developer, "Nintendo R&D1");
}

TEST_F(MetadataServiceTest, PopulatesNameArtAndMetadata) {
  const int id = makeEntry("mario.sfc", "hashA", 6);
  MetadataService service(library, source, media, mediaDir);
  service.populate(id);

  const auto entry = library.getEntry(id);
  ASSERT_TRUE(entry.has_value());
  EXPECT_EQ(entry->displayName, "Super Metroid");
  EXPECT_EQ(entry->metadata.developer, "Nintendo R&D1");
  EXPECT_EQ(entry->metadata.genres, (std::vector<std::string>{"Action"}));
  EXPECT_EQ(entry->metadata.releaseYear, 1994u);
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
  EXPECT_EQ(after->metadata.developer, "Nintendo R&D1");
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

// Populating is a repeatable operation now, and every write wakes both groupers. A re-run that
// finds nothing new has to be silent rather than merely harmless
TEST_F(MetadataServiceTest, PopulatingTwiceWritesNothingTheSecondTime) {
  const auto id = makeEntry("Grandia (USA).cue", "hash-grandia", 7);
  giveContentFile("hash-grandia", "C:/roms/Grandia (USA).cue");

  MetadataService service(library, source, media, mediaDir, nullptr);

  auto updates = 0;
  ScopedConnection connection = EventDispatcher::instance().subscribe<library::EntryUpdatedEvent>(
      [&updates](const library::EntryUpdatedEvent &) { ++updates; });

  service.populate(id);
  EXPECT_GE(updates, 1) << "the first run wrote nothing at all";

  updates = 0;
  service.populate(id);
  EXPECT_EQ(updates, 0) << "a re-run with nothing new to say still published a change";
}

// The first answer is no longer permanent: a file that says more arriving later moves the
// identity onto it
TEST_F(MetadataServiceTest, ARicherFileArrivingLaterRederivesTheIdentity) {
  const auto id = makeEntry("grandia.cue", "hash-grandia", 7);
  giveContentFile("hash-grandia", "C:/roms/grandia.cue");

  MetadataService service(library, source, media, mediaDir, nullptr);
  service.populate(id);

  const auto bare = library.getEntry(id);
  ASSERT_TRUE(bare.has_value());
  EXPECT_TRUE(bare->metadata.regions.empty());

  // A second copy of the same dump, named the way a No-Intro set names it
  giveContentFile("hash-grandia", "C:/roms/Grandia (USA).cue");
  service.populate(id);

  const auto rederived = library.getEntry(id);
  ASSERT_TRUE(rederived.has_value());
  EXPECT_EQ(rederived->metadata.regions, (std::vector<std::string>{"US"}));
  EXPECT_EQ(rederived->normalizedTitle, "grandia");
}

} // namespace firelight::metadata

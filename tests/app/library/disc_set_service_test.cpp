#include <firelight/event_dispatcher.hpp>
#include <firelight/library/disc_set_playlist.hpp>
#include <firelight/library/disc_set_service.hpp>
#include <firelight/library/entry_resolver.hpp>
#include <firelight/library/filename_tags.hpp>
#include <firelight/library/library_events.hpp>
#include <firelight/library/library_ingest_service.hpp>
#include <firelight/library/sqlite_user_library.hpp>

#include <QTemporaryDir>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

// Exercises the shape change end to end: content files arrive, ingest turns them into
// entries, and metadata giving those entries a title is what folds the discs of one game
// together. The trigger is the real one, so what is asserted is what a scan produces
namespace firelight::library {

class DiscSetServiceTest : public testing::Test {
protected:
  // The service writes real playlists, so the fixture gives it somewhere of its own to do it
  QTemporaryDir m_root;
  SqliteUserLibraryRepository m_repo{":memory:"};
  LibraryIngestService m_ingest{m_repo};
  DiscSetService m_discSets{m_repo, (m_root.path() + "/appdata").toStdString()};

  std::string romsPath() const { return (m_root.path() + "/roms").toStdString(); }

  std::vector<EntryAbsorbedEvent> m_absorbed;
  ScopedConnection m_absorbedConnection = EventDispatcher::instance().subscribe<EntryAbsorbedEvent>(
      [this](const EntryAbsorbedEvent &event) { m_absorbed.push_back(event); });

  // Catalogues one disc, which is what gives it an entry of its own
  int addDisc(const std::string &title, const std::string &hash, const int discNumber, const int platformId = 7,
              const int gameId = 0) {
    ContentFile file;
    file.m_type = ContentType::Disc;
    // Two releases put their disc 2 at the same name and size, so the hash is what keeps
    // the names apart
    file.m_filePath = romsPath() + "/" + title + " (Disc " + std::to_string(discNumber) + ") [" + hash + "].cue";
    file.m_platformId = platformId;
    file.m_contentHash = hash;
    file.m_discNumber = discNumber;
    file.m_gameId = gameId;
    file.m_fileSizeBytes = 1000 + discNumber;
    EXPECT_TRUE(m_repo.create(file));
    return file.m_id;
  }

  // A disc the content database knows, catalogued and titled the way a scan does it
  void addIdentifiedDisc(const std::string &title, const std::string &hash, const int discNumber, const int gameId,
                         const std::vector<std::string> &regions = {}) {
    addDisc(title, hash, discNumber, 7, gameId);

    if (!regions.empty()) {
      setRegions(hash, regions);
    }

    nameIt(hash, title);
  }

  // Stands in for metadata population, which is what publishes the event a set forms on
  void nameIt(const std::string &hash, const std::string &title) {
    auto entry = m_repo.getEntryWithContentHash(hash);
    ASSERT_TRUE(entry.has_value());
    entry->displayName = title;
    entry->normalizedTitle = normalizeTitle(title);
    ASSERT_TRUE(m_repo.updateEntryMetadata(*entry));
  }

  // Stands in for the region a scrape or the filename tags resolved
  void setRegions(const std::string &hash, const std::vector<std::string> &regions) {
    const auto entry = m_repo.getEntryWithContentHash(hash);
    ASSERT_TRUE(entry.has_value());
    GameMetadata metadata;
    metadata.regions = regions;
    ASSERT_TRUE(m_repo.applyEntryMetadata(entry->id, metadata, {metadata_fields::REGIONS}, false));
  }

  // One disc, catalogued and titled in the order a scan does it
  int addTitledDisc(const std::string &title, const std::string &hash, const int discNumber, const int platformId = 7) {
    const auto fileId = addDisc(title, hash, discNumber, platformId);
    nameIt(hash, title);
    return fileId;
  }

  int entryCount() { return static_cast<int>(m_repo.getEntries(0, -1).size()); }

  std::vector<RunConfiguration> configsFor(const std::string &hash) { return m_repo.getRunConfigurations(hash); }

  // The identity a set launches under: the lowest disc still holding an entry
  std::string survivorOfSet(const int setId) {
    auto entries = m_repo.getEntriesInDiscSet(setId);

    if (entries.empty()) {
      return "";
    }

    std::ranges::sort(entries, [](const Entry &left, const Entry &right) {
      return std::tie(left.discNumber, left.id) < std::tie(right.discNumber, right.id);
    });

    return entries.front().contentHash;
  }

  // Where the set launching under this identity keeps its playlist
  std::string playlistFor(const std::string &hash) const {
    return playlistPathFor(hash, (m_root.path() + "/appdata").toStdString());
  }

  static std::string readFile(const std::string &path) {
    std::ifstream in(path, std::ios::binary);
    return std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
  }
};

// Membership is not presence. A set that dissolved because its discs were unreachable would null
// disc_set_id on every file and entry, and putting the discs back does not put the set back
TEST_F(DiscSetServiceTest, DiscsGoingMissingDoesNotDissolveTheSet) {
  const auto discOne = addTitledDisc("Final Fantasy VII", "disc1", 1);
  const auto discTwo = addTitledDisc("Final Fantasy VII", "disc2", 2);

  const auto survivor = m_repo.getEntryWithContentHash("disc1");
  ASSERT_TRUE(survivor.has_value());
  ASSERT_TRUE(survivor->discSetId.has_value());
  const auto setId = *survivor->discSetId;
  ASSERT_EQ(m_repo.getDiscsInSet(setId).size(), 2u);

  ASSERT_TRUE(m_repo.markContentFileMissing(discOne));
  ASSERT_TRUE(m_repo.markContentFileMissing(discTwo));

  EXPECT_EQ(m_repo.getPresentDiscsInSet(setId).size(), 0u) << "a missing disc still counts as present";
  ASSERT_EQ(m_repo.getDiscsInSet(setId).size(), 2u) << "membership went with the bytes";

  ASSERT_TRUE(m_repo.getDiscSet(setId).has_value()) << "the set was dissolved because its discs were unreachable";

  for (const auto &disc : m_repo.getDiscsInSet(setId)) {
    ASSERT_TRUE(disc.m_discSetId.has_value()) << disc.m_filePath << " lost its set";
    EXPECT_EQ(*disc.m_discSetId, setId);
  }

  const auto stillGrouped = m_repo.getEntryWithContentHash("disc1");
  ASSERT_TRUE(stillGrouped.has_value());
  ASSERT_TRUE(stillGrouped->discSetId.has_value()) << "the entry was ungrouped";
}

// The count the badge reads is presence, so a disc going away has to show up as one fewer
TEST_F(DiscSetServiceTest, AMissingDiscDropsOutOfThePresentCountAndThePlaylist) {
  addTitledDisc("Final Fantasy VII", "disc1", 1);
  const auto discTwo = addTitledDisc("Final Fantasy VII", "disc2", 2);
  addTitledDisc("Final Fantasy VII", "disc3", 3);

  const auto survivor = m_repo.getEntryWithContentHash("disc1");
  ASSERT_TRUE(survivor.has_value());
  const auto setId = *survivor->discSetId;
  ASSERT_EQ(m_repo.getPresentDiscsInSet(setId).size(), 3u);

  ASSERT_TRUE(m_repo.markContentFileMissing(discTwo));

  EXPECT_EQ(m_repo.getPresentDiscsInSet(setId).size(), 2u);

  ASSERT_TRUE(m_discSets.materializePlaylist(setId, survivorOfSet(setId)));
  const auto playlist = readFile(playlistFor(survivorOfSet(setId)));
  EXPECT_EQ(playlist.find("disc2"), std::string::npos) << "the playlist names a disc that is not there";
}

// The set's way in is anchored on the disc its identity came from, but it launches through the
// playlist. Losing that disc must not take the whole game away while the others are still here
TEST_F(DiscSetServiceTest, ASetStillLaunchesWhenTheDiscItIsAnchoredOnGoesMissing) {
  const auto discOne = addTitledDisc("Final Fantasy VII", "disc1", 1);
  addTitledDisc("Final Fantasy VII", "disc2", 2);
  addTitledDisc("Final Fantasy VII", "disc3", 3);

  const auto survivor = m_repo.getEntryWithContentHash("disc1");
  ASSERT_TRUE(survivor.has_value());
  ASSERT_TRUE(survivor->discSetId.has_value());

  EntryResolver resolver(m_repo, (m_root.path() + "/appdata").toStdString());
  ASSERT_TRUE(resolver.resolve(*survivor).valid) << "the set was unlaunchable before anything went wrong";

  ASSERT_TRUE(m_repo.markContentFileMissing(discOne));
  ASSERT_TRUE(m_discSets.materializePlaylist(*survivor->discSetId, "disc1"));

  const auto after = m_repo.getEntryWithContentHash("disc1");
  ASSERT_TRUE(after.has_value());
  EXPECT_TRUE(resolver.resolve(*after).valid) << "losing the anchor disc took a set that still has two discs away";
}

// The shape change: two discs, one row
TEST_F(DiscSetServiceTest, TwoDiscsConvergeOnOneEntry) {
  addTitledDisc("Final Fantasy VII", "disc1", 1);
  addTitledDisc("Final Fantasy VII", "disc2", 2);

  EXPECT_EQ(entryCount(), 1);

  const auto survivor = m_repo.getEntryWithContentHash("disc1");
  ASSERT_TRUE(survivor.has_value());
  ASSERT_TRUE(survivor->discSetId.has_value());
  EXPECT_FALSE(survivor->discSetUserSet);

  // Both discs are still catalogued, and both are in the set
  const auto discs = m_repo.getDiscsInSet(*survivor->discSetId);
  ASSERT_EQ(discs.size(), 2u);
  EXPECT_EQ(discs[0].m_discNumber, 1);
  EXPECT_EQ(discs[1].m_discNumber, 2);

  // No disc keeps a way in of its own, so the core is handed one path for the whole game and
  // writes one memory card
  const auto configs = configsFor("disc1");
  ASSERT_FALSE(configs.empty());
  EXPECT_TRUE(std::ranges::all_of(configs, [](const RunConfiguration &configuration) {
    return configuration.type == RunConfiguration::TYPE_PLAYLIST;
  })) << "a disc kept a way in of its own";
  EXPECT_TRUE(std::ranges::any_of(configs, [&](const RunConfiguration &configuration) {
    return configuration.discSetId == survivor->discSetId;
  })) << "the set has no way in of its own";
  EXPECT_TRUE(configsFor("disc2").empty());

  EXPECT_TRUE(std::filesystem::exists(playlistFor("disc1")));
  EXPECT_EQ(readFile(playlistFor("disc1")), romsPath() + "/Final Fantasy VII (Disc 1) [disc1].cue\n" + romsPath() +
                                                "/Final Fantasy VII (Disc 2) [disc2].cue\n");
}

// The lowest disc stands for the set however the files were found
TEST_F(DiscSetServiceTest, DiscOrderOnDiskDoesNotDecideWhichEntrySurvives) {
  addTitledDisc("Chrono Cross", "disc2", 2);
  addTitledDisc("Chrono Cross", "disc3", 3);
  addTitledDisc("Chrono Cross", "disc1", 1);

  EXPECT_EQ(entryCount(), 1);
  EXPECT_TRUE(m_repo.getEntryWithContentHash("disc1").has_value());
  EXPECT_FALSE(m_repo.getEntryWithContentHash("disc2").has_value());
  EXPECT_FALSE(m_repo.getEntryWithContentHash("disc3").has_value());
}

// The tenet: a disc that was played before its set formed must not take its saves with it.
// The move happens elsewhere, so what is asserted here is that it is announced
TEST_F(DiscSetServiceTest, AbsorbingAnEntryAnnouncesWhereItsDataShouldGo) {
  addTitledDisc("Grandia", "disc2", 2);
  addTitledDisc("Grandia", "disc1", 1);

  ASSERT_EQ(m_absorbed.size(), 1u);
  EXPECT_EQ(m_absorbed.front().absorbedContentHash, "disc2");
  EXPECT_EQ(m_absorbed.front().survivingContentHash, "disc1");

  const auto survivor = m_repo.getEntryWithContentHash("disc1");
  ASSERT_TRUE(survivor.has_value());
  EXPECT_EQ(m_absorbed.front().survivingEntryId, survivor->id);
}

// A late disc 1 moves the identity once, and it never moves again
TEST_F(DiscSetServiceTest, ALateFirstDiscTakesOverTheSetItWalksInto) {
  addTitledDisc("Xenogears", "disc2", 2);
  addTitledDisc("Xenogears", "disc3", 3);

  const auto beforeSet = m_repo.getEntryWithContentHash("disc2")->discSetId;
  ASSERT_TRUE(beforeSet.has_value());

  addTitledDisc("Xenogears", "disc1", 1);

  const auto survivor = m_repo.getEntryWithContentHash("disc1");
  ASSERT_TRUE(survivor.has_value());
  // The same set, so its title and the playlist it will get do not start over
  EXPECT_EQ(survivor->discSetId, beforeSet);
  EXPECT_EQ(m_repo.getDiscsInSet(*beforeSet).size(), 3u);
}

// Two releases of one game fold to the same title, and which disc pairs with which is a
// guess. Guessing wrong deletes an entry, so nothing happens
TEST_F(DiscSetServiceTest, TwoCopiesOfOneDiscStopASetFromForming) {
  addTitledDisc("Parasite Eve", "usaDisc2", 2);
  addTitledDisc("Parasite Eve", "japanDisc2", 2);
  addTitledDisc("Parasite Eve", "usaDisc1", 1);

  EXPECT_EQ(entryCount(), 3);
  EXPECT_FALSE(m_repo.getEntryWithContentHash("usaDisc1")->discSetId.has_value());
}

// The copy that stops it may already be in a set, where it has no entry left to be found
// by title. Without looking there, whichever release was scanned first would decide
TEST_F(DiscSetServiceTest, ADiscAlreadyInASetStillCountsAsACopy) {
  addTitledDisc("Parasite Eve", "usaDisc1", 1);
  addTitledDisc("Parasite Eve", "usaDisc2", 2);
  ASSERT_EQ(entryCount(), 1);

  addTitledDisc("Parasite Eve", "japanDisc2", 2);

  EXPECT_EQ(entryCount(), 2);
  const auto japan = m_repo.getEntryWithContentHash("japanDisc2");
  ASSERT_TRUE(japan.has_value());
  EXPECT_FALSE(japan->discSetId.has_value());
}

// With the regions known there is no ambiguity to back off from: each release's discs
// find each other whatever order the files were scanned in
TEST_F(DiscSetServiceTest, TwoReleasesOfOneGameFormTwoSets) {
  addDisc("Suikoden II", "usaDisc1", 1);
  addDisc("Suikoden II", "japanDisc2", 2);
  addDisc("Suikoden II", "japanDisc1", 1);
  addDisc("Suikoden II", "usaDisc2", 2);

  setRegions("usaDisc1", {"US"});
  setRegions("usaDisc2", {"US"});
  setRegions("japanDisc1", {"JP"});
  setRegions("japanDisc2", {"JP"});

  for (const auto *hash : {"usaDisc1", "japanDisc2", "japanDisc1", "usaDisc2"}) {
    nameIt(hash, "Suikoden II");
  }

  EXPECT_EQ(entryCount(), 2);

  const auto usa = m_repo.getEntryWithContentHash("usaDisc1");
  const auto japan = m_repo.getEntryWithContentHash("japanDisc1");
  ASSERT_TRUE(usa.has_value());
  ASSERT_TRUE(japan.has_value());
  ASSERT_TRUE(usa->discSetId.has_value());
  ASSERT_TRUE(japan->discSetId.has_value());
  EXPECT_NE(*usa->discSetId, *japan->discSetId);
  EXPECT_EQ(m_repo.getDiscsInSet(*usa->discSetId).size(), 2u);
  EXPECT_EQ(m_repo.getDiscsInSet(*japan->discSetId).size(), 2u);
}

// A single-disc game is not a set, and neither is a game whose only peer is on another
// console
TEST_F(DiscSetServiceTest, OneDiscIsNotASet) {
  addTitledDisc("Silent Hill", "onlyDisc", 1);
  addTitledDisc("Silent Hill", "saturnDisc", 2, 8);

  EXPECT_EQ(entryCount(), 2);
  EXPECT_FALSE(m_repo.getEntryWithContentHash("onlyDisc")->discSetId.has_value());
  EXPECT_FALSE(m_repo.getEntryWithContentHash("saturnDisc")->discSetId.has_value());
}

// A cartridge has no disc number, so it is never pulled into a set by its title
TEST_F(DiscSetServiceTest, ContentWithNoDiscNumberIsLeftAlone) {
  addTitledDisc("Resident Evil 2", "disc1", 1);
  addDisc("Resident Evil 2", "cartridge", 0);
  nameIt("cartridge", "Resident Evil 2");

  EXPECT_EQ(entryCount(), 2);
  EXPECT_FALSE(m_repo.getEntryWithContentHash("cartridge")->discSetId.has_value());
}

// The escape hatch, and it has to survive the next scan
TEST_F(DiscSetServiceTest, DetachingADiscGivesItItsEntryBackForGood) {
  addTitledDisc("Lunar", "disc1", 1);
  const auto secondDisc = addTitledDisc("Lunar", "disc2", 2);
  ASSERT_EQ(entryCount(), 1);

  EXPECT_TRUE(m_discSets.detachDisc(secondDisc));
  EXPECT_EQ(entryCount(), 2);

  const auto detached = m_repo.getEntryWithContentHash("disc2");
  ASSERT_TRUE(detached.has_value());
  EXPECT_TRUE(detached->discSetUserSet);
  EXPECT_FALSE(detached->discSetId.has_value());
  EXPECT_EQ(m_repo.getRunConfigurations("disc2").size(), 1u);

  // One disc is not a set, so the set is gone with it
  const auto survivor = m_repo.getEntryWithContentHash("disc1");
  ASSERT_TRUE(survivor.has_value());
  EXPECT_FALSE(survivor->discSetId.has_value());

  // The disc keeps the game's name rather than falling back to its filename
  EXPECT_EQ(detached->displayName, "Lunar");

  // Metadata running again must not put back what the user took out
  nameIt("disc1", "Lunar");
  EXPECT_EQ(entryCount(), 2);
}

// Detaching the disc that stood for the set must not strand the others
TEST_F(DiscSetServiceTest, DetachingTheRepresentativeDiscLeavesEveryOtherDiscPlayable) {
  const auto firstDisc = addTitledDisc("Riven", "disc1", 1);
  addTitledDisc("Riven", "disc2", 2);
  ASSERT_EQ(entryCount(), 1);

  EXPECT_TRUE(m_discSets.detachDisc(firstDisc));

  EXPECT_EQ(entryCount(), 2);
  EXPECT_EQ(m_repo.getRunConfigurations("disc1").size(), 1u);
  EXPECT_EQ(m_repo.getRunConfigurations("disc2").size(), 1u);
}

// Handing a detached disc back to automatic grouping puts it where it belongs
TEST_F(DiscSetServiceTest, ClearingTheUserChoiceLetsTheSetFormAgain) {
  addTitledDisc("Koudelka", "disc1", 1);
  const auto secondDisc = addTitledDisc("Koudelka", "disc2", 2);
  ASSERT_TRUE(m_discSets.detachDisc(secondDisc));
  ASSERT_EQ(entryCount(), 2);

  const auto detached = m_repo.getEntryWithContentHash("disc2");
  ASSERT_TRUE(detached.has_value());
  EXPECT_TRUE(m_discSets.clearUserChoice(detached->id));

  EXPECT_EQ(entryCount(), 1);
  EXPECT_TRUE(m_repo.getEntryWithContentHash("disc1")->discSetId.has_value());
}

// The identity of a set is read from its playlist's first line, so a late disc 1 has to
// become that line
TEST_F(DiscSetServiceTest, ALateFirstDiscBecomesThePlaylistsFirstLine) {
  addTitledDisc("Xenogears", "disc2", 2);
  addTitledDisc("Xenogears", "disc3", 3);

  const auto beforeSet = m_repo.getEntryWithContentHash("disc2")->discSetId;
  ASSERT_TRUE(beforeSet.has_value());
  ASSERT_TRUE(std::filesystem::exists(playlistFor("disc2")));

  addTitledDisc("Xenogears", "disc1", 1);

  // The identity moved to disc 1, so the playlist is named after it and lists it first
  EXPECT_EQ(readFile(playlistFor("disc1")), romsPath() + "/Xenogears (Disc 1) [disc1].cue\n" + romsPath() +
                                                "/Xenogears (Disc 2) [disc2].cue\n" + romsPath() +
                                                "/Xenogears (Disc 3) [disc3].cue\n");
  EXPECT_FALSE(std::filesystem::exists(playlistFor("disc2")))
      << "the playlist for the identity the set no longer launches under was left behind";
}

// Somebody's own playlist is theirs. We write in one directory of our own and never anywhere
// else, so there is nothing to adopt and nothing to shadow
TEST_F(DiscSetServiceTest, NothingIsEverWrittenNearTheGames) {
  const auto ownPlaylist = romsPath() + "/Grandia.m3u";
  std::filesystem::create_directories(romsPath());
  {
    std::ofstream out(ownPlaylist, std::ios::binary);
    out << "Grandia (Disc 2) [disc2].cue\nGrandia (Disc 1) [disc1].cue\n";
  }

  addTitledDisc("Grandia", "disc1", 1);
  addTitledDisc("Grandia", "disc2", 2);

  // Their order, their bytes, untouched
  EXPECT_EQ(readFile(ownPlaylist), "Grandia (Disc 2) [disc2].cue\nGrandia (Disc 1) [disc1].cue\n");

  auto playlistsBesideTheGames = 0;
  for (const auto &entry : std::filesystem::directory_iterator(romsPath())) {
    if (entry.path().extension() == ".m3u") {
      ++playlistsBesideTheGames;
    }
  }

  EXPECT_EQ(playlistsBesideTheGames, 1) << "a playlist of ours was written into the games folder";
  EXPECT_TRUE(std::filesystem::exists(playlistFor("disc1")));
}

// A playlist naming a disc that has been taken out would launch into a missing file
TEST_F(DiscSetServiceTest, DetachingOneOfThreeRewritesThePlaylist) {
  addTitledDisc("Riven", "disc1", 1);
  addTitledDisc("Riven", "disc2", 2);
  const auto thirdDisc = addTitledDisc("Riven", "disc3", 3);

  const auto survivor = m_repo.getEntryWithContentHash("disc1");
  ASSERT_TRUE(survivor.has_value());
  ASSERT_TRUE(survivor->discSetId.has_value());
  const auto setId = *survivor->discSetId;

  ASSERT_TRUE(m_discSets.detachDisc(thirdDisc));

  EXPECT_TRUE(m_repo.getDiscSet(setId).has_value());
  EXPECT_EQ(readFile(playlistFor("disc1")),
            romsPath() + "/Riven (Disc 1) [disc1].cue\n" + romsPath() + "/Riven (Disc 2) [disc2].cue\n");
}

// A set that dissolves must not leave its entry pointed at a playlist for a set that is gone
TEST_F(DiscSetServiceTest, DissolvingASetRetiresItsPlaylist) {
  addTitledDisc("Koudelka", "disc1", 1);
  const auto secondDisc = addTitledDisc("Koudelka", "disc2", 2);

  const auto before = m_repo.getEntryWithContentHash("disc1");
  ASSERT_TRUE(before.has_value());
  ASSERT_TRUE(before->discSetId.has_value());
  ASSERT_TRUE(std::filesystem::exists(playlistFor("disc1")));

  ASSERT_TRUE(m_discSets.detachDisc(secondDisc));

  EXPECT_FALSE(std::filesystem::exists(playlistFor("disc1")));

  // Both discs launch on their own again, and neither through the playlist
  for (const auto *hash : {"disc1", "disc2"}) {
    const auto configs = configsFor(hash);
    ASSERT_EQ(configs.size(), 1u) << hash;
    EXPECT_EQ(configs.front().type, RunConfiguration::TYPE_ROM) << hash;
  }
}

// The file is a render of what the database already knows, so losing it costs a write and
// nothing else. The game never leaves the library
TEST_F(DiscSetServiceTest, AMissingArtifactIsRegeneratedAndTheGameNeverHides) {
  addTitledDisc("Final Fantasy VII", "disc1", 1);
  addTitledDisc("Final Fantasy VII", "disc2", 2);

  const auto survivor = m_repo.getEntryWithContentHash("disc1");
  ASSERT_TRUE(survivor.has_value());
  ASSERT_TRUE(survivor->discSetId.has_value());
  ASSERT_TRUE(std::filesystem::exists(playlistFor("disc1")));

  std::filesystem::remove(playlistFor("disc1"));

  const auto whileMissing = m_repo.getEntryWithContentHash("disc1");
  ASSERT_TRUE(whileMissing.has_value());
  EXPECT_FALSE(whileMissing->hidden) << "losing a file we generated took the game with it";

  EXPECT_TRUE(m_discSets.materializePlaylist(*survivor->discSetId, "disc1"));
  EXPECT_TRUE(std::filesystem::exists(playlistFor("disc1")));
}

// Rewriting costs a write and a file change somebody's tooling may be watching for
TEST_F(DiscSetServiceTest, MaterializingAnUnchangedPlaylistWritesNothing) {
  addTitledDisc("Grandia", "disc1", 1);
  addTitledDisc("Grandia", "disc2", 2);

  const auto survivor = m_repo.getEntryWithContentHash("disc1");
  ASSERT_TRUE(survivor.has_value());
  const auto before = std::filesystem::last_write_time(playlistFor("disc1"));

  EXPECT_FALSE(m_discSets.materializePlaylist(*survivor->discSetId, "disc1"));
  EXPECT_EQ(std::filesystem::last_write_time(playlistFor("disc1")), before) << "an unchanged playlist was rewritten";
}

// A disc going away leaves the playlist naming a file that is not there any more
TEST_F(DiscSetServiceTest, MaterializingBringsAStalePlaylistUpToDate) {
  addTitledDisc("Lunar", "disc1", 1);
  addTitledDisc("Lunar", "disc2", 2);
  const auto thirdDisc = addTitledDisc("Lunar", "disc3", 3);

  const auto survivor = m_repo.getEntryWithContentHash("disc1");
  ASSERT_TRUE(survivor.has_value());
  ASSERT_NE(readFile(playlistFor("disc1")).find("disc3"), std::string::npos);

  // Disc 3 goes away, the way a scan removes a file that is gone
  ASSERT_TRUE(m_repo.deleteContentFile(thirdDisc));
  EXPECT_TRUE(m_discSets.materializePlaylist(*survivor->discSetId, "disc1"));

  EXPECT_EQ(readFile(playlistFor("disc1")).find("disc3"), std::string::npos)
      << "the playlist still names a disc that is gone";
}

// The survivor was found by walking a set's discs back to their entries, but an absorbed disc
// has no entry — so losing the survivor's own disc left the set with nobody to rebuild for
TEST_F(DiscSetServiceTest, LosingTheSurvivorsOwnDiscStillRewritesThePlaylist) {
  const auto firstDisc = addTitledDisc("Panzer Dragoon Saga", "disc1", 1);
  addTitledDisc("Panzer Dragoon Saga", "disc2", 2);
  addTitledDisc("Panzer Dragoon Saga", "disc3", 3);

  const auto survivor = m_repo.getEntryWithContentHash("disc1");
  ASSERT_TRUE(survivor.has_value());
  ASSERT_TRUE(survivor->discSetId.has_value());
  const auto setId = *survivor->discSetId;

  // Disc 1 goes away the way a scan removes a file that is gone
  ASSERT_TRUE(m_repo.deleteContentFile(firstDisc));

  const auto remaining = survivorOfSet(setId);
  ASSERT_FALSE(remaining.empty()) << "the set was left with nobody to rebuild for";
  EXPECT_TRUE(m_discSets.materializePlaylist(setId, remaining));

  EXPECT_TRUE(std::filesystem::exists(playlistFor(remaining)));
  EXPECT_EQ(readFile(playlistFor(remaining)).find("disc1"), std::string::npos)
      << "the playlist still names the disc that is gone";
}

// Nothing supplies the count yet, so a set records 0 and the verdict says nothing about discs
TEST_F(DiscSetServiceTest, ASetRecordsTheDiscCountItWasToldAndOtherwiseNothing) {
  addTitledDisc("Shenmue", "disc1", 1);
  addTitledDisc("Shenmue", "disc2", 2);

  const auto survivor = m_repo.getEntryWithContentHash("disc1");
  ASSERT_TRUE(survivor.has_value());
  ASSERT_TRUE(survivor->discSetId.has_value());
  EXPECT_EQ(m_repo.getDiscSet(*survivor->discSetId)->discCount, 0);

  // What metadata population would supply once the content database carries it
  GameMetadata metadata;
  metadata.discCount = 4;
  ASSERT_TRUE(m_repo.applyEntryMetadata(survivor->id, metadata, {metadata_fields::DISC_COUNT}, false));

  EXPECT_EQ(m_repo.getDiscSet(*survivor->discSetId)->discCount, 4);
}

// A playlist left behind by an earlier run is catalogued before the disc folders and, because
// it hashes to disc 1, it is what creates that disc's entry — named after itself, with no
// region tag on it. Every other disc has one, so the set split in two
TEST_F(DiscSetServiceTest, AStalePlaylistDoesNotSplitTheSetInTwo) {
  // The leftover playlist, carrying disc 1's identity and its own bare name
  ContentFile playlist;
  playlist.m_type = ContentType::Disc;
  playlist.m_filePath = romsPath() + "/Final Fantasy VII.m3u";
  playlist.m_platformId = 7;
  playlist.m_contentHash = "disc1";
  playlist.m_fileSizeBytes = 200;
  ASSERT_TRUE(m_repo.create(playlist));
  nameIt("disc1", "Final Fantasy VII");

  addDisc("Final Fantasy VII", "disc1", 1);
  addDisc("Final Fantasy VII", "disc2", 2);
  addDisc("Final Fantasy VII", "disc3", 3);

  // The discs carry their region; the playlist's own name never did, so disc 1's entry has none
  setRegions("disc2", {"US"});
  setRegions("disc3", {"US"});

  for (const auto *hash : {"disc1", "disc2", "disc3"}) {
    nameIt(hash, "Final Fantasy VII");
  }

  EXPECT_EQ(entryCount(), 1) << "the game is in the library more than once";
}

// The same shape without the region ever arriving: an entry that knows nothing about its
// region must not read as a different release from one that does
TEST_F(DiscSetServiceTest, AnUnknownRegionGroupsWithAKnownOne) {
  addTitledDisc("Grandia", "disc1", 1);
  addDisc("Grandia", "disc2", 2);
  setRegions("disc2", {"US"});
  nameIt("disc2", "Grandia");

  EXPECT_EQ(entryCount(), 1) << "not knowing a region was taken as knowing it is different";
}

// A set owns its way in, so dissolving the set has to take it along. One left behind keeps
// getRunConfigurations answering, which is what decides whether a game can ever be hidden
TEST_F(DiscSetServiceTest, ADeletedSetTakesItsWayInWithIt) {
  addTitledDisc("Final Fantasy VII", "disc1", 1);
  addTitledDisc("Final Fantasy VII", "disc2", 2);

  const auto survivor = m_repo.getEntryWithContentHash("disc1");
  ASSERT_TRUE(survivor.has_value());
  ASSERT_TRUE(survivor->discSetId.has_value());

  const auto setId = *survivor->discSetId;
  ASSERT_TRUE(std::ranges::any_of(configsFor("disc1"), [setId](const RunConfiguration &configuration) {
    return configuration.discSetId == setId;
  })) << "the set never had a way in of its own";

  ASSERT_TRUE(m_repo.deleteDiscSet(setId));

  EXPECT_FALSE(std::ranges::any_of(configsFor("disc1"), [setId](const RunConfiguration &configuration) {
    return configuration.discSetId == setId;
  })) << "the set is gone but its way in is still there";
}

// The last file going is what hides a game. A way in anchored on a disc that no longer exists
// would leave a game nobody can launch sitting in the library forever
TEST_F(DiscSetServiceTest, AGameWhoseFilesAreAllGoneLeavesTheLibrary) {
  const auto discOne = addTitledDisc("Final Fantasy VII", "disc1", 1);
  const auto discTwo = addTitledDisc("Final Fantasy VII", "disc2", 2);

  const auto survivor = m_repo.getEntryWithContentHash("disc1");
  ASSERT_TRUE(survivor.has_value());
  ASSERT_TRUE(survivor->discSetId.has_value());

  for (const auto &file : m_repo.getContentFiles()) {
    ASSERT_TRUE(m_repo.deleteContentFile(file.m_id));
  }

  EXPECT_TRUE(configsFor("disc1").empty()) << "a way in outlived every file it could launch";
  EXPECT_TRUE(configsFor("disc2").empty());

  const auto after = m_repo.getEntryWithContentHash("disc1");
  ASSERT_TRUE(after.has_value());
  EXPECT_FALSE(after->isContentAvailable) << "the game is unlaunchable but still on the shelf";

  EXPECT_GT(discOne, 0);
  EXPECT_GT(discTwo, 0);
}

// The whole point of the content database: discs whose filenames agree about nothing still
// fold together, because the identity does not come from the filename
TEST_F(DiscSetServiceTest, TheDatabaseGroupsDiscsWhoseNamesDisagree) {
  addIdentifiedDisc("Biohazard", "disc1", 1, 25390);
  addIdentifiedDisc("Resident Evil", "disc2", 2, 25390);

  EXPECT_EQ(entryCount(), 1) << "a resolved game id did not bring two differently named discs together";

  const auto survivor = m_repo.getEntryWithContentHash("disc1");
  ASSERT_TRUE(survivor.has_value());
  ASSERT_TRUE(survivor->discSetId.has_value());
  EXPECT_EQ(m_repo.getDiscsInSet(*survivor->discSetId).size(), 2u);
}

// One game id spans a game's regional releases, so once it is resolved the region check is the
// only thing left keeping two releases apart -- and absorbing deletes an entry
TEST_F(DiscSetServiceTest, OneGameIdStillDoesNotMergeTwoReleases) {
  addIdentifiedDisc("Biohazard", "japanDisc1", 1, 25390, {"JP"});
  addIdentifiedDisc("Resident Evil", "usaDisc2", 2, 25390, {"US"});

  EXPECT_EQ(entryCount(), 2) << "discs of two releases were folded into one set";
}

// Coverage arrives one dump at a time, so a database that knows one disc and not its sibling
// must not split a set that the titles alone would have formed
TEST_F(DiscSetServiceTest, PartialDatabaseCoverageStillGroups) {
  addIdentifiedDisc("Grandia", "disc1", 1, 4242);
  addTitledDisc("Grandia", "disc2", 2);

  EXPECT_EQ(entryCount(), 1) << "knowing one disc and not the other split the set";
}

} // namespace firelight::library

#include <firelight/event_dispatcher.hpp>
#include <firelight/library/disc_set_playlist.hpp>
#include <firelight/library/disc_set_service.hpp>
#include <firelight/library/filename_tags.hpp>
#include <firelight/library/library_events.hpp>
#include <firelight/library/library_ingest_service.hpp>
#include <firelight/library/sqlite_user_library.hpp>

#include <QTemporaryDir>
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
  int addDisc(const std::string &title, const std::string &hash, const int discNumber, const int platformId = 7) {
    ContentFile file;
    file.m_type = ContentType::Disc;
    // Two releases put their disc 2 at the same name and size, so the hash is what keeps
    // the names apart
    file.m_filePath = romsPath() + "/" + title + " (Disc " + std::to_string(discNumber) + ") [" + hash + "].cue";
    file.m_platformId = platformId;
    file.m_contentHash = hash;
    file.m_discNumber = discNumber;
    file.m_fileSizeBytes = 1000 + discNumber;
    EXPECT_TRUE(m_repo.create(file));
    return file.m_id;
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

  static std::string readFile(const std::string &path) {
    std::ifstream in(path, std::ios::binary);
    return std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
  }
};

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

  // Exactly one way in, and it is the playlist, so the core is handed one path for the whole
  // game and writes one memory card
  const auto configs = configsFor("disc1");
  ASSERT_EQ(configs.size(), 1u);
  EXPECT_EQ(configs.front().type, RunConfiguration::TYPE_PLAYLIST);
  EXPECT_TRUE(configsFor("disc2").empty());

  const auto set = m_repo.getDiscSet(*survivor->discSetId);
  ASSERT_TRUE(set.has_value());
  EXPECT_TRUE(set->playlistOwned);
  EXPECT_TRUE(std::filesystem::exists(set->playlistPath));
  EXPECT_EQ(readFile(set->playlistPath),
            std::string(PLAYLIST_MARKER) +
                "\n"
                "Final Fantasy VII (Disc 1) [disc1].cue\nFinal Fantasy VII (Disc 2) [disc2].cue\n");
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
  const auto before = m_repo.getDiscSet(*beforeSet);
  ASSERT_TRUE(before.has_value());
  EXPECT_EQ(readFile(before->playlistPath), std::string(PLAYLIST_MARKER) +
                                                "\n"
                                                "Xenogears (Disc 2) [disc2].cue\nXenogears (Disc 3) [disc3].cue\n");

  addTitledDisc("Xenogears", "disc1", 1);

  const auto after = m_repo.getDiscSet(*beforeSet);
  ASSERT_TRUE(after.has_value());
  EXPECT_EQ(readFile(after->playlistPath),
            std::string(PLAYLIST_MARKER) +
                "\n"
                "Xenogears (Disc 1) [disc1].cue\nXenogears (Disc 2) [disc2].cue\nXenogears (Disc 3) [disc3].cue\n");
}

// A playlist somebody wrote is the one they take to other frontends
TEST_F(DiscSetServiceTest, AUserWrittenPlaylistIsAdoptedNotOverwritten) {
  const auto ownPlaylist = romsPath() + "/Grandia.m3u";
  std::filesystem::create_directories(romsPath());
  {
    std::ofstream out(ownPlaylist, std::ios::binary);
    out << "Grandia (Disc 2) [disc2].cue\nGrandia (Disc 1) [disc1].cue\n";
  }

  addTitledDisc("Grandia", "disc1", 1);
  addTitledDisc("Grandia", "disc2", 2);

  const auto survivor = m_repo.getEntryWithContentHash("disc1");
  ASSERT_TRUE(survivor.has_value());
  ASSERT_TRUE(survivor->discSetId.has_value());

  const auto set = m_repo.getDiscSet(*survivor->discSetId);
  ASSERT_TRUE(set.has_value());
  EXPECT_EQ(set->playlistPath, ownPlaylist);
  EXPECT_FALSE(set->playlistOwned);
  // Their order, untouched, even though it is not the one we would have written
  EXPECT_EQ(readFile(ownPlaylist), "Grandia (Disc 2) [disc2].cue\nGrandia (Disc 1) [disc1].cue\n");
}

// Nothing is written beside somebody's games when they asked for that
TEST_F(DiscSetServiceTest, AppDataPlaylistsLeaveTheGamesFolderAlone) {
  m_discSets.setPlaylistLocation(PlaylistLocation::AppData);

  addTitledDisc("Lunar", "disc1", 1);
  addTitledDisc("Lunar", "disc2", 2);

  const auto survivor = m_repo.getEntryWithContentHash("disc1");
  ASSERT_TRUE(survivor.has_value());
  ASSERT_TRUE(survivor->discSetId.has_value());

  const auto set = m_repo.getDiscSet(*survivor->discSetId);
  ASSERT_TRUE(set.has_value());
  EXPECT_TRUE(set->playlistPath.starts_with((m_root.path() + "/appdata/playlists/").toStdString()));
  EXPECT_FALSE(std::filesystem::exists(romsPath() + "/Lunar.m3u"));

  // The scanner never walks app data, so a set whose playlist lives there still has a way in
  const auto configs = configsFor("disc1");
  ASSERT_EQ(configs.size(), 1u);
  EXPECT_EQ(configs.front().type, RunConfiguration::TYPE_PLAYLIST);
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

  const auto set = m_repo.getDiscSet(setId);
  ASSERT_TRUE(set.has_value());
  EXPECT_EQ(readFile(set->playlistPath), std::string(PLAYLIST_MARKER) +
                                             "\n"
                                             "Riven (Disc 1) [disc1].cue\nRiven (Disc 2) [disc2].cue\n");
}

// A set that dissolves must not leave its entry pointed at a playlist for a set that is gone
TEST_F(DiscSetServiceTest, DissolvingASetRetiresItsPlaylist) {
  addTitledDisc("Koudelka", "disc1", 1);
  const auto secondDisc = addTitledDisc("Koudelka", "disc2", 2);

  const auto before = m_repo.getEntryWithContentHash("disc1");
  ASSERT_TRUE(before.has_value());
  ASSERT_TRUE(before->discSetId.has_value());
  const auto playlistPath = m_repo.getDiscSet(*before->discSetId)->playlistPath;
  ASSERT_TRUE(std::filesystem::exists(playlistPath));

  ASSERT_TRUE(m_discSets.detachDisc(secondDisc));

  EXPECT_FALSE(std::filesystem::exists(playlistPath));

  // Both discs launch on their own again, and neither through the playlist
  for (const auto *hash : {"disc1", "disc2"}) {
    const auto configs = configsFor(hash);
    ASSERT_EQ(configs.size(), 1u) << hash;
    EXPECT_EQ(configs.front().type, RunConfiguration::TYPE_ROM) << hash;
  }
}

// The set's only way in is a file we generate, so deleting it hid the game while all three
// discs were still sitting on disk. Nothing regenerated it, and nothing noticed
TEST_F(DiscSetServiceTest, DeletingThePlaylistBringsTheGameBackOnReconcile) {
  addTitledDisc("Final Fantasy VII", "disc1", 1);
  addTitledDisc("Final Fantasy VII", "disc2", 2);

  const auto survivor = m_repo.getEntryWithContentHash("disc1");
  ASSERT_TRUE(survivor.has_value());
  ASSERT_TRUE(survivor->discSetId.has_value());
  const auto playlistPath = m_repo.getDiscSet(*survivor->discSetId)->playlistPath;
  ASSERT_TRUE(std::filesystem::exists(playlistPath));

  // What a scan does when the file it catalogued has gone
  std::filesystem::remove(playlistPath);
  const auto playlistFile = m_repo.getContentFileWithPath(playlistPath);
  ASSERT_TRUE(playlistFile.has_value());
  ASSERT_TRUE(m_repo.deleteContentFile(playlistFile->m_id));

  const auto refreshed = m_repo.getEntryWithContentHash("disc1");
  ASSERT_TRUE(refreshed.has_value()) << "the entry was deleted rather than hidden";

  m_discSets.reconcilePlaylists();

  EXPECT_TRUE(std::filesystem::exists(playlistPath)) << "the playlist was not regenerated";

  const auto restored = m_repo.getEntryWithContentHash("disc1");
  ASSERT_TRUE(restored.has_value());
  EXPECT_FALSE(restored->hidden) << "the game is still hidden even though its playlist is back";
  EXPECT_EQ(restored->id, survivor->id);

  const auto configs = configsFor("disc1");
  ASSERT_EQ(configs.size(), 1u);
  EXPECT_EQ(configs.front().type, RunConfiguration::TYPE_PLAYLIST);
}

// A reconcile runs on every scan, and rewriting a file inside a watched folder starts another
// scan, so an unchanged playlist must not be touched at all
TEST_F(DiscSetServiceTest, ReconcilingAnUnchangedSetRewritesNothing) {
  addTitledDisc("Grandia", "disc1", 1);
  addTitledDisc("Grandia", "disc2", 2);

  const auto survivor = m_repo.getEntryWithContentHash("disc1");
  ASSERT_TRUE(survivor.has_value());
  const auto playlistPath = m_repo.getDiscSet(*survivor->discSetId)->playlistPath;
  const auto before = std::filesystem::last_write_time(playlistPath);

  m_discSets.reconcilePlaylists();
  m_discSets.reconcilePlaylists();

  EXPECT_EQ(std::filesystem::last_write_time(playlistPath), before) << "an unchanged playlist was rewritten";
  EXPECT_EQ(entryCount(), 1);
}

// A disc going away leaves the playlist naming a file that is not there any more
TEST_F(DiscSetServiceTest, ReconcileBringsAStalePlaylistUpToDate) {
  addTitledDisc("Lunar", "disc1", 1);
  addTitledDisc("Lunar", "disc2", 2);
  const auto thirdDisc = addTitledDisc("Lunar", "disc3", 3);

  const auto survivor = m_repo.getEntryWithContentHash("disc1");
  ASSERT_TRUE(survivor.has_value());
  const auto playlistPath = m_repo.getDiscSet(*survivor->discSetId)->playlistPath;
  ASSERT_NE(readFile(playlistPath).find("disc3"), std::string::npos);

  // Disc 3 goes away, the way a scan removes a file that is gone
  ASSERT_TRUE(m_repo.deleteContentFile(thirdDisc));
  m_discSets.reconcilePlaylists();

  EXPECT_EQ(readFile(playlistPath).find("disc3"), std::string::npos) << "the playlist still names a disc that is gone";
}

} // namespace firelight::library

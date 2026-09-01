// TODO: NEEDS REVIEW
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
#include <sstream>

// Exercises the shape change end to end: content files arrive, ingest turns them into
// entries, and metadata giving those entries a title is what folds the discs of one game
// together. The trigger is the real one, so what is asserted is what a scan produces
namespace firelight::library {

class DiscSetServiceTest : public testing::Test {
protected:
  // The service writes real playlists, so the fixture gives it somewhere of its own to do it
  QTemporaryDir m_root;
  SqliteUserLibraryRepository m_repo{":memory:"};
  DiscSetService m_discSets{m_repo, (m_root.path() + "/appdata").toStdString()};
  LibraryIngestService m_ingest{m_repo, m_discSets};

  std::string romsPath() const { return (m_root.path() + "/roms").toStdString(); }

  std::vector<EntryAbsorbedEvent> m_absorbed;
  ScopedConnection m_absorbedConnection = EventDispatcher::instance().subscribe<EntryAbsorbedEvent>(
      [this](const EntryAbsorbedEvent &event) { m_absorbed.push_back(event); });

  // TODO
  // What a set moving onto a lower disc announces, so saves and playtime can follow it
  std::vector<EntryIdentityChangedEvent> m_identityChanges;
  ScopedConnection m_identityChangedConnection = EventDispatcher::instance().subscribe<EntryIdentityChangedEvent>(
      [this](const EntryIdentityChangedEvent &event) { m_identityChanges.push_back(event); });

  // Catalogues one disc, which is what gives it an entry of its own
  int addDisc(const std::string &title, const std::string &hash, const int discNumber, const int platformId = 7,
              const std::vector<std::string> &regions = {}) {
    ContentFile file;
    file.m_type = ContentType::Disc;
    // Two releases put their disc 2 at the same name and size, so the hash is what keeps
    // the names apart
    file.m_filePath = romsPath() + "/" + title + " (Disc " + std::to_string(discNumber) + ") [" + hash + "].cue";
    file.m_platformId = platformId;
    file.m_contentHash = hash;
    file.m_discNumber = discNumber;
    file.m_normalizedTitle = normalizeTitle(title);
    file.m_regions = regions;
    file.m_fileSizeBytes = 1000 + discNumber;
    EXPECT_TRUE(m_repo.create(file));
    return file.m_id;
  }

  // TODO
  // A disc the content database knows, catalogued and titled the way a scan does it. The region
  // goes onto the file as well as the entry because placement runs at ingest, before anything has
  // had a chance to name the entry
  void addIdentifiedDisc(const std::string &title, const std::string &hash, const int discNumber,
                         const std::vector<std::string> &regions = {}) {
    addDisc(title, hash, discNumber, 7, regions);

    if (!regions.empty()) {
      setRegions(hash, regions);
    }

    nameIt(hash, title);
  }

  // TODO
  // Stands in for metadata population. A disc reached through its set has no entry of its own, so
  // there is nothing to name
  void nameIt(const std::string &hash, const std::string &title) {
    auto entry = m_repo.getEntryWithContentHash(hash);

    if (!entry.has_value()) {
      return;
    }

    entry->displayName = title;
    entry->normalizedTitle = normalizeTitle(title);
    ASSERT_TRUE(m_repo.updateEntryMetadata(*entry));
  }

  // Stands in for the region a scrape or the filename tags resolved
  void setRegions(const std::string &hash, const std::vector<std::string> &regions) {
    const auto entry = m_repo.getEntryWithContentHash(hash);

    if (!entry.has_value()) {
      return;
    }

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

  int entryCount() { return static_cast<int>(m_repo.getEntries().size()); }

  std::vector<RunConfiguration> configsFor(const std::string &hash) { return m_repo.getRunConfigurations(hash); }

  // TODO
  // The identity a set launches under, which is the hash of the entry left standing over it. The
  // discs converge on one entry, so there is nothing to choose between
  std::string survivorOfSet(const int setId) {
    const auto entries = m_repo.getEntriesInDiscSet(setId);
    return entries.empty() ? "" : entries.front().contentHash;
  }

  // Where the set launching under this identity keeps its playlist
  std::string playlistFor(const std::string &hash) const {
    return playlistPathFor(hash, (m_root.path() + "/appdata").toStdString());
  }

  static std::string readFile(const std::string &path) {
    std::ifstream in(path, std::ios::binary);
    return std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
  }

  // TODO
  // What a playlist names, without the line saying the file is ours
  static std::string discLinesOf(const std::string &path) {
    std::istringstream contents(readFile(path));
    std::string discLines;
    std::string line;

    while (std::getline(contents, line)) {
      if (!line.starts_with("#")) {
        discLines += line + "\n";
      }
    }

    return discLines;
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
    const auto member = m_repo.getDiscSetMemberForContentFile(disc.m_id);
    ASSERT_TRUE(member.has_value()) << disc.m_filePath << " lost its set";
    EXPECT_EQ(member->m_discSetId, setId);
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
  const auto playlist = discLinesOf(playlistFor(survivorOfSet(setId)));
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
  const auto second = addTitledDisc("Final Fantasy VII", "disc2", 2);

  EXPECT_EQ(entryCount(), 1);

  const auto survivor = m_repo.getEntryWithContentHash("disc1");
  ASSERT_TRUE(survivor.has_value());
  ASSERT_TRUE(survivor->discSetId.has_value());

  // TODO
  // Which rung placed a disc lives on the membership row, so a filename guess is readable as one
  const auto placed = m_repo.getDiscSetMemberForContentFile(second);
  ASSERT_TRUE(placed.has_value());
  EXPECT_EQ(placed->m_source, DiscSource::Filename);

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
    return configuration.discSetId.has_value();
  })) << "a disc kept a way in of its own";
  EXPECT_TRUE(std::ranges::any_of(configs, [&](const RunConfiguration &configuration) {
    return configuration.discSetId == survivor->discSetId;
  })) << "the set has no way in of its own";
  EXPECT_TRUE(configsFor("disc2").empty());

  EXPECT_TRUE(std::filesystem::exists(playlistFor("disc1")));
  EXPECT_TRUE(isGeneratedPlaylist(readFile(playlistFor("disc1")))) << "the playlist does not say it is ours";
  EXPECT_EQ(discLinesOf(playlistFor("disc1")), romsPath() + "/Final Fantasy VII (Disc 1) [disc1].cue\n" + romsPath() +
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
// TODO
// Nothing is absorbed when a lower disc arrives: the entry that was standing for the set moves
// onto it, and says so, so what is keyed on the old dump can follow
TEST_F(DiscSetServiceTest, TakingOverFromALowerDiscAnnouncesWhereTheDataShouldGo) {
  addTitledDisc("Grandia", "disc2", 2);

  const auto before = m_repo.getEntryWithContentHash("disc2");
  ASSERT_TRUE(before.has_value());
  const auto entryId = before->id;

  addTitledDisc("Grandia", "disc1", 1);

  ASSERT_EQ(m_identityChanges.size(), 1u) << "the identity moved without saying so";
  EXPECT_EQ(m_identityChanges.front().entryId, entryId);
  EXPECT_EQ(m_identityChanges.front().previousContentHash, "disc2");
  EXPECT_EQ(m_identityChanges.front().contentHash, "disc1");

  EXPECT_TRUE(m_absorbed.empty()) << "nothing should have been absorbed";
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
// TODO
// A set holds each disc number once, so a second copy of disc 2 cannot join the set holding the
// first and gets one of its own. Disc 1 then matches both, which it records rather than hides
TEST_F(DiscSetServiceTest, TwoCopiesOfOneDiscCannotShareASet) {
  const auto usaTwo = addTitledDisc("Parasite Eve", "usaDisc2", 2);
  const auto japanTwo = addTitledDisc("Parasite Eve", "japanDisc2", 2);
  addTitledDisc("Parasite Eve", "usaDisc1", 1);

  // The entry that stood for the first set has re-keyed onto disc 1, so the sets are found
  // through the discs rather than through an entry
  const auto first = m_repo.getDiscSetMemberForContentFile(usaTwo);
  const auto second = m_repo.getDiscSetMemberForContentFile(japanTwo);
  ASSERT_TRUE(first.has_value());
  ASSERT_TRUE(second.has_value());
  EXPECT_NE(first->m_discSetId, second->m_discSetId) << "two copies of disc 2 shared one set";

  // Disc 1 could have belonged to either, so where it landed is marked as a guess
  const auto members = m_repo.getDiscSetMembers(first->m_discSetId);
  const auto discOne = std::ranges::find_if(members, [](const DiscSetMember &m) { return m.m_discNumber == 1; });
  ASSERT_NE(discOne, members.end());
  EXPECT_TRUE(discOne->m_isUncertain) << "an ambiguous placement was recorded as certain";
}

// The copy that stops it may already be in a set, where it has no entry left to be found
// by title. Without looking there, whichever release was scanned first would decide
// TODO
// The number already being taken is what keeps a second copy out, so a set that has formed is no
// more able to take one than a set still forming
TEST_F(DiscSetServiceTest, ADiscAlreadyInASetStillHoldsItsNumber) {
  addTitledDisc("Parasite Eve", "usaDisc1", 1);
  addTitledDisc("Parasite Eve", "usaDisc2", 2);
  ASSERT_EQ(entryCount(), 1);

  addTitledDisc("Parasite Eve", "japanDisc2", 2);

  EXPECT_EQ(entryCount(), 2);

  const auto japan = m_repo.getEntryWithContentHash("japanDisc2");
  ASSERT_TRUE(japan.has_value());
  ASSERT_TRUE(japan->discSetId.has_value()) << "the copy should have a set of its own";

  const auto usa = m_repo.getEntryWithContentHash("usaDisc1");
  ASSERT_TRUE(usa.has_value());
  EXPECT_NE(*japan->discSetId, *usa->discSetId);
}

// With the regions known there is no ambiguity to back off from: each release's discs
// find each other whatever order the files were scanned in
TEST_F(DiscSetServiceTest, TwoReleasesOfOneGameFormTwoSets) {
  addDisc("Suikoden II", "usaDisc1", 1, 7, {"US"});
  addDisc("Suikoden II", "japanDisc2", 2, 7, {"JP"});
  addDisc("Suikoden II", "japanDisc1", 1, 7, {"JP"});
  addDisc("Suikoden II", "usaDisc2", 2, 7, {"US"});

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
// TODO
// A lone disc is a set of one, the same shape as any other set, so nothing branches on how many
// discs a game came on. These two are different platforms and so different games
TEST_F(DiscSetServiceTest, OneDiscIsASetOfOne) {
  addTitledDisc("Silent Hill", "onlyDisc", 1);
  addTitledDisc("Silent Hill", "saturnDisc", 2, 8);

  EXPECT_EQ(entryCount(), 2);

  for (const auto *hash : {"onlyDisc", "saturnDisc"}) {
    const auto entry = m_repo.getEntryWithContentHash(hash);
    ASSERT_TRUE(entry.has_value());
    ASSERT_TRUE(entry->discSetId.has_value()) << hash << " did not get a set of its own";
    EXPECT_EQ(m_repo.getDiscSetMembers(*entry->discSetId).size(), 1u);
  }
}

// A cartridge has no disc number, so it is never pulled into a set by its title
// TODO
// A cartridge launches through its own file, so it never joins a set however much its name looks
// like a disc of one
TEST_F(DiscSetServiceTest, ACartridgeIsLeftAlone) {
  addTitledDisc("Resident Evil 2", "disc1", 1);

  ContentFile cartridge;
  cartridge.m_type = ContentType::Cartridge;
  cartridge.m_filePath = romsPath() + "/Resident Evil 2.md";
  cartridge.m_platformId = 7;
  cartridge.m_contentHash = "cartridge";
  cartridge.m_normalizedTitle = normalizeTitle("Resident Evil 2");
  cartridge.m_fileSizeBytes = 2048;
  ASSERT_TRUE(m_repo.create(cartridge));
  nameIt("cartridge", "Resident Evil 2");

  EXPECT_EQ(entryCount(), 2);
  EXPECT_FALSE(m_repo.getEntryWithContentHash("cartridge")->discSetId.has_value());
}

// TODO
// A set is identified by its anchor's hash, so a disc nothing could hash cannot be in one and
// launches as itself
TEST_F(DiscSetServiceTest, ADiscNothingCouldHashLaunchesAsItself) {
  ContentFile unhashed;
  unhashed.m_type = ContentType::Disc;
  unhashed.m_filePath = romsPath() + "/Mystery.bin";
  unhashed.m_platformId = 7;
  unhashed.m_normalizedTitle = normalizeTitle("Mystery");
  unhashed.m_fileSizeBytes = 4096;
  ASSERT_TRUE(m_repo.create(unhashed));

  const auto entry = m_repo.getEntryWithContentHash("");
  ASSERT_TRUE(entry.has_value()) << "content nothing could hash never reached the library";
  EXPECT_FALSE(entry->discSetId.has_value());
  EXPECT_EQ(configsFor("").size(), 1u) << "it was left with no way in";
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
  EXPECT_EQ(m_repo.getRunConfigurations("disc2").size(), 1u);

  // TODO
  // A disc on its own is a set of one, and the row records that a person put it there
  ASSERT_TRUE(detached->discSetId.has_value());
  const auto members = m_repo.getDiscSetMembers(*detached->discSetId);
  ASSERT_EQ(members.size(), 1u);
  EXPECT_EQ(members.front().m_source, DiscSource::User);

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
  EXPECT_EQ(discLinesOf(playlistFor("disc1")), romsPath() + "/Xenogears (Disc 1) [disc1].cue\n" + romsPath() +
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
  EXPECT_EQ(discLinesOf(playlistFor("disc1")),
            romsPath() + "/Riven (Disc 1) [disc1].cue\n" + romsPath() + "/Riven (Disc 2) [disc2].cue\n");
}

// A set that dissolves must not leave its entry pointed at a playlist for a set that is gone
// TODO
// A set of one is a set like any other, so detaching leaves two of them rather than dissolving
// anything. Each keeps its own way in
TEST_F(DiscSetServiceTest, DetachingLeavesTwoSetsEachWithItsOwnWayIn) {
  addTitledDisc("Koudelka", "disc1", 1);
  const auto secondDisc = addTitledDisc("Koudelka", "disc2", 2);

  const auto before = m_repo.getEntryWithContentHash("disc1");
  ASSERT_TRUE(before.has_value());
  ASSERT_TRUE(before->discSetId.has_value());
  ASSERT_TRUE(std::filesystem::exists(playlistFor("disc1")));

  ASSERT_TRUE(m_discSets.detachDisc(secondDisc));

  // The set the disc left is still identified by disc 1, so its playlist stands
  EXPECT_TRUE(std::filesystem::exists(playlistFor("disc1")));

  for (const auto *hash : {"disc1", "disc2"}) {
    const auto configs = configsFor(hash);
    ASSERT_EQ(configs.size(), 1u) << hash;
    EXPECT_TRUE(configs.front().discSetId.has_value()) << hash;
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
  ASSERT_NE(discLinesOf(playlistFor("disc1")).find("disc3"), std::string::npos);

  // Disc 3 goes away, the way a scan removes a file that is gone
  ASSERT_TRUE(m_repo.deleteContentFile(thirdDisc));
  EXPECT_TRUE(m_discSets.materializePlaylist(*survivor->discSetId, "disc1"));

  EXPECT_EQ(discLinesOf(playlistFor("disc1")).find("disc3"), std::string::npos)
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

  // Disc 1 goes away the way a scan reports a file that is gone, which keeps its row
  ASSERT_TRUE(m_repo.markContentFileMissing(firstDisc));

  const auto remaining = survivorOfSet(setId);
  ASSERT_FALSE(remaining.empty()) << "the set was left with nobody to rebuild for";
  EXPECT_TRUE(m_discSets.materializePlaylist(setId, remaining));

  EXPECT_TRUE(std::filesystem::exists(playlistFor(remaining)));
  EXPECT_EQ(discLinesOf(playlistFor(remaining)).find("disc1"), std::string::npos)
      << "the playlist still names the disc that is gone";
}

// Nothing supplies the count yet, so a set records 0 and the verdict says nothing about discs
// TODO
// Scanning discs says nothing about how many the game came on, because owning two is not knowing
// there are only two. A playlist naming them is the one source that does not need a database
TEST_F(DiscSetServiceTest, ASetLearnsItsDiscCountFromAPlaylistAndNotFromScanning) {
  const auto one = addTitledDisc("Shenmue", "disc1", 1);
  addTitledDisc("Shenmue", "disc2", 2);

  const auto entry = m_repo.getEntryWithContentHash("disc1");
  ASSERT_TRUE(entry.has_value());
  ASSERT_TRUE(entry->discSetId.has_value());
  EXPECT_EQ(m_repo.getDiscSet(*entry->discSetId)->discCount, 0) << "a count was guessed from what is on hand";

  const auto onePath = m_repo.getContentFile(one)->m_filePath;
  const auto setId =
      m_discSets.claimPlaylist(romsPath() + "/shenmue.m3u", {onePath, romsPath() + "/disc2.cue",
                                                             romsPath() + "/disc3.cue", romsPath() + "/disc4.cue"});

  ASSERT_TRUE(setId.has_value());
  EXPECT_EQ(m_repo.getDiscSet(*setId)->discCount, 4);
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
  addDisc("Grandia", "disc1", 1);
  addDisc("Grandia", "disc2", 2, 7, {"US"});

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

// TODO
// The last file going takes every way in with it and leaves the entry standing, so the game is
// still on the shelf and badged rather than gone
TEST_F(DiscSetServiceTest, AGameWhoseFilesAreAllGoneStaysInTheLibrary) {
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

// TODO
// A set is identified by its lowest disc, so disc 1 turning up later moves the identity onto it.
// The entry is re-keyed where it stands rather than replaced, so everything a person put on it
// survives the move
TEST_F(DiscSetServiceTest, ALateFirstDiscRekeysTheEntryInPlace) {
  addDisc("Final Fantasy VII", "disc2", 2);

  auto entry = m_repo.getEntryWithContentHash("disc2");
  ASSERT_TRUE(entry.has_value());
  const auto entryId = entry->id;

  // Things a person put on the entry, which a delete-and-recreate would lose
  entry->rating = 4;
  entry->favorite = true;
  ASSERT_TRUE(m_repo.update(*entry));

  addDisc("Final Fantasy VII", "disc1", 1);

  const auto after = m_repo.getEntry(entryId);
  ASSERT_TRUE(after.has_value()) << "the entry was replaced rather than re-keyed";
  EXPECT_EQ(after->contentHash, "disc1") << "the identity did not move onto the lower disc";
  EXPECT_EQ(after->rating, 4u) << "a rating did not survive the move";
  EXPECT_TRUE(after->favorite) << "a favourite did not survive the move";

  EXPECT_EQ(entryCount(), 1) << "a second entry was stood up beside the one that moved";
}

// TODO
// A row holding a position for a disc nobody has carries no hash, so it cannot be what the set is
// identified by however low its number
TEST_F(DiscSetServiceTest, ADiscNobodyHasCannotAnchorTheSet) {
  const auto two = addDisc("Lunar", "disc2", 2);
  const auto ownedPath = m_repo.getContentFile(two)->m_filePath;

  const auto setId =
      m_discSets.claimPlaylist(romsPath() + "/lunar.m3u", {romsPath() + "/missing-disc1.cue", ownedPath});
  ASSERT_TRUE(setId.has_value());

  auto entry = m_repo.getEntryWithContentHash("disc2");
  ASSERT_TRUE(entry.has_value());

  EXPECT_FALSE(m_discSets.syncSetEntry(*setId)) << "a claim with no file behind it was used as the anchor";
  EXPECT_EQ(m_repo.getEntry(entry->id)->contentHash, "disc2");
}

// TODO
// A playlist naming three discs where only one is on disk. Every line gets a row, the two nobody
// has yet hold their positions, and the set knows it is a three-disc game
TEST_F(DiscSetServiceTest, APlaylistClaimsDiscsThatAreNotHereYet) {
  const auto owned = addDisc("Final Fantasy VII", "disc2", 2);
  const auto ownedPath = m_repo.getContentFile(owned)->m_filePath;

  const auto setId = m_discSets.claimPlaylist(
      romsPath() + "/ff7.m3u", {romsPath() + "/missing-disc1.cue", ownedPath, romsPath() + "/missing-disc3.cue"});

  ASSERT_TRUE(setId.has_value());

  const auto members = m_repo.getDiscSetMembers(*setId);
  ASSERT_EQ(members.size(), 3u);

  EXPECT_FALSE(members[0].m_contentFileId.has_value()) << "a disc nobody has cannot name a file";
  EXPECT_TRUE(members[1].m_contentFileId.has_value());
  EXPECT_FALSE(members[2].m_contentFileId.has_value());

  // The playlist's order is the disc order, whatever the files are called
  EXPECT_EQ(members[0].m_discNumber, 1);
  EXPECT_EQ(members[1].m_discNumber, 2);
  EXPECT_EQ(members[2].m_discNumber, 3);

  for (const auto &member : members) {
    EXPECT_EQ(member.m_source, DiscSource::PlaylistFile);
  }

  const auto set = m_repo.getDiscSet(*setId);
  ASSERT_TRUE(set.has_value());
  EXPECT_EQ(set->discCount, 3) << "the line count is what says how many discs the game came on";
}

// TODO
// The disc turning up later fills in the row that was holding its place, rather than making a
// second claim on it
TEST_F(DiscSetServiceTest, ADiscArrivingLaterFillsInThePlaylistsClaim) {
  const auto missingPath = romsPath() + "/Final Fantasy VII (Disc 1) [disc1].cue";

  const auto setId = m_discSets.claimPlaylist(romsPath() + "/ff7.m3u", {missingPath});
  ASSERT_TRUE(setId.has_value());
  ASSERT_FALSE(m_repo.getDiscSetMembers(*setId).front().m_contentFileId.has_value());

  const auto arrived = addDisc("Final Fantasy VII", "disc1", 1);
  const auto landedIn = m_discSets.place(*m_repo.getContentFile(arrived));

  EXPECT_EQ(landedIn, setId) << "the arriving disc did not land in the set that claimed it";

  const auto members = m_repo.getDiscSetMembers(*setId);
  ASSERT_EQ(members.size(), 1u) << "binding a claim should not add a second row";
  EXPECT_EQ(members.front().m_contentFileId, arrived);
}

// TODO
// A playlist speaks with more authority than a filename, so it moves a disc grouping had guessed
// into the wrong set
TEST_F(DiscSetServiceTest, APlaylistMovesADiscGroupingHadGuessedAt) {
  const auto one = addDisc("Lunar", "disc1", 1);
  const auto guessedSet = m_discSets.place(*m_repo.getContentFile(one));
  ASSERT_TRUE(guessedSet.has_value());

  const auto path = m_repo.getContentFile(one)->m_filePath;
  const auto claimedSet = m_discSets.claimPlaylist(romsPath() + "/lunar.m3u", {path});

  ASSERT_TRUE(claimedSet.has_value());
  EXPECT_EQ(m_repo.getDiscSetMembers(*claimedSet).front().m_source, DiscSource::PlaylistFile);
}

// TODO
// Two discs of one game find the same set, in either scan order, with nothing named and nothing
// deleted
TEST_F(DiscSetServiceTest, PlacementPutsTwoDiscsInOneSet) {
  const auto one = addDisc("Final Fantasy VII", "disc1", 1);
  const auto two = addDisc("Final Fantasy VII", "disc2", 2);

  const auto firstSet = m_discSets.place(*m_repo.getContentFile(one));
  const auto secondSet = m_discSets.place(*m_repo.getContentFile(two));

  ASSERT_TRUE(firstSet.has_value());
  ASSERT_TRUE(secondSet.has_value());
  EXPECT_EQ(*firstSet, *secondSet);

  const auto members = m_repo.getDiscSetMembers(*firstSet);
  ASSERT_EQ(members.size(), 2u);
  EXPECT_EQ(members[0].m_discNumber, 1);
  EXPECT_EQ(members[1].m_discNumber, 2);
}

// TODO
// Placing the same file twice leaves it where it is rather than making a second claim on it
TEST_F(DiscSetServiceTest, PlacingADiscTwiceLeavesItWhereItIs) {
  const auto one = addDisc("Lunar", "disc1", 1);
  const auto file = m_repo.getContentFile(one);

  const auto first = m_discSets.place(*file);
  const auto again = m_discSets.place(*file);

  ASSERT_TRUE(first.has_value());
  EXPECT_EQ(again, first);
  EXPECT_EQ(m_repo.getDiscSetMembers(*first).size(), 1u);
}

// TODO
// A set already holding a disc at this number cannot be the right one, so the second copy gets a
// set of its own rather than displacing what is there
TEST_F(DiscSetServiceTest, ASecondCopyOfOneDiscGetsItsOwnSet) {
  const auto first = addDisc("Parasite Eve", "copyA", 2);
  const auto second = addDisc("Parasite Eve", "copyB", 2);

  const auto setA = m_discSets.place(*m_repo.getContentFile(first));
  const auto setB = m_discSets.place(*m_repo.getContentFile(second));

  ASSERT_TRUE(setA.has_value());
  ASSERT_TRUE(setB.has_value());
  EXPECT_NE(*setA, *setB) << "two copies of disc 2 were put in one set";
}

// TODO
// A lone disc is a set of one, which is the same shape as any other set rather than a special case
TEST_F(DiscSetServiceTest, ALoneDiscIsASetOfOne) {
  const auto only = addDisc("Grandia", "disc1", 1);

  const auto setId = m_discSets.place(*m_repo.getContentFile(only));

  ASSERT_TRUE(setId.has_value());
  EXPECT_EQ(m_repo.getDiscSetMembers(*setId).size(), 1u);
}

// TODO
// Membership is written as rows beside the column that still carries it, so the two say the same
// thing about which discs a set holds and at what numbers
TEST_F(DiscSetServiceTest, FormingASetRecordsItsMembership) {
  addTitledDisc("Final Fantasy VII", "disc1", 1);
  addTitledDisc("Final Fantasy VII", "disc2", 2);

  const auto survivor = m_repo.getEntryWithContentHash("disc1");
  ASSERT_TRUE(survivor.has_value());
  ASSERT_TRUE(survivor->discSetId.has_value());

  const auto members = m_repo.getDiscSetMembers(*survivor->discSetId);
  ASSERT_EQ(members.size(), 2u);

  EXPECT_EQ(members[0].m_discNumber, 1);
  EXPECT_EQ(members[1].m_discNumber, 2);

  for (const auto &member : members) {
    EXPECT_TRUE(member.m_contentFileId.has_value()) << "a catalogued disc should name its file";
    EXPECT_FALSE(member.m_memberPath.empty());
    EXPECT_EQ(member.m_source, DiscSource::Filename);
    EXPECT_FALSE(member.m_isUncertain);
  }

  // The column everything still reads and the rows beside it must agree
  const auto discs = m_repo.getDiscsInSet(*survivor->discSetId);
  EXPECT_EQ(discs.size(), members.size());
}

// TODO
// A disc leaving a set takes its membership row with it, so the rows and the column do not drift
// apart when somebody detaches one
TEST_F(DiscSetServiceTest, DetachingADiscTakesItsMembershipRowWithIt) {
  addTitledDisc("Lunar", "disc1", 1);
  const auto discTwoFileId = addTitledDisc("Lunar", "disc2", 2);

  const auto survivor = m_repo.getEntryWithContentHash("disc1");
  ASSERT_TRUE(survivor.has_value());
  ASSERT_TRUE(survivor->discSetId.has_value());
  const auto setId = *survivor->discSetId;

  ASSERT_EQ(m_repo.getDiscSetMembers(setId).size(), 2u);

  ASSERT_TRUE(m_discSets.detachDisc(discTwoFileId));

  // TODO
  // The disc takes its row with it and the set carries on as a set of one
  const auto remaining = m_repo.getDiscSetMembers(setId);
  ASSERT_EQ(remaining.size(), 1u);
  EXPECT_EQ(m_repo.getContentFile(*remaining.front().m_contentFileId)->m_contentHash, "disc1");
  EXPECT_EQ(m_repo.getDiscsInSet(setId).size(), 1u);
}

// TODO
// A disc has one home, so a second set cannot claim a file the first already holds
TEST_F(DiscSetServiceTest, ADiscBelongsToOneSetOnly) {
  addTitledDisc("Final Fantasy VII", "disc1", 1);
  addTitledDisc("Final Fantasy VII", "disc2", 2);

  const auto survivor = m_repo.getEntryWithContentHash("disc1");
  ASSERT_TRUE(survivor.has_value());
  ASSERT_TRUE(survivor->discSetId.has_value());

  const auto members = m_repo.getDiscSetMembers(*survivor->discSetId);
  ASSERT_FALSE(members.empty());

  DiscSetMember stolen{.m_discSetId = *survivor->discSetId + 1,
                       .m_discNumber = 1,
                       .m_contentFileId = members.front().m_contentFileId,
                       .m_memberPath = "somewhere/else.cue",
                       .m_source = DiscSource::User};

  EXPECT_FALSE(m_repo.create(stolen)) << "a file already in a set was taken by a second one";
}

// TODO
// The dump carries its title from the filename at ingest, so discs group on that without waiting
// for metadata population to write a title onto the entry
TEST_F(DiscSetServiceTest, DiscsGroupOnTheTitlesTheirFilenamesGaveThem) {
  addDisc("Final Fantasy VII", "disc1", 1);
  addDisc("Final Fantasy VII", "disc2", 2);

  EXPECT_EQ(entryCount(), 1) << "two discs sharing a filename title did not become one game";

  const auto entry = m_repo.getEntryWithContentHash("disc1");
  ASSERT_TRUE(entry.has_value());
  EXPECT_TRUE(entry->normalizedTitle.empty()) << "grouping should not have needed a titled entry";

  ASSERT_TRUE(entry->discSetId.has_value());
  EXPECT_EQ(m_repo.getDiscsInSet(*entry->discSetId).size(), 2u);

  // The set launches through its anchor, so the disc reached through it keeps no way in
  EXPECT_TRUE(configsFor("disc2").empty());
  EXPECT_EQ(configsFor("disc1").size(), 1u);
}

// TODO
// One title spans a game's regional releases, so the region check is the only thing left keeping
// two of them apart
TEST_F(DiscSetServiceTest, TwoRegionalReleasesOfOneTitleDoNotMerge) {
  addIdentifiedDisc("Resident Evil 2", "japanDisc1", 1, {"JP"});
  addIdentifiedDisc("Resident Evil 2", "usaDisc2", 2, {"US"});

  EXPECT_EQ(entryCount(), 2) << "discs of two releases were folded into one set";
}

// Coverage arrives one dump at a time, so a database that knows one disc and not its sibling
// must not split a set that the titles alone would have formed
TEST_F(DiscSetServiceTest, PartialDatabaseCoverageStillGroups) {
  addIdentifiedDisc("Grandia", "disc1", 1);
  addTitledDisc("Grandia", "disc2", 2);

  EXPECT_EQ(entryCount(), 1) << "knowing one disc and not the other split the set";
}

// TODO
// A disc that is not on disk keeps its hash, so it keeps anchoring. An unplugged drive moving the
// identity onto a disc that happens to be present would take every save with it
TEST_F(DiscSetServiceTest, AMissingAnchorStillIdentifiesTheSet) {
  const auto discOne = addTitledDisc("Final Fantasy VII", "disc1", 1);
  addTitledDisc("Final Fantasy VII", "disc2", 2);
  addTitledDisc("Final Fantasy VII", "disc3", 3);

  const auto before = m_repo.getEntryWithContentHash("disc1");
  ASSERT_TRUE(before.has_value());
  const auto entryId = before->id;

  ASSERT_TRUE(m_repo.markContentFileMissing(discOne));

  // The row survives with its hash, or the rest of this proves nothing
  const auto gone = m_repo.getContentFile(discOne);
  ASSERT_TRUE(gone.has_value());
  EXPECT_EQ(gone->m_contentHash, "disc1");
  EXPECT_NE(gone->m_missingSince, 0);

  m_identityChanges.clear();
  m_absorbed.clear();

  // A real arrival, so placement and the way in are rebuilt while the anchor is unreachable
  addTitledDisc("Final Fantasy VII", "disc4", 4);

  const auto after = m_repo.getEntry(entryId);
  ASSERT_TRUE(after.has_value());
  EXPECT_EQ(after->contentHash, "disc1") << "the identity walked onto a disc that happened to be present";
  EXPECT_TRUE(m_identityChanges.empty()) << "an identity move was announced for a disc that only went missing";
  EXPECT_TRUE(m_absorbed.empty());
  EXPECT_EQ(entryCount(), 1);

  // The playlist still belongs to disc 1, and no longer names the file that is gone
  EXPECT_TRUE(std::filesystem::exists(playlistFor("disc1")));
  EXPECT_FALSE(std::filesystem::exists(playlistFor("disc2"))) << "the set re-anchored onto a present disc";
  EXPECT_EQ(discLinesOf(playlistFor("disc1")).find("disc1]"), std::string::npos)
      << "the playlist names a disc that is not there";
}

// TODO
// Folding two rows into one says which hash survived and which was taken over, because saves and
// playtime are keyed on the hash and have to be told where to go
TEST_F(DiscSetServiceTest, FoldingTwoEntriesIntoOneSaysWhichHashSurvived) {
  addTitledDisc("Koudelka", "disc1", 1);
  const auto secondDisc = addTitledDisc("Koudelka", "disc2", 2);

  ASSERT_TRUE(m_discSets.detachDisc(secondDisc));
  ASSERT_EQ(entryCount(), 2);

  const auto kept = m_repo.getEntryWithContentHash("disc1");
  const auto absorbed = m_repo.getEntryWithContentHash("disc2");
  ASSERT_TRUE(kept.has_value());
  ASSERT_TRUE(absorbed.has_value());

  m_absorbed.clear();
  ASSERT_TRUE(m_discSets.clearUserChoice(absorbed->id));

  ASSERT_EQ(m_absorbed.size(), 1u) << "a row was taken over without saying so";
  EXPECT_EQ(m_absorbed.front().survivingEntryId, kept->id);
  EXPECT_EQ(m_absorbed.front().absorbedContentHash, "disc2");
  EXPECT_EQ(m_absorbed.front().survivingContentHash, "disc1");

  EXPECT_FALSE(m_repo.getEntry(absorbed->id).has_value());
  EXPECT_EQ(m_repo.getEntry(kept->id)->contentHash, "disc1");
  EXPECT_TRUE(configsFor("disc2").empty()) << "the taken-over row kept a way in";
}

// TODO
// Taking the disc a set is identified by out of it leaves the rest of the set still named after
// the game rather than after whichever file is left
TEST_F(DiscSetServiceTest, DetachingTheDiscASetIsIdentifiedByLeavesTheRestNamed) {
  const auto firstDisc = addTitledDisc("Lunar", "disc1", 1);
  addTitledDisc("Lunar", "disc2", 2);

  ASSERT_TRUE(m_discSets.detachDisc(firstDisc));
  ASSERT_EQ(entryCount(), 2);

  const auto rest = m_repo.getEntryWithContentHash("disc2");
  ASSERT_TRUE(rest.has_value());
  EXPECT_EQ(rest->displayName, "Lunar") << "the discs left behind fell back to a filename";

  const auto detached = m_repo.getEntryWithContentHash("disc1");
  ASSERT_TRUE(detached.has_value());
  EXPECT_EQ(detached->displayName, "Lunar") << "the disc taken out fell back to a filename";
}

// TODO
// Taking the only disc out of a set leaves nothing behind it, because a set holding no discs is
// not something anybody can act on
TEST_F(DiscSetServiceTest, DetachingTheOnlyDiscLeavesNoEmptySetBehind) {
  const auto only = addTitledDisc("Patapon", "disc1", 1);

  const auto before = m_repo.getEntryWithContentHash("disc1");
  ASSERT_TRUE(before.has_value());
  ASSERT_TRUE(before->discSetId.has_value());
  const auto previousSetId = *before->discSetId;

  ASSERT_TRUE(m_discSets.detachDisc(only));

  EXPECT_FALSE(m_repo.getDiscSet(previousSetId).has_value()) << "the set it left is still there holding nothing";
  EXPECT_EQ(entryCount(), 1);

  const auto after = m_repo.getEntryWithContentHash("disc1");
  ASSERT_TRUE(after.has_value());
  ASSERT_TRUE(after->discSetId.has_value());
  EXPECT_NE(*after->discSetId, previousSetId);
}

// TODO
// Renumbering records that a person decided it, so placement running again leaves the number alone
TEST_F(DiscSetServiceTest, RenumberingADiscIsRecordedAsThePersonsChoice) {
  addTitledDisc("Grandia", "disc1", 1);
  const auto second = addTitledDisc("Grandia", "disc2", 2);

  const auto member = m_repo.getDiscSetMemberForContentFile(second);
  ASSERT_TRUE(member.has_value());
  EXPECT_EQ(member->m_source, DiscSource::Filename);

  auto renumbered = *member;
  renumbered.m_discNumber = 3;
  renumbered.m_source = DiscSource::User;
  renumbered.m_isUncertain = false;
  ASSERT_TRUE(m_repo.create(renumbered));

  const auto after = m_repo.getDiscSetMemberForContentFile(second);
  ASSERT_TRUE(after.has_value());
  EXPECT_EQ(after->m_discNumber, 3);
  EXPECT_EQ(after->m_source, DiscSource::User);
  EXPECT_FALSE(after->m_isUncertain);
}

// TODO
// A disc a person took out of a set stays out of it when a playlist claims that set afterwards.
// The person's choice sits above the playlist on the ladder
TEST_F(DiscSetServiceTest, APlaylistDoesNotUndoADetach) {
  const auto firstDisc = addTitledDisc("Riven", "disc1", 1);
  const auto secondDisc = addTitledDisc("Riven", "disc2", 2);

  ASSERT_TRUE(m_discSets.detachDisc(secondDisc));

  const auto detachedInto = m_repo.getDiscSetMemberForContentFile(secondDisc);
  ASSERT_TRUE(detachedInto.has_value());
  EXPECT_EQ(detachedInto->m_source, DiscSource::User);

  const auto firstPath = m_repo.getContentFile(firstDisc)->m_filePath;
  const auto secondPath = m_repo.getContentFile(secondDisc)->m_filePath;

  ASSERT_TRUE(m_discSets.claimPlaylist(romsPath() + "/riven.m3u", {firstPath, secondPath}).has_value());

  const auto after = m_repo.getDiscSetMemberForContentFile(secondDisc);
  ASSERT_TRUE(after.has_value());
  EXPECT_EQ(after->m_discSetId, detachedInto->m_discSetId) << "a playlist pulled back a disc a person took out";
  EXPECT_EQ(after->m_source, DiscSource::User);
}

// TODO
// A disc whose name carries no number is the only one anything knows about, so it takes first
// place in a set of its own rather than being left out of the model
TEST_F(DiscSetServiceTest, ADiscWithNoNumberIsDiscOneOfItsOwnSet) {
  addDisc("Mega Man Legends", "onlyDisc", 0);

  const auto entry = m_repo.getEntryWithContentHash("onlyDisc");
  ASSERT_TRUE(entry.has_value());
  ASSERT_TRUE(entry->discSetId.has_value()) << "a disc carrying no number joined nothing";

  const auto members = m_repo.getDiscSetMembers(*entry->discSetId);
  ASSERT_EQ(members.size(), 1u);
  EXPECT_EQ(members.front().m_discNumber, 1);
}

// TODO
// One disc dumped twice is two files of one disc, and both have to reach the renderer so it can
// pick the one it can open
TEST_F(DiscSetServiceTest, OneDiscDumpedTwiceIsTwoMembersOfOneSet) {
  const auto asCue = addDisc("Mega Man Legends", "sameDump", 1);

  ContentFile asChd;
  asChd.m_type = ContentType::Disc;
  asChd.m_filePath = romsPath() + "/Mega Man Legends.chd";
  asChd.m_platformId = 7;
  asChd.m_contentHash = "sameDump";
  asChd.m_discNumber = 1;
  asChd.m_normalizedTitle = normalizeTitle("Mega Man Legends");
  asChd.m_fileSizeBytes = 2000;
  ASSERT_TRUE(m_repo.create(asChd));

  const auto placed = m_repo.getDiscSetMemberForContentFile(asChd.m_id);
  ASSERT_TRUE(placed.has_value()) << "the second copy of one disc joined nothing";

  const auto held = m_repo.getDiscSetMemberForContentFile(asCue);
  ASSERT_TRUE(held.has_value());
  EXPECT_EQ(placed->m_discSetId, held->m_discSetId) << "two copies of one disc landed in different sets";

  EXPECT_EQ(m_repo.getDiscSetMembers(placed->m_discSetId).size(), 2u);
  EXPECT_EQ(entryCount(), 1) << "one disc dumped twice read as two games";
}

// TODO
// A file at the path a set's playlist would take that is not one of ours is left exactly as it is,
// because it is somebody else's and the path is one anybody can write to
TEST_F(DiscSetServiceTest, APlaylistWeDidNotWriteIsNeverOverwritten) {
  addTitledDisc("Riven", "disc1", 1);
  addTitledDisc("Riven", "disc2", 2);

  const auto entry = m_repo.getEntryWithContentHash("disc1");
  ASSERT_TRUE(entry.has_value());
  ASSERT_TRUE(entry->discSetId.has_value());

  const auto path = playlistFor("disc1");
  const std::string theirs = "somebody else wrote this\n";
  std::filesystem::create_directories(std::filesystem::path(path).parent_path());
  {
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    out << theirs;
  }

  EXPECT_FALSE(m_discSets.materializePlaylist(*entry->discSetId, "disc1")) << "somebody else's file was overwritten";
  EXPECT_EQ(readFile(path), theirs);
}

// TODO
// The same rule when a set stops launching under a hash: a file at that path we did not write is
// not ours to delete
TEST_F(DiscSetServiceTest, APlaylistWeDidNotWriteIsNeverDeleted) {
  const auto path = playlistFor("someHash");
  const std::string theirs = "somebody else wrote this\n";
  std::filesystem::create_directories(std::filesystem::path(path).parent_path());
  {
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    out << theirs;
  }

  addTitledDisc("Xenogears", "disc2", 2);
  addTitledDisc("Xenogears", "disc1", 1);

  EXPECT_TRUE(std::filesystem::exists(path)) << "somebody else's file was deleted";
  EXPECT_EQ(readFile(path), theirs);
}

} // namespace firelight::library

// TODO: NEEDS REVIEW
#include "app/library/variant_group_service.hpp"

#include <firelight/library/filename_tags.hpp>
#include <firelight/library/sqlite_user_library.hpp>
#include <firelight/library/user_library_service.hpp>
#include <firelight/settings/sqlite_settings_repository.hpp>

#include <QDir>
#include <gtest/gtest.h>

// Verifies which entry stands for a variant group, and when that is allowed to move.
// The ranking itself is covered by firelight_library_test; this is about the rules
// around a user's pin and a file going missing
namespace firelight::library {

class VariantGroupServiceTest : public testing::Test {
protected:
  SqliteUserLibraryRepository m_repo{":memory:"};
  UserLibraryService m_library{m_repo, (QDir::tempPath() + "/fl_vgs_test").toStdString()};
  settings::SqliteSettingsRepository m_settingsRepo{":memory:"};
  settings::SettingsService m_settings{m_settingsRepo};

  void SetUp() override {
    m_settings.setGlobalValue(VariantGroupService::REGION_PRIORITY_KEY, "US,EU,JP,WORLD");
    m_settings.setGlobalValue(VariantGroupService::LANGUAGE_PRIORITY_KEY, "en");
  }

  // A release is only a candidate to stand for a group if it has something to launch, so every
  // fixture entry gets a file the way a scan would give it one
  void catalogue(const std::string &hash) {
    ContentFile file{
        .m_fileSizeBytes = 1024, .m_filePath = "/roms/" + hash + ".sfc", .m_platformId = 6, .m_contentHash = hash};
    ASSERT_TRUE(m_repo.create(file));
  }

  // Creates an entry in a group with the given regions
  int makeMember(const int groupId, const std::string &name, const std::string &hash,
                 const std::vector<std::string> &regions) {
    Entry entry;
    entry.displayName = name;
    entry.contentHash = hash;
    entry.platformId = 6;
    m_repo.createEntry(entry);
    catalogue(hash);

    GameMetadata metadata;
    metadata.regions = regions;
    m_repo.applyEntryMetadata(entry.id, metadata, {metadata_fields::REGIONS}, false);
    m_repo.setEntryVariantGroup(entry.id, groupId, true);

    return entry.id;
  }

  // Creates an entry belonging to no group
  int makeUngrouped(const std::string &name, const std::string &hash, const std::vector<std::string> &regions,
                    const unsigned platformId = 6) {
    Entry entry;
    entry.displayName = name;
    entry.contentHash = hash;
    entry.platformId = platformId;
    m_repo.createEntry(entry);
    catalogue(hash);

    GameMetadata metadata;
    metadata.regions = regions;
    m_repo.applyEntryMetadata(entry.id, metadata, {metadata_fields::REGIONS}, false);

    return entry.id;
  }

  // Creates an entry with a derived title, which is what auto grouping matches on
  int makeTitled(const std::string &name, const std::string &hash, const std::string &normalizedTitle,
                 const unsigned platformId = 6) {
    Entry entry;
    entry.displayName = name;
    entry.contentHash = hash;
    entry.platformId = platformId;
    m_repo.createEntry(entry);
    catalogue(hash);

    entry.normalizedTitle = normalizedTitle;
    m_repo.updateEntryMetadata(entry);

    return entry.id;
  }

  // Creates an entry whose metadata came from parsing the given filename, the way populate does
  int makeUngroupedFromFile(const std::string &fileName) {
    const auto tags = parseFilenameTags(fileName);

    Entry entry;
    entry.displayName = tags.title;
    entry.contentHash = fileName;
    entry.platformId = 6;
    m_repo.createEntry(entry);

    entry.normalizedTitle = normalizeTitle(tags.title);
    m_repo.updateEntryMetadata(entry);

    // The disc a file is lives on the content file, so the entry needs one to read it through
    ContentFile file;
    file.m_filePath = fileName;
    file.m_fileSizeBytes = 1;
    file.m_fileMd5 = fileName;
    file.m_fileCrc32 = "0";
    file.m_platformId = 6;
    file.m_contentHash = fileName;
    file.m_discNumber = tags.discNumber;
    m_repo.create(file);

    GameMetadata metadata;
    metadata.regions = tags.regions;
    m_repo.applyEntryMetadata(entry.id, metadata, {metadata_fields::REGIONS}, false);

    return entry.id;
  }

  int makeGroup(const std::string &title) {
    VariantGroup group{.title = title};
    m_repo.createVariantGroup(group);
    return group.id;
  }

  // Availability is read from the files, so making a release unplayable means taking its bytes away
  void setHidden(const int entryId, const bool hidden) {
    const auto entry = m_repo.getEntry(entryId);
    ASSERT_TRUE(entry.has_value());

    for (const auto &file : m_repo.getContentFilesWithContentHash(entry->contentHash)) {
      if (hidden) {
        ASSERT_TRUE(m_repo.markContentFileMissing(file.m_id));
      } else {
        ASSERT_TRUE(m_repo.reviveContentFile(file.m_id));
      }
    }
  }

  [[nodiscard]] std::optional<int> primaryOf(const int groupId) {
    const auto group = m_repo.getVariantGroup(groupId);
    return group.has_value() ? group->primaryEntryId : std::nullopt;
  }
};

TEST_F(VariantGroupServiceTest, PicksThePreferredRegion) {
  VariantGroupService service(m_library, m_settings);

  const auto groupId = makeGroup("Super Metroid");
  makeMember(groupId, "Super Metroid (Japan)", "hashJp", {"JP"});
  const auto usa = makeMember(groupId, "Super Metroid (USA)", "hashUsa", {"US"});

  service.reevaluate(groupId);

  EXPECT_EQ(primaryOf(groupId), usa);
}

TEST_F(VariantGroupServiceTest, APinnedPrimaryDoesNotMove) {
  VariantGroupService service(m_library, m_settings);

  const auto groupId = makeGroup("Super Metroid");
  const auto japan = makeMember(groupId, "Super Metroid (Japan)", "hashJp", {"JP"});
  makeMember(groupId, "Super Metroid (USA)", "hashUsa", {"US"});

  ASSERT_TRUE(m_library.setVariantGroupPrimary(groupId, japan));
  service.reevaluate(groupId);

  EXPECT_EQ(primaryOf(groupId), japan);
}

// A group whose primary cannot be launched is broken, so a missing file is bypassed
// even when the user chose it -- but the pin itself survives, so the choice comes
// back with the file
TEST_F(VariantGroupServiceTest, AHiddenPinnedPrimaryIsBypassedAndThenRestored) {
  VariantGroupService service(m_library, m_settings);

  const auto groupId = makeGroup("Super Metroid");
  const auto japan = makeMember(groupId, "Super Metroid (Japan)", "hashJp", {"JP"});
  const auto usa = makeMember(groupId, "Super Metroid (USA)", "hashUsa", {"US"});

  ASSERT_TRUE(m_library.setVariantGroupPrimary(groupId, japan));

  setHidden(japan, true);
  EXPECT_EQ(service.resolvePrimary(groupId), usa);

  // The pin is what makes the choice come back, so it must survive the file going
  // missing rather than being overwritten by the fallback
  EXPECT_EQ(primaryOf(groupId), japan);

  setHidden(japan, false);
  EXPECT_EQ(service.resolvePrimary(groupId), japan);
}

TEST_F(VariantGroupServiceTest, ChangingThePreferenceMovesAnAutomaticPrimary) {
  VariantGroupService service(m_library, m_settings);

  const auto groupId = makeGroup("Super Metroid");
  const auto japan = makeMember(groupId, "Super Metroid (Japan)", "hashJp", {"JP"});
  const auto usa = makeMember(groupId, "Super Metroid (USA)", "hashUsa", {"US"});

  service.reevaluate(groupId);
  ASSERT_EQ(primaryOf(groupId), usa);

  m_settings.setGlobalValue(VariantGroupService::REGION_PRIORITY_KEY, "JP,US,EU");
  service.reevaluateAll();

  EXPECT_EQ(primaryOf(groupId), japan);
}

// A group whose every release is unavailable still has to stand for something: with variants
// collapsed, a group with no primary has no row and the game vanishes from the library
TEST_F(VariantGroupServiceTest, AGroupWithOnlyUnavailableMembersStillHasAPrimary) {
  VariantGroupService service(m_library, m_settings);

  const auto groupId = makeGroup("Super Metroid");
  const auto japan = makeMember(groupId, "Super Metroid (Japan)", "hashJp", {"JP"});

  setHidden(japan, true);
  service.reevaluate(groupId);

  EXPECT_EQ(primaryOf(groupId), japan);
}

// Grouping names the set after whichever entry comes to stand for it
TEST_F(VariantGroupServiceTest, GroupingTwoEntriesTitlesTheSetAfterItsPrimary) {
  const auto japan = makeUngrouped("Zelda (Japan)", "hashJ", {"JP"});
  const auto usa = makeUngrouped("Zelda (USA)", "hashU", {"US"});

  VariantGroupService service(m_library, m_settings);

  const auto groupId = service.createGroupFrom({japan, usa});
  ASSERT_TRUE(groupId.has_value());

  const auto group = m_repo.getVariantGroup(*groupId);
  ASSERT_TRUE(group.has_value());
  EXPECT_EQ(group->primaryEntryId, usa);
  EXPECT_EQ(group->title, "Zelda (USA)");
  EXPECT_FALSE(group->titleUserSet);
}

// Two games on different systems are not two versions of one game
TEST_F(VariantGroupServiceTest, EntriesOnDifferentPlatformsWillNotGroup) {
  const auto snes = makeUngrouped("Zelda (USA)", "hashU", {"US"});
  const auto gameboy = makeUngrouped("Zelda (Japan)", "hashJ", {"JP"}, 7);

  VariantGroupService service(m_library, m_settings);

  EXPECT_FALSE(service.createGroupFrom({snes, gameboy}).has_value());
}

// One entry is not a set
TEST_F(VariantGroupServiceTest, ASingleEntryWillNotGroup) {
  const auto usa = makeUngrouped("Zelda (USA)", "hashU", {"US"});
  VariantGroupService service(m_library, m_settings);

  EXPECT_FALSE(service.createGroupFrom({usa}).has_value());
}

// Taking one of two out leaves nothing to group, so the group goes with it
TEST_F(VariantGroupServiceTest, RemovingDownToOneEntryDissolvesTheGroup) {
  const auto groupId = makeGroup("Zelda");
  const auto usa = makeMember(groupId, "Zelda (USA)", "hashU", {"US"});
  const auto japan = makeMember(groupId, "Zelda (Japan)", "hashJ", {"JP"});

  VariantGroupService service(m_library, m_settings);

  ASSERT_TRUE(service.removeFromGroup(japan));

  EXPECT_FALSE(m_repo.getVariantGroup(groupId).has_value());

  const auto survivor = m_repo.getEntry(usa);
  ASSERT_TRUE(survivor.has_value());
  EXPECT_FALSE(survivor->variantGroupId.has_value());
}

// A set of three survives losing one
TEST_F(VariantGroupServiceTest, RemovingFromAGroupOfThreeKeepsTheGroup) {
  const auto groupId = makeGroup("Zelda");
  makeMember(groupId, "Zelda (USA)", "hashU", {"US"});
  const auto europe = makeMember(groupId, "Zelda (Europe)", "hashE", {"EU"});
  makeMember(groupId, "Zelda (Japan)", "hashJ", {"JP"});

  VariantGroupService service(m_library, m_settings);

  ASSERT_TRUE(service.removeFromGroup(europe));

  EXPECT_TRUE(m_repo.getVariantGroup(groupId).has_value());
}

// The flag is what a "don't ask me again" answer writes
TEST_F(VariantGroupServiceTest, TheAutoLaunchFlagRoundTrips) {
  const auto groupId = makeGroup("Zelda");
  makeMember(groupId, "Zelda (USA)", "hashU", {"US"});

  VariantGroupService service(m_library, m_settings);

  ASSERT_TRUE(service.setAutoLaunchPrimary(groupId, true));

  const auto group = m_repo.getVariantGroup(groupId);
  ASSERT_TRUE(group.has_value());
  EXPECT_TRUE(group->autoLaunchPrimary);
}

// Renaming stops the title following whatever stands for the group
TEST_F(VariantGroupServiceTest, ARenamedGroupStopsFollowingItsPrimary) {
  const auto groupId = makeGroup("Zelda (USA)");
  const auto usa = makeMember(groupId, "Zelda (USA)", "hashU", {"US"});
  const auto japan = makeMember(groupId, "Zelda (Japan)", "hashJ", {"JP"});

  VariantGroupService service(m_library, m_settings);

  ASSERT_TRUE(service.setTitle(groupId, "The Legend of Zelda"));
  ASSERT_TRUE(service.pinPrimary(groupId, japan));

  const auto group = m_repo.getVariantGroup(groupId);
  ASSERT_TRUE(group.has_value());
  EXPECT_EQ(group->title, "The Legend of Zelda");
  EXPECT_EQ(group->primaryEntryId, japan);
}

// Answering for the whole library must agree with answering one group at a time
TEST_F(VariantGroupServiceTest, ResolvingEveryGroupAtOnceMatchesResolvingEachAlone) {
  const auto first = makeGroup("Zelda");
  makeMember(first, "Zelda (Japan)", "hashJ", {"JP"});
  const auto firstUsa = makeMember(first, "Zelda (USA)", "hashU", {"US"});

  const auto second = makeGroup("Metroid");
  const auto secondEurope = makeMember(second, "Metroid (Europe)", "hashME", {"EU"});
  makeMember(second, "Metroid (Japan)", "hashMJ", {"JP"});

  VariantGroupService service(m_library, m_settings);

  const auto resolution = service.resolveAll(m_repo.getEntries(), m_repo.getVariantGroups());

  EXPECT_EQ(resolution.primaryByGroup.at(first), firstUsa);
  EXPECT_EQ(resolution.primaryByGroup.at(second), secondEurope);
  EXPECT_EQ(resolution.memberCountByGroup.at(first), 2);
  EXPECT_EQ(resolution.memberCountByGroup.at(second), 2);
}

// A hidden entry cannot be launched, so it does not get to stand for the group
TEST_F(VariantGroupServiceTest, ResolvingEveryGroupAtOnceSkipsHiddenEntries) {
  const auto groupId = makeGroup("Zelda");
  const auto usa = makeMember(groupId, "Zelda (USA)", "hashU", {"US"});
  const auto japan = makeMember(groupId, "Zelda (Japan)", "hashJ", {"JP"});
  setHidden(usa, true);

  VariantGroupService service(m_library, m_settings);

  const auto resolution = service.resolveAll(m_repo.getEntries(), m_repo.getVariantGroups());

  EXPECT_EQ(resolution.primaryByGroup.at(groupId), japan);
}

// Two dumps of one game land in a group without anyone asking
TEST_F(VariantGroupServiceTest, TwoEntriesWithTheSameTitleGroupThemselves) {
  const auto usa = makeTitled("Zelda (USA)", "hashU", "zelda");
  const auto japan = makeTitled("Zelda (Japan)", "hashJ", "zelda");

  VariantGroupService service(m_library, m_settings);

  EXPECT_TRUE(service.autoGroupByTitle(japan));

  const auto grouped = m_repo.getEntry(usa);
  const auto other = m_repo.getEntry(japan);
  ASSERT_TRUE(grouped.has_value());
  ASSERT_TRUE(other.has_value());
  ASSERT_TRUE(grouped->variantGroupId.has_value());
  EXPECT_EQ(grouped->variantGroupId, other->variantGroupId);
}

// A third release joins the group the first two made rather than starting a second.
// It is added while the service is alive, so the grouping happens off the entry event
// rather than through a call
TEST_F(VariantGroupServiceTest, AThirdEntryJoinsTheExistingGroup) {
  const auto usa = makeTitled("Zelda (USA)", "hashU", "zelda");
  const auto japan = makeTitled("Zelda (Japan)", "hashJ", "zelda");

  VariantGroupService service(m_library, m_settings);
  ASSERT_TRUE(service.autoGroupByTitle(japan));

  makeTitled("Zelda (Europe)", "hashE", "zelda");

  EXPECT_EQ(m_repo.getVariantGroups().size(), 1u);
  EXPECT_EQ(m_repo.getEntriesInVariantGroup(*m_repo.getEntry(usa)->variantGroupId).size(), 3u);
}

// Two systems' releases of the same game are not versions of one entry
TEST_F(VariantGroupServiceTest, MatchingTitlesOnDifferentPlatformsDoNotGroup) {
  makeTitled("Zelda (USA)", "hashU", "zelda");
  const auto gameboy = makeTitled("Zelda (Japan)", "hashJ", "zelda", 7);

  VariantGroupService service(m_library, m_settings);

  EXPECT_FALSE(service.autoGroupByTitle(gameboy));
  EXPECT_TRUE(m_repo.getVariantGroups().empty());
}

// A regional retitle is not an exact match, and is left for someone to group by hand
TEST_F(VariantGroupServiceTest, ARetitledReleaseIsNotGroupedAutomatically) {
  makeTitled("Final Fantasy IV", "hashJ", "final fantasy iv");
  const auto west = makeTitled("Final Fantasy II", "hashU", "final fantasy ii");

  VariantGroupService service(m_library, m_settings);

  EXPECT_FALSE(service.autoGroupByTitle(west));
  EXPECT_TRUE(m_repo.getVariantGroups().empty());
}

// Nothing to match against yet is not a group of one
TEST_F(VariantGroupServiceTest, AnEntryWithNoMatchIsLeftAlone) {
  const auto only = makeTitled("Zelda (USA)", "hashU", "zelda");

  VariantGroupService service(m_library, m_settings);

  EXPECT_FALSE(service.autoGroupByTitle(only));
  EXPECT_FALSE(m_repo.getEntry(only)->variantGroupId.has_value());
}

// An entry with no derived title yet must not group with every other untitled entry
TEST_F(VariantGroupServiceTest, EntriesWithoutADerivedTitleDoNotGroup) {
  makeTitled("Zelda (USA)", "hashU", "");
  const auto japan = makeTitled("Zelda (Japan)", "hashJ", "");

  VariantGroupService service(m_library, m_settings);

  EXPECT_FALSE(service.autoGroupByTitle(japan));
  EXPECT_TRUE(m_repo.getVariantGroups().empty());
}

// Taking an entry out of a group has to stick, or the automatic grouper puts it straight back
TEST_F(VariantGroupServiceTest, RemovingAnAutoGroupedEntryKeepsItOut) {
  makeTitled("Zelda (USA)", "hashU", "zelda");
  makeTitled("Zelda (Europe)", "hashE", "zelda");
  const auto japan = makeTitled("Zelda (Japan)", "hashJ", "zelda");

  VariantGroupService service(m_library, m_settings);
  ASSERT_TRUE(service.autoGroupByTitle(japan));
  ASSERT_TRUE(m_repo.getEntry(japan)->variantGroupId.has_value());

  ASSERT_TRUE(service.removeFromGroup(japan));

  EXPECT_FALSE(m_repo.getEntry(japan)->variantGroupId.has_value());
}

// A rescan republishes the entry event, which is the moment the grouper would undo the removal
TEST_F(VariantGroupServiceTest, ARemovedEntryStaysOutAcrossLaterUpdates) {
  makeTitled("Zelda (USA)", "hashU", "zelda");
  makeTitled("Zelda (Europe)", "hashE", "zelda");
  const auto japan = makeTitled("Zelda (Japan)", "hashJ", "zelda");

  VariantGroupService service(m_library, m_settings);
  ASSERT_TRUE(service.autoGroupByTitle(japan));
  ASSERT_TRUE(service.removeFromGroup(japan));

  auto entry = m_repo.getEntry(japan);
  ASSERT_TRUE(entry.has_value());
  entry->displayName = "Zelda (Japan) again";
  ASSERT_TRUE(m_repo.update(*entry));

  EXPECT_FALSE(m_repo.getEntry(japan)->variantGroupId.has_value());
}

// Handing an entry back to automatic grouping puts it where it belongs again
TEST_F(VariantGroupServiceTest, ClearingTheUserChoiceLetsItGroupAgain) {
  makeTitled("Zelda (USA)", "hashU", "zelda");
  const auto japan = makeTitled("Zelda (Japan)", "hashJ", "zelda");

  VariantGroupService service(m_library, m_settings);
  ASSERT_TRUE(service.autoGroupByTitle(japan));
  ASSERT_TRUE(service.removeFromGroup(japan));
  ASSERT_FALSE(m_repo.getEntry(japan)->variantGroupId.has_value());

  EXPECT_TRUE(service.clearUserChoice(japan));
  EXPECT_TRUE(m_repo.getEntry(japan)->variantGroupId.has_value());
}

// A set the user built by hand is not rearranged by the grouper
TEST_F(VariantGroupServiceTest, AHandMadeGroupIsLeftAloneByAutomaticGrouping) {
  const auto usa = makeTitled("Zelda (USA)", "hashU", "zelda");
  const auto japan = makeTitled("Metroid (Japan)", "hashJ", "metroid");

  VariantGroupService service(m_library, m_settings);
  const auto groupId = service.createGroupFrom({usa, japan});
  ASSERT_TRUE(groupId.has_value());

  // Something with the USA entry's title arrives, and must not disturb the hand-made set
  const auto europe = makeTitled("Zelda (Europe)", "hashE", "zelda");

  EXPECT_EQ(m_repo.getEntry(usa)->variantGroupId, groupId);
  EXPECT_FALSE(m_repo.getEntry(europe)->variantGroupId.has_value());
}

// Titled once at creation. Pinning a different release does not rename the set
TEST_F(VariantGroupServiceTest, PinningADifferentPrimaryDoesNotRenameTheGroup) {
  const auto groupId = makeGroup("Zelda");
  makeMember(groupId, "Zelda (USA)", "hashU", {"US"});
  const auto japan = makeMember(groupId, "Zelda (Japan)", "hashJ", {"JP"});

  VariantGroupService service(m_library, m_settings);
  ASSERT_TRUE(service.pinPrimary(groupId, japan));

  EXPECT_EQ(m_repo.getVariantGroup(groupId)->title, "Zelda");
}

// Discs of one game are not regional variants of each other. Nothing pinned this before, and
// the forms below are exactly the ones whose disc tag used to parse as nothing while still
// being stripped from the title, leaving every disc sharing a normalized title
TEST_F(VariantGroupServiceTest, DiscsOfOneGameAreNotVariants) {
  const auto discOne = makeUngroupedFromFile("Final Fantasy VII (USA) (Disc 1).cue");
  const auto discTwo = makeUngroupedFromFile("Final Fantasy VII (USA) (Disc 2).cue");
  const auto discThree = makeUngroupedFromFile("Final Fantasy VII (USA) (CD3).cue");

  VariantGroupService service(m_library, m_settings);

  EXPECT_FALSE(service.autoGroupByTitle(discTwo));
  EXPECT_FALSE(service.autoGroupByTitle(discThree));

  EXPECT_TRUE(m_repo.getVariantGroups().empty());
  EXPECT_FALSE(m_repo.getEntry(discOne)->variantGroupId.has_value());
  EXPECT_FALSE(m_repo.getEntry(discTwo)->variantGroupId.has_value());
}

} // namespace firelight::library

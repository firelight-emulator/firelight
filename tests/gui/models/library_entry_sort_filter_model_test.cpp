// TODO: NEEDS REVIEW
#include "gui/models/library_entry_sort_filter_model.hpp"

#include "app/library/gui/entry_list_model.hpp"
#include "app/library/gui/playlist_item_model.hpp"
#include "app/service_accessor.hpp"
#include "sqlite_achievement_repository.hpp"

#include <firelight/achievement_service.hpp>
#include <firelight/activity/sqlite_activity_log.hpp>
#include <firelight/library/disc_set_service.hpp>
#include <firelight/library/folder_info.hpp>
#include <firelight/library/library_ingest_service.hpp>
#include <firelight/library/sqlite_user_library.hpp>
#include <firelight/library/user_library_service.hpp>
#include <firelight/platforms/platform_service.hpp>
#include <firelight/settings/sqlite_settings_repository.hpp>

#include <QDir>
#include <QEventLoop>
#include <QSignalSpy>
#include <QTimer>
#include <algorithm>
#include <gtest/gtest.h>
#include <library/variant_group_service.hpp>

// Verifies what the sorted/filtered view puts in front of QML: which rows survive
// each filter, what order they come out in, and that a property change is picked
// up. The apply is coalesced to the end of the event loop turn, so each test
// pumps before asserting
namespace firelight::gui {
namespace {

void pump() {
  QEventLoop loop;
  QTimer::singleShot(50, &loop, &QEventLoop::quit);
  loop.exec();
}

library::Entry makeEntry(const std::string &name, const std::string &hash, const unsigned platformId,
                         const bool isFavorite = false) {
  library::Entry entry;
  entry.displayName = name;
  entry.contentHash = hash;
  entry.platformId = platformId;
  entry.favorite = isFavorite;
  return entry;
}

} // namespace

class LibraryEntrySortFilterModelTest : public testing::Test {
protected:
  library::SqliteUserLibraryRepository m_repo{":memory:"};
  library::DiscSetService m_discSets{m_repo, ""};
  library::LibraryIngestService m_ingest{m_repo, m_discSets};
  library::UserLibraryService m_service{m_repo, (QDir::tempPath() + "/fl_lesfm_test").toStdString()};
  activity::SqliteActivityLog m_activityLog{":memory:"};
  platforms::PlatformService m_platformService;
  achievements::SqliteAchievementRepository m_achievementRepo{":memory:"};
  achievements::AchievementService m_achievementService{m_achievementRepo};
  settings::SqliteSettingsRepository m_settingsRepo{":memory:"};
  settings::SettingsService m_settingsService{m_settingsRepo};
  library::VariantGroupService m_variantGroups{m_service, m_settingsService};

  void SetUp() override {
    auto charlie = makeEntry("Charlie", "hashC", 3);
    auto alpha = makeEntry("alpha", "hashA", 3, true);
    auto bravo = makeEntry("Bravo", "hashB", 7);

    ASSERT_TRUE(m_repo.createEntry(charlie));
    ASSERT_TRUE(m_repo.createEntry(alpha));
    ASSERT_TRUE(m_repo.createEntry(bravo));

    // A game is only playable if it has something to launch, so the fixtures get files the way a
    // scan would give them one
    for (const auto &hash : {"hashC", "hashA", "hashB"}) {
      ASSERT_TRUE(catalogue(hash));
    }

    m_source.emplace(m_service, m_activityLog, m_platformService, m_achievementService, m_settingsService);
    m_model.setSourceModel(&m_source.value());

    ServiceAccessor::setLibraryService(&m_service);
    m_folders.emplace();
    m_model.setFolderModel(&m_folders.value());
  }

  void TearDown() override { ServiceAccessor::setLibraryService(nullptr); }

  std::optional<library::EntryListModel> m_source;
  std::optional<LibraryFolderListModel> m_folders;
  LibraryEntrySortFilterModel m_model;

  // A hand-picked collection the folder model knows about
  int makeManualCollection(const QString &name) { return m_folders->createFolder(name, -1); }

  // A criteria-driven collection the folder model knows about
  int makeSmartCollection(const QString &name, const QString &filterJson) {
    return m_folders->addSmartFolder(name, filterJson);
  }

  bool catalogue(const std::string &hash) {
    library::ContentFile file{
        .m_fileSizeBytes = 1024, .m_filePath = "/roms/" + hash + ".gb", .m_platformId = 7, .m_contentHash = hash};

    return m_repo.create(file);
  }

  // Every copy of Bravo goes away, which is how a game becomes unavailable
  bool takeBravosFilesAway() {
    for (const auto &file : m_repo.getContentFilesWithContentHash("hashB")) {
      if (!m_repo.markContentFileMissing(file.m_id)) {
        return false;
      }
    }

    return true;
  }

  // Puts the two Zelda entries in a group, and returns whether it was made
  bool groupUsaAndJapan() {
    auto usa = makeEntry("Zelda (USA)", "hashU", 3);
    auto japan = makeEntry("Zelda (Japan)", "hashJ", 3);

    if (!m_repo.createEntry(usa) || !m_repo.createEntry(japan)) {
      return false;
    }

    usa.metadata.regions = {"US"};
    japan.metadata.regions = {"JP"};
    m_repo.update(usa);
    m_repo.update(japan);
    pump();

    return m_variantGroups.createGroupFrom({usa.id, japan.id}).has_value();
  }

  // Makes a hand-picked collection and returns its id
  int makeCollection(const std::string &name) {
    library::FolderInfo folder;
    folder.displayName = name;

    return m_repo.create(folder) ? folder.id : -1;
  }

  // The id behind a visible row, so a test can name a game rather than an id
  int entryIdNamed(const QString &name) {
    for (auto row = 0; row < m_model.rowCount(QModelIndex()); ++row) {
      if (m_model.data(m_model.index(row, 0), library::EntryListModel::DisplayName).toString() == name) {
        return m_model.getEntryIdAt(row);
      }
    }

    return -1;
  }

  std::vector<QString> names() {
    std::vector<QString> result;

    for (auto row = 0; row < m_model.rowCount(QModelIndex()); ++row) {
      result.push_back(m_model.data(m_model.index(row, 0), library::EntryListModel::DisplayName).toString());
    }

    return result;
  }
};

// A game whose files went away is the one somebody most needs to see, because the row is the only
// thing that can tell them where it was. Dropping it here is what made the tile disappear
TEST_F(LibraryEntrySortFilterModelTest, AGameWithNoReachableFilesStillShows) {
  ASSERT_TRUE(takeBravosFilesAway());
  pump();

  EXPECT_EQ(m_model.getCount(), 3) << "the row vanished instead of staying to be badged";

  const auto shown = names();
  EXPECT_NE(std::ranges::find(shown, QStringLiteral("Bravo")), shown.end());
}

// The user's own control for not wanting to look at them, which is off by default
TEST_F(LibraryEntrySortFilterModelTest, HideUnavailableIsWhatTakesItAway) {
  ASSERT_TRUE(takeBravosFilesAway());
  pump();
  ASSERT_EQ(m_model.getCount(), 3);

  m_model.getFilter()->setPlayable(LibraryFilter::Yes);
  m_model.applyFilters();
  pump();

  EXPECT_EQ(m_model.getCount(), 2);
  EXPECT_EQ(names(), (std::vector<QString>{"alpha", "Charlie"}));
}

// With nothing filtered, every entry shows, ordered by name and case-insensitively
TEST_F(LibraryEntrySortFilterModelTest, SortsByDisplayNameIgnoringCase) {
  EXPECT_EQ(m_model.getCount(), 3);
  EXPECT_EQ(names(), (std::vector<QString>{"alpha", "Bravo", "Charlie"}));
}

// Reversing the direction reverses the rows
TEST_F(LibraryEntrySortFilterModelTest, DescendingReversesOrder) {
  m_model.setSortAscending(false);
  m_model.applyFilters();

  EXPECT_EQ(names(), (std::vector<QString>{"Charlie", "Bravo", "alpha"}));
}

// The text filter matches anywhere in the name, regardless of case
TEST_F(LibraryEntrySortFilterModelTest, FilterTextMatchesSubstring) {
  m_model.getFilter()->setNameContains("RAV");
  m_model.applyFilters();

  EXPECT_EQ(m_model.getCount(), 1);
  EXPECT_EQ(names(), (std::vector<QString>{"Bravo"}));
}

// Only entries on a listed platform survive; an empty list accepts every platform
TEST_F(LibraryEntrySortFilterModelTest, PlatformIdsRestrictRows) {
  m_model.getFilter()->setPlatformIds({7});
  m_model.applyFilters();

  EXPECT_EQ(names(), (std::vector<QString>{"Bravo"}));

  m_model.getFilter()->setPlatformIds({});
  m_model.applyFilters();

  EXPECT_EQ(m_model.getCount(), 3);
}

// createEntry does not persist the flag, so it is set through the model the way
// the heart button does
TEST_F(LibraryEntrySortFilterModelTest, FavoritesOnlyRestrictsRows) {
  m_source->setEntryFavorite(m_model.getEntryIdAt(0), true);
  m_model.getFilter()->setFavorite(LibraryFilter::Yes);
  m_model.applyFilters();

  EXPECT_EQ(names(), (std::vector<QString>{"alpha"}));
}

// Filters combine rather than replacing each other
TEST_F(LibraryEntrySortFilterModelTest, FiltersCompose) {
  m_model.getFilter()->setPlatformIds({3});
  m_model.getFilter()->setNameContains("a");
  m_model.applyFilters();

  EXPECT_EQ(names(), (std::vector<QString>{"alpha", "Charlie"}));
}

// Setting properties stages them; nothing changes until the caller commits, so
// changing several costs one pass rather than one each
TEST_F(LibraryEntrySortFilterModelTest, RebuildsOnlyWhenApplied) {
  auto touched = 0;
  QObject::connect(&m_model, &QAbstractItemModel::modelReset, [&touched] { ++touched; });
  QObject::connect(&m_model, &QAbstractItemModel::rowsRemoved, [&touched] { ++touched; });
  QObject::connect(&m_model, &QAbstractItemModel::layoutChanged, [&touched] { ++touched; });

  m_model.getFilter()->setNameContains("a");
  m_model.setSortAscending(false);
  m_model.getFilter()->setPlatformIds({3});

  EXPECT_EQ(touched, 0);
  EXPECT_EQ(m_model.getCount(), 3);

  m_model.applyFilters();

  EXPECT_GT(touched, 0);
  EXPECT_EQ(names(), (std::vector<QString>{"Charlie", "alpha"}));
}

// A row added to the library after the last pass shows up on its own
TEST_F(LibraryEntrySortFilterModelTest, PicksUpSourceInsertions) {
  auto delta = makeEntry("Delta", "hashD", 3);
  ASSERT_TRUE(m_repo.createEntry(delta));
  pump();

  EXPECT_EQ(m_model.getCount(), 4);
  EXPECT_EQ(names(), (std::vector<QString>{"alpha", "Bravo", "Charlie", "Delta"}));
}

// The id at a visible row is the id of the entry shown there, and out-of-range asks answer -1
TEST_F(LibraryEntrySortFilterModelTest, EntryIdAtFollowsVisibleOrder) {
  const auto firstId = m_model.data(m_model.index(0, 0), library::EntryListModel::Id).toInt();

  EXPECT_EQ(m_model.getEntryIdAt(0), firstId);
  EXPECT_EQ(m_model.getEntryIdAt(-1), -1);
  EXPECT_EQ(m_model.getEntryIdAt(m_model.getCount()), -1);
}

// A filter narrowing the list keeps the rows that survived, so the view keeps its
// delegates and its scroll position instead of rebuilding from scratch
TEST_F(LibraryEntrySortFilterModelTest, FilteringRemovesRowsWithoutResetting) {
  auto resets = 0;
  auto removes = 0;
  QObject::connect(&m_model, &QAbstractItemModel::modelReset, [&resets] { ++resets; });
  QObject::connect(&m_model, &QAbstractItemModel::rowsRemoved, [&removes] { ++removes; });

  // Drops the rows either side of Bravo, so removals come in two runs
  m_model.getFilter()->setNameContains("RAV");
  m_model.applyFilters();

  EXPECT_EQ(resets, 0);
  EXPECT_EQ(removes, 2);
  EXPECT_EQ(names(), (std::vector<QString>{"Bravo"}));
}

TEST_F(LibraryEntrySortFilterModelTest, WideningAFilterInsertsWithoutResetting) {
  m_model.getFilter()->setNameContains("RAV");
  m_model.applyFilters();
  ASSERT_EQ(m_model.getCount(), 1);

  auto resets = 0;
  auto inserts = 0;
  QObject::connect(&m_model, &QAbstractItemModel::modelReset, [&resets] { ++resets; });
  QObject::connect(&m_model, &QAbstractItemModel::rowsInserted, [&inserts] { ++inserts; });

  m_model.getFilter()->setNameContains("");
  m_model.applyFilters();

  EXPECT_EQ(resets, 0);
  EXPECT_GT(inserts, 0);
  EXPECT_EQ(names(), (std::vector<QString>{"alpha", "Bravo", "Charlie"}));
}

// A reorder is described as a layout change rather than a reset, which is what lets the
// view move the delegates it already has instead of building new ones
TEST_F(LibraryEntrySortFilterModelTest, ReorderingIsALayoutChangeNotAReset) {
  auto resets = 0;
  auto layoutChanges = 0;
  QObject::connect(&m_model, &QAbstractItemModel::modelReset, [&resets] { ++resets; });
  QObject::connect(&m_model, &QAbstractItemModel::layoutChanged, [&layoutChanges] { ++layoutChanges; });

  m_model.setSortAscending(false);
  m_model.applyFilters();

  EXPECT_EQ(resets, 0);
  EXPECT_GT(layoutChanges, 0);
  EXPECT_EQ(names(), (std::vector<QString>{"Charlie", "Bravo", "alpha"}));
}

// Applying with nothing changed must not disturb the view at all
TEST_F(LibraryEntrySortFilterModelTest, AnUnchangedApplyEmitsNothing) {
  auto touched = 0;
  QObject::connect(&m_model, &QAbstractItemModel::modelReset, [&touched] { ++touched; });
  QObject::connect(&m_model, &QAbstractItemModel::rowsRemoved, [&touched] { ++touched; });
  QObject::connect(&m_model, &QAbstractItemModel::rowsInserted, [&touched] { ++touched; });

  m_model.applyFilters();
  m_model.applyFilters();

  EXPECT_EQ(touched, 0);
}

// Art arriving does not move a row, so nothing about the order changes -- but the
// view still has to be told, or the tile keeps showing what it had
TEST_F(LibraryEntrySortFilterModelTest, ASourceChangeReachesTheViewWithoutMovingRows) {
  auto changed = 0;
  auto structural = 0;
  QObject::connect(&m_model, &QAbstractItemModel::dataChanged, [&changed] { ++changed; });
  QObject::connect(&m_model, &QAbstractItemModel::modelReset, [&structural] { ++structural; });
  QObject::connect(&m_model, &QAbstractItemModel::rowsRemoved, [&structural] { ++structural; });
  QObject::connect(&m_model, &QAbstractItemModel::rowsInserted, [&structural] { ++structural; });

  const auto entryId = m_model.getEntryIdAt(0);
  m_source->setEntryFavorite(entryId, true);
  pump();

  EXPECT_GT(changed, 0);
  EXPECT_EQ(structural, 0);
  EXPECT_EQ(m_model.getCount(), 3);
}

// TODO
// Grouping is unwired, so a group in the database changes nothing about what the grid shows
TEST_F(LibraryEntrySortFilterModelTest, AGroupedEntryStillGetsItsOwnRow) {
  ASSERT_TRUE(groupUsaAndJapan());
  pump();

  EXPECT_EQ(m_model.getCount(), 5) << "a grouped release lost its row";

  const auto shown = names();
  EXPECT_EQ(std::count(shown.begin(), shown.end(), QStringLiteral("Zelda (USA)")), 1);
  EXPECT_EQ(std::count(shown.begin(), shown.end(), QStringLiteral("Zelda (Japan)")), 1);
}

// TODO
// The chips count the rows the grid shows
TEST_F(LibraryEntrySortFilterModelTest, PlatformCountsFollowTheRows) {
  ASSERT_TRUE(groupUsaAndJapan());
  pump();

  EXPECT_EQ(m_model.getCountByPlatform().value("3").toInt(), 4);
}

// TODO
// A getter that reads what a pass is showing while its setter writes what the next pass will show
// means a write followed by a read hands back the old value
TEST_F(LibraryEntrySortFilterModelTest, EveryGetterReadsTheStagedValue) {
  m_model.getFilter()->setNameContains(QStringLiteral("zel"));
  m_model.getFilter()->setFavorite(LibraryFilter::Yes);
  m_model.getFilter()->setPlayable(LibraryFilter::Yes);
  m_model.getFilter()->setPlatformIds({3});
  m_model.setSortRole(LibraryEntrySortFilterModel::LastPlayedAt);
  m_model.setSortAscending(false);

  EXPECT_EQ(m_model.getFilter()->getNameContains(), QStringLiteral("zel"));
  EXPECT_TRUE((m_model.getFilter()->getFavorite() == LibraryFilter::Yes));
  EXPECT_TRUE((m_model.getFilter()->getPlayable() == LibraryFilter::Yes));
  EXPECT_EQ(m_model.getFilter()->getPlatformIds(), QVariantList{3});
  EXPECT_EQ(m_model.getSortRole(), LibraryEntrySortFilterModel::LastPlayedAt)
      << "sortRole read back the committed value while its siblings read the staged one";
  EXPECT_FALSE(m_model.isSortAscending());
  EXPECT_EQ(m_model.getSortDisplayName(), QStringLiteral("Last Played"));
}

// TODO
// Clearing has to clear everything and take effect, or the button leaves filters on that the
// controls say are off
TEST_F(LibraryEntrySortFilterModelTest, ClearingFiltersClearsEveryOne) {
  ASSERT_TRUE(takeBravosFilesAway());
  pump();

  m_model.getFilter()->setNameContains(QStringLiteral("zzz"));
  m_model.getFilter()->setFavorite(LibraryFilter::Yes);
  m_model.getFilter()->setPlayable(LibraryFilter::Yes);
  m_model.getFilter()->setPlatformIds({99});
  m_model.applyFilters();
  pump();
  ASSERT_EQ(m_model.getCount(), 0);

  m_model.clearAllFilters();
  pump();

  EXPECT_EQ(m_model.getCount(), 3) << "clearing left a filter applied";
  EXPECT_TRUE(m_model.getFilter()->getNameContains().isEmpty());
  EXPECT_FALSE((m_model.getFilter()->getFavorite() == LibraryFilter::Yes));
  EXPECT_FALSE((m_model.getFilter()->getPlayable() == LibraryFilter::Yes)) << "hideUnavailable was never cleared";
  EXPECT_TRUE(m_model.getFilter()->getPlatformIds().isEmpty());
  EXPECT_FALSE(m_model.anyFiltersActive());
}

// TODO
// Seven dimensions the criteria always had and the grid could never reach. Three of them, to say
// the reach is real rather than the plumbing merely compiling
TEST_F(LibraryEntrySortFilterModelTest, AGenreFilterReachesTheRows) {
  auto entry = m_repo.getEntryWithContentHash("hashA");
  ASSERT_TRUE(entry.has_value());

  GameMetadata metadata;
  metadata.genres = {"Puzzle"};
  ASSERT_TRUE(m_repo.applyEntryMetadata(entry->id, metadata, {metadata_fields::GENRES}, false));
  m_source->reset();
  pump();

  m_model.getFilter()->setGenres({QStringLiteral("puzzle")});
  m_model.applyFilters();
  pump();

  EXPECT_EQ(names(), (std::vector<QString>{"alpha"}));
}

TEST_F(LibraryEntrySortFilterModelTest, AYearRangeReachesTheRows) {
  auto entry = m_repo.getEntryWithContentHash("hashC");
  ASSERT_TRUE(entry.has_value());

  GameMetadata metadata;
  metadata.releaseYear = 1994;
  ASSERT_TRUE(m_repo.applyEntryMetadata(entry->id, metadata, {metadata_fields::RELEASE_YEAR}, false));
  m_source->reset();
  pump();

  m_model.getFilter()->setYearMin(1990);
  m_model.getFilter()->setYearMax(1999);
  m_model.applyFilters();
  pump();

  EXPECT_EQ(names(), (std::vector<QString>{"Charlie"}));
}

TEST_F(LibraryEntrySortFilterModelTest, ADeveloperFilterReachesTheRows) {
  auto entry = m_repo.getEntryWithContentHash("hashB");
  ASSERT_TRUE(entry.has_value());

  GameMetadata metadata;
  metadata.developer = "Konami";
  ASSERT_TRUE(m_repo.applyEntryMetadata(entry->id, metadata, {metadata_fields::DEVELOPER}, false));
  m_source->reset();
  pump();

  m_model.getFilter()->setDeveloper(QStringLiteral("konami"));
  m_model.applyFilters();
  pump();

  EXPECT_EQ(names(), (std::vector<QString>{"Bravo"}));
}

// TODO
// A rolling window resolved per row moves partway through a pass, so two rows the same age can
// land on opposite sides of it
TEST_F(LibraryEntrySortFilterModelTest, TheClockIsStampedOncePerPass) {
  const int64_t now = 1'700'000'000'000;
  const int64_t day = 86'400'000;

  activity::PlaySession recent;
  recent.contentHash = "hashA";
  recent.startedAt = static_cast<uint64_t>(now - 2 * day);
  recent.endedAt = static_cast<uint64_t>(now - 2 * day + 1000);
  recent.unpausedDurationMillis = 1000;
  ASSERT_TRUE(m_activityLog.createPlaySession(recent));

  m_source->reset();
  pump();

  m_model.getFilter()->setPlayedWithinDays(7);
  m_model.applyFilters(now);
  pump();
  EXPECT_EQ(names(), (std::vector<QString>{"alpha"}));

  m_model.applyFilters(now + 30 * day);
  pump();
  EXPECT_TRUE(names().empty()) << "the window was resolved against a clock the caller did not set";
}

// A collection narrows by membership, which no criterion can express
TEST_F(LibraryEntrySortFilterModelTest, AScopeNarrowsToTheCollectionsMembers) {
  const auto collection = makeCollection("Favourites");
  ASSERT_NE(collection, -1);
  m_source->addEntryToFolder(entryIdNamed("Bravo"), collection);

  m_model.setScopeFolderId(collection);
  m_model.applyFilters();

  EXPECT_EQ(names(), (std::vector<QString>{"Bravo"}));
}

// The scope is staged the way every other value is, so it cannot move a pass that is already running
TEST_F(LibraryEntrySortFilterModelTest, TheScopeTakesEffectOnlyWhenApplied) {
  const auto collection = makeCollection("Favourites");
  ASSERT_NE(collection, -1);
  m_source->addEntryToFolder(entryIdNamed("Bravo"), collection);

  m_model.setScopeFolderId(collection);

  EXPECT_EQ(m_model.getCount(), 3);
  EXPECT_TRUE(m_model.isPending());

  m_model.applyFilters();

  EXPECT_EQ(m_model.getCount(), 1);
  EXPECT_FALSE(m_model.isPending());
}

// Standing in a collection is not something the user filtered, so the filter-active affordances stay
// off and the empty view reads as an empty collection rather than as filters matching nothing
TEST_F(LibraryEntrySortFilterModelTest, AScopeIsNotAFilterTheUserSet) {
  const auto collection = makeCollection("Favourites");
  ASSERT_NE(collection, -1);

  m_model.setScopeFolderId(collection);
  m_model.applyFilters();

  EXPECT_FALSE(m_model.anyFiltersActive());
}

// Clearing the filters must not take the view out of the collection it is showing
TEST_F(LibraryEntrySortFilterModelTest, ClearingFiltersLeavesTheScope) {
  const auto collection = makeCollection("Favourites");
  ASSERT_NE(collection, -1);
  m_source->addEntryToFolder(entryIdNamed("Bravo"), collection);
  m_source->addEntryToFolder(entryIdNamed("Charlie"), collection);

  m_model.setScopeFolderId(collection);
  m_model.getFilter()->setNameContains("Bravo");
  m_model.applyFilters();

  ASSERT_EQ(names(), (std::vector<QString>{"Bravo"}));

  m_model.clearAllFilters();

  EXPECT_EQ(m_model.getScopeFolderId(), collection);
  EXPECT_EQ(names(), (std::vector<QString>{"Bravo", "Charlie"}));
}

// TODO
// Every consumer of this table reads the same three keys — the sort menu and the collection
// dialog both render "text" and store "role". Renaming one silently renders blank rows
TEST_F(LibraryEntrySortFilterModelTest, EverySortOptionCarriesTextRoleAndValue) {
  const auto options = LibraryEntrySortFilterModel::getSortOptions();

  ASSERT_FALSE(options.isEmpty());

  for (const auto &option : options) {
    const auto fields = option.toMap();

    EXPECT_TRUE(fields.contains("text")) << "a sort with no text renders as a blank row";
    EXPECT_TRUE(fields.contains("role"));
    EXPECT_TRUE(fields.contains("value"));
    EXPECT_FALSE(fields.value("text").toString().isEmpty());
    EXPECT_FALSE(fields.value("role").toString().isEmpty());
  }
}

// TODO
// Taking a game out of the collection you are looking at should not pull the tile out from under
// the cursor. The pass that is on screen keeps showing what it was built from
TEST_F(LibraryEntrySortFilterModelTest, ARowTakenOutOfTheCollectionStaysUntilTheNextPass) {
  const auto collection = makeCollection("Favourites");
  ASSERT_NE(collection, -1);

  const auto bravo = entryIdNamed("Bravo");
  const auto charlie = entryIdNamed("Charlie");
  m_source->addEntryToFolder(bravo, collection);
  m_source->addEntryToFolder(charlie, collection);

  m_model.setScopeFolderId(collection);
  m_model.applyFilters();
  ASSERT_EQ(names(), (std::vector<QString>{"Bravo", "Charlie"}));

  m_source->removeEntryFromFolder(bravo, collection);

  EXPECT_EQ(names(), (std::vector<QString>{"Bravo", "Charlie"})) << "the row vanished under the cursor";

  m_model.applyFilters();

  EXPECT_EQ(names(), (std::vector<QString>{"Charlie"}));
}

// TODO
// The row that is on its way out says so, so a delegate can show it as removed rather than looking
// like the press did nothing
TEST_F(LibraryEntrySortFilterModelTest, ARowOnItsWayOutOfTheCollectionSaysSo) {
  const auto collection = makeCollection("Favourites");
  ASSERT_NE(collection, -1);

  const auto bravo = entryIdNamed("Bravo");
  const auto charlie = entryIdNamed("Charlie");
  m_source->addEntryToFolder(bravo, collection);
  m_source->addEntryToFolder(charlie, collection);

  m_model.setScopeFolderId(collection);
  m_model.applyFilters();

  m_source->removeEntryFromFolder(bravo, collection);

  for (auto row = 0; row < m_model.rowCount(QModelIndex()); ++row) {
    const auto index = m_model.index(row, 0);
    const auto removed = m_model.data(index, LibraryEntrySortFilterModel::RemovedFromScope).toBool();

    EXPECT_EQ(removed, m_model.getEntryIdAt(row) == bravo);
  }
}

// TODO
// Nothing is on its way out when the view is not scoped to a collection
TEST_F(LibraryEntrySortFilterModelTest, WithNoCollectionNothingReadsAsRemoved) {
  m_model.applyFilters();

  for (auto row = 0; row < m_model.rowCount(QModelIndex()); ++row) {
    EXPECT_FALSE(m_model.data(m_model.index(row, 0), LibraryEntrySortFilterModel::RemovedFromScope).toBool());
  }
}

// Opening a hand-picked collection narrows by membership, which is the only axis it has
TEST_F(LibraryEntrySortFilterModelTest, OpeningAManualCollectionNarrowsToItsMembers) {
  const auto collection = makeManualCollection("Favourites");
  ASSERT_NE(collection, -1);
  m_source->addEntryToFolder(entryIdNamed("Bravo"), collection);

  m_model.setOpenFolderId(collection);

  EXPECT_FALSE(m_model.isOpenFolderSmart());
  EXPECT_EQ(m_model.getScopeFolderId(), collection);
  EXPECT_EQ(names(), (std::vector<QString>{"Bravo"}));
}

// A smart collection carries criteria instead of members, so membership must stay out of it
TEST_F(LibraryEntrySortFilterModelTest, OpeningASmartCollectionAdoptsItsCriteriaAndNotMembership) {
  const auto collection = makeSmartCollection("Platform 7", R"({"platformIds":[7]})");
  ASSERT_NE(collection, -1);

  m_model.setOpenFolderId(collection);

  EXPECT_TRUE(m_model.isOpenFolderSmart());
  EXPECT_EQ(m_model.getScopeFolderId(), -1);
  EXPECT_EQ(names(), (std::vector<QString>{"Bravo"}));
}

// Opening a collection is something the app did, so the view has no reason to announce a refinement
TEST_F(LibraryEntrySortFilterModelTest, OpeningACollectionIsNotARefinement) {
  const auto collection = makeSmartCollection("Platform 7", R"({"platformIds":[7]})");
  ASSERT_NE(collection, -1);

  const QSignalSpy spy(&m_model, &LibraryEntrySortFilterModel::refinementChanged);
  m_model.setOpenFolderId(collection);

  EXPECT_EQ(spy.count(), 0);
}

// A refinement the user made still announces itself, or nothing would ever redraw
TEST_F(LibraryEntrySortFilterModelTest, ARefinementInsideACollectionStillAnnouncesItself) {
  const auto collection = makeManualCollection("Favourites");
  ASSERT_NE(collection, -1);
  m_model.setOpenFolderId(collection);

  const QSignalSpy spy(&m_model, &LibraryEntrySortFilterModel::refinementChanged);
  m_model.getFilter()->setNameContains("Bravo");

  EXPECT_GT(spy.count(), 0);
}

// A collection remembers how it was last sorted
TEST_F(LibraryEntrySortFilterModelTest, OpeningACollectionAdoptsItsPinnedSort) {
  const auto collection = makeManualCollection("Favourites");
  ASSERT_NE(collection, -1);
  ASSERT_TRUE(m_folders->setFolderSort(collection, "releaseYear", false));

  m_model.setOpenFolderId(collection);

  EXPECT_TRUE(m_model.isSortPinnedToOpenFolder());
  EXPECT_EQ(m_model.getSortRole(), LibraryEntrySortFilterModel::ReleaseYear);
  EXPECT_FALSE(m_model.isSortAscending());
}

// A collection with no sort of its own uses the one everything else uses
TEST_F(LibraryEntrySortFilterModelTest, ACollectionWithNoPinnedSortUsesTheDefault) {
  const auto collection = makeManualCollection("Favourites");
  ASSERT_NE(collection, -1);

  m_model.setOpenFolderId(collection);

  EXPECT_FALSE(m_model.isSortPinnedToOpenFolder());
  EXPECT_EQ(m_model.getSortRole(), LibraryEntrySortFilterModel::DisplayName);
  EXPECT_TRUE(m_model.isSortAscending());
}

// Sorting inside a collection is what makes the pin, so it is there next time
TEST_F(LibraryEntrySortFilterModelTest, SortingInsideACollectionPinsItToTheCollection) {
  const auto collection = makeManualCollection("Favourites");
  ASSERT_NE(collection, -1);
  m_model.setOpenFolderId(collection);

  m_model.setSortRole(LibraryEntrySortFilterModel::ReleaseYear);

  const auto *folder = m_folders->findFolder(collection);
  ASSERT_NE(folder, nullptr);
  EXPECT_EQ(folder->sortRole, "releaseYear");
  EXPECT_TRUE(m_model.isSortPinnedToOpenFolder());
}

// Clearing the pin is the only way back out of one
TEST_F(LibraryEntrySortFilterModelTest, ClearingThePinHandsTheCollectionBackToTheDefaultSort) {
  const auto collection = makeManualCollection("Favourites");
  ASSERT_NE(collection, -1);
  ASSERT_TRUE(m_folders->setFolderSort(collection, "releaseYear", false));
  m_model.setOpenFolderId(collection);

  m_model.setSortPinnedToOpenFolder(false);

  EXPECT_FALSE(m_model.isSortPinnedToOpenFolder());
  EXPECT_EQ(m_model.getSortRole(), LibraryEntrySortFilterModel::DisplayName);
  EXPECT_TRUE(m_model.isSortAscending());
}

// Leaving a collection puts the whole library back, criteria and all
TEST_F(LibraryEntrySortFilterModelTest, LeavingACollectionRestoresTheWholeLibrary) {
  const auto collection = makeSmartCollection("Platform 7", R"({"platformIds":[7]})");
  ASSERT_NE(collection, -1);
  m_model.setOpenFolderId(collection);
  ASSERT_EQ(m_model.getCount(), 1);

  m_model.setOpenFolderId(-1);

  EXPECT_FALSE(m_model.anyFiltersActive());
  EXPECT_EQ(m_model.getCount(), 3);
}

// Moving between collections shows the one just opened, not the one before it
TEST_F(LibraryEntrySortFilterModelTest, SwitchingCollectionsShowsTheOneJustOpened) {
  const auto alpha = makeManualCollection("Alpha");
  const auto beta = makeManualCollection("Beta");
  ASSERT_NE(alpha, -1);
  ASSERT_NE(beta, -1);
  m_source->addEntryToFolder(entryIdNamed("Bravo"), alpha);
  m_source->addEntryToFolder(entryIdNamed("Charlie"), beta);

  m_model.setOpenFolderId(alpha);
  ASSERT_EQ(names(), (std::vector<QString>{"Bravo"}));

  m_model.setOpenFolderId(-1);
  m_model.setOpenFolderId(beta);

  EXPECT_EQ(m_model.getOpenFolderId(), beta);
  EXPECT_EQ(names(), (std::vector<QString>{"Charlie"}));
}

// Going straight from one collection to another, without passing through the library
TEST_F(LibraryEntrySortFilterModelTest, SwitchingCollectionsDirectlyShowsTheOneJustOpened) {
  const auto alpha = makeManualCollection("Alpha");
  const auto beta = makeSmartCollection("Platform 7", R"({"platformIds":[7]})");
  ASSERT_NE(alpha, -1);
  ASSERT_NE(beta, -1);
  m_source->addEntryToFolder(entryIdNamed("Charlie"), alpha);

  m_model.setOpenFolderId(alpha);
  ASSERT_EQ(names(), (std::vector<QString>{"Charlie"}));

  m_model.setOpenFolderId(beta);

  EXPECT_TRUE(m_model.isOpenFolderSmart());
  EXPECT_EQ(m_model.getScopeFolderId(), -1);
  EXPECT_EQ(names(), (std::vector<QString>{"Bravo"}));
}

// Resetting puts the collection's saved criteria back without the view having to re-assign the id,
// which is what used to leave the collection stuck on whichever one was open last
TEST_F(LibraryEntrySortFilterModelTest, ResetToSavedPutsTheCollectionsCriteriaBack) {
  const auto collection = makeSmartCollection("Platform 7", R"({"platformIds":[7]})");
  ASSERT_NE(collection, -1);
  m_model.setOpenFolderId(collection);
  ASSERT_EQ(names(), (std::vector<QString>{"Bravo"}));

  m_model.getFilter()->setNameContains("nothing matches this");
  m_model.applyFilters();
  ASSERT_EQ(m_model.getCount(), 0);

  m_model.resetToSaved();

  EXPECT_EQ(m_model.getOpenFolderId(), collection);
  EXPECT_EQ(names(), (std::vector<QString>{"Bravo"}));
}

// Resetting must not move which collection is open, or the next one opened inherits this one
TEST_F(LibraryEntrySortFilterModelTest, ResetToSavedLeavesTheOpenCollectionAlone) {
  const auto alpha = makeSmartCollection("Platform 7", R"({"platformIds":[7]})");
  const auto beta = makeManualCollection("Beta");
  ASSERT_NE(alpha, -1);
  ASSERT_NE(beta, -1);
  m_source->addEntryToFolder(entryIdNamed("Charlie"), beta);

  m_model.setOpenFolderId(alpha);
  m_model.resetToSaved();
  m_model.setOpenFolderId(beta);

  EXPECT_FALSE(m_model.isOpenFolderSmart());
  EXPECT_EQ(names(), (std::vector<QString>{"Charlie"}));
}

// The custom order is the one the user arranged, which starts as the order things were added
TEST_F(LibraryEntrySortFilterModelTest, CustomSortsByTheArrangedOrder) {
  m_model.setSortRole(LibraryEntrySortFilterModel::Custom);
  m_model.setSortAscending(true);
  m_model.applyFilters();

  EXPECT_EQ(names(), (std::vector<QString>{"Charlie", "alpha", "Bravo"}));
}

// Reversing it is the same order backwards, not a different one
TEST_F(LibraryEntrySortFilterModelTest, CustomReversesCleanly) {
  m_model.setSortRole(LibraryEntrySortFilterModel::Custom);
  m_model.setSortAscending(false);
  m_model.applyFilters();

  EXPECT_EQ(names(), (std::vector<QString>{"Bravo", "alpha", "Charlie"}));
}

// The menu offers it, and by the role name the folder table stores
TEST_F(LibraryEntrySortFilterModelTest, CustomIsOfferedAsASortOption) {
  auto found = false;

  for (const auto &option : LibraryEntrySortFilterModel::getSortOptions()) {
    if (option.toMap().value("value").toInt() == LibraryEntrySortFilterModel::Custom) {
      found = true;
      EXPECT_EQ(option.toMap().value("role").toString(), QString("position"));
      EXPECT_EQ(option.toMap().value("text").toString(), QString("Custom"));
    }
  }

  EXPECT_TRUE(found);
}

TEST_F(LibraryEntrySortFilterModelTest, ThePickedSetTakesEffectOnlyWhenApplied) {
  const auto alpha = entryIdNamed("alpha");
  const auto bravo = entryIdNamed("Bravo");

  m_model.setPickedEntryIds({bravo});
  m_model.setPickedMode(LibraryEntrySortFilterModel::OnlyPicked);

  EXPECT_EQ(m_model.getCount(), 3);

  m_model.applyFilters();

  EXPECT_EQ(names(), (std::vector<QString>{"Bravo"}));

  m_model.setPickedEntryIds({alpha, bravo});
  m_model.setPickedMode(LibraryEntrySortFilterModel::HidePicked);

  EXPECT_EQ(names(), (std::vector<QString>{"Bravo"}));

  m_model.applyFilters();

  EXPECT_EQ(names(), (std::vector<QString>{"Charlie"}));
}

TEST_F(LibraryEntrySortFilterModelTest, ThePickedGettersReadTheStagedValue) {
  m_model.setPickedEntryIds(QVariantList{4, 9});
  m_model.setPickedMode(LibraryEntrySortFilterModel::HidePicked);

  EXPECT_EQ(m_model.getPickedEntryIds(), (QVariantList{4, 9}));
  EXPECT_EQ(m_model.getPickedMode(), LibraryEntrySortFilterModel::HidePicked);
}

TEST_F(LibraryEntrySortFilterModelTest, OnlyPickedShowsThePickedRowsWhateverTheCriteria) {
  m_model.setPickedEntryIds({entryIdNamed("Bravo"), entryIdNamed("Charlie")});
  m_model.setPickedMode(LibraryEntrySortFilterModel::OnlyPicked);
  m_model.getFilter()->setNameContains("alpha");
  m_model.getFilter()->setPlatformIds({3});
  m_model.applyFilters();

  EXPECT_EQ(names(), (std::vector<QString>{"Bravo", "Charlie"}));
}

TEST_F(LibraryEntrySortFilterModelTest, OnlyPickedShowsThePickedRowsWhateverTheScope) {
  const auto collection = makeCollection("Favourites");
  ASSERT_NE(collection, -1);
  m_source->addEntryToFolder(entryIdNamed("Bravo"), collection);

  m_model.setPickedEntryIds({entryIdNamed("Charlie")});
  m_model.setPickedMode(LibraryEntrySortFilterModel::OnlyPicked);
  m_model.setScopeFolderId(collection);
  m_model.applyFilters();

  EXPECT_EQ(names(), (std::vector<QString>{"Charlie"}));
}

TEST_F(LibraryEntrySortFilterModelTest, OnlyPickedWithNothingPickedShowsNothing) {
  m_model.setPickedMode(LibraryEntrySortFilterModel::OnlyPicked);
  m_model.applyFilters();

  EXPECT_EQ(m_model.getCount(), 0);
}

TEST_F(LibraryEntrySortFilterModelTest, HidePickedTakesThePickedRowsAway) {
  m_model.setPickedEntryIds({entryIdNamed("Bravo")});
  m_model.setPickedMode(LibraryEntrySortFilterModel::HidePicked);
  m_model.applyFilters();

  EXPECT_EQ(names(), (std::vector<QString>{"alpha", "Charlie"}));
}

TEST_F(LibraryEntrySortFilterModelTest, HidePickedComposesWithTheCriteria) {
  m_model.setPickedEntryIds({entryIdNamed("alpha")});
  m_model.setPickedMode(LibraryEntrySortFilterModel::HidePicked);
  m_model.getFilter()->setPlatformIds({3});
  m_model.applyFilters();

  EXPECT_EQ(names(), (std::vector<QString>{"Charlie"}));
}

TEST_F(LibraryEntrySortFilterModelTest, HidePickedComposesWithTheScope) {
  const auto collection = makeCollection("Favourites");
  ASSERT_NE(collection, -1);
  m_source->addEntryToFolder(entryIdNamed("Bravo"), collection);
  m_source->addEntryToFolder(entryIdNamed("Charlie"), collection);

  m_model.setPickedEntryIds({entryIdNamed("Bravo")});
  m_model.setPickedMode(LibraryEntrySortFilterModel::HidePicked);
  m_model.setScopeFolderId(collection);
  m_model.applyFilters();

  EXPECT_EQ(names(), (std::vector<QString>{"Charlie"}));
}

TEST_F(LibraryEntrySortFilterModelTest, ShowAllIgnoresThePickedSet) {
  m_model.setPickedEntryIds({entryIdNamed("Bravo")});
  m_model.applyFilters();

  EXPECT_EQ(m_model.getPickedMode(), LibraryEntrySortFilterModel::ShowAll);
  EXPECT_EQ(names(), (std::vector<QString>{"alpha", "Bravo", "Charlie"}));

  m_model.getFilter()->setPlatformIds({7});
  m_model.applyFilters();

  EXPECT_EQ(names(), (std::vector<QString>{"Bravo"}));
}

TEST_F(LibraryEntrySortFilterModelTest, StagingThePickedSetAnnouncesOnlyItself) {
  const QSignalSpy refinementSpy(&m_model, &LibraryEntrySortFilterModel::refinementChanged);
  const QSignalSpy filtersOrSortSpy(&m_model, &LibraryEntrySortFilterModel::filtersOrSortChanged);
  const QSignalSpy pickedEntryIdsSpy(&m_model, &LibraryEntrySortFilterModel::pickedEntryIdsChanged);
  const QSignalSpy pickedModeSpy(&m_model, &LibraryEntrySortFilterModel::pickedModeChanged);

  m_model.setPickedEntryIds({entryIdNamed("Bravo")});
  m_model.setPickedMode(LibraryEntrySortFilterModel::HidePicked);

  EXPECT_EQ(refinementSpy.count(), 0);
  EXPECT_EQ(filtersOrSortSpy.count(), 0);
  EXPECT_EQ(pickedEntryIdsSpy.count(), 1);
  EXPECT_EQ(pickedModeSpy.count(), 1);
  EXPECT_FALSE(m_model.isPending());
  EXPECT_FALSE(m_model.anyFiltersActive());
}

TEST_F(LibraryEntrySortFilterModelTest, RestagingTheSamePickedSetAnnouncesNothing) {
  m_model.setPickedEntryIds({entryIdNamed("Bravo")});
  m_model.setPickedMode(LibraryEntrySortFilterModel::HidePicked);

  const QSignalSpy pickedEntryIdsSpy(&m_model, &LibraryEntrySortFilterModel::pickedEntryIdsChanged);
  const QSignalSpy pickedModeSpy(&m_model, &LibraryEntrySortFilterModel::pickedModeChanged);

  m_model.setPickedEntryIds({entryIdNamed("Bravo")});
  m_model.setPickedMode(LibraryEntrySortFilterModel::HidePicked);

  EXPECT_EQ(pickedEntryIdsSpy.count(), 0);
  EXPECT_EQ(pickedModeSpy.count(), 0);
}

} // namespace firelight::gui

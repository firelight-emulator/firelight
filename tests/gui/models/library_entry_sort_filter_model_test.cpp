#include "gui/models/library_entry_sort_filter_model.hpp"

#include "app/library/gui/entry_list_model.hpp"
#include "sqlite_achievement_repository.hpp"

#include <firelight/achievement_service.hpp>
#include <firelight/activity/sqlite_activity_log.hpp>
#include <firelight/library/library_ingest_service.hpp>
#include <firelight/library/sqlite_user_library.hpp>
#include <firelight/library/user_library_service.hpp>
#include <firelight/platforms/platform_service.hpp>
#include <firelight/settings/sqlite_settings_repository.hpp>

#include <QDir>
#include <QEventLoop>
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
  library::LibraryIngestService m_ingest{m_repo};
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

    m_source.emplace(m_service, m_activityLog, m_platformService, m_achievementService, m_variantGroups,
                     m_settingsService);
    m_model.setSourceModel(&m_source.value());
  }

  std::optional<library::EntryListModel> m_source;
  LibraryEntrySortFilterModel m_model;

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

  m_model.setHideUnavailable(true);
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
  m_model.setFilterText("RAV");
  m_model.applyFilters();

  EXPECT_EQ(m_model.getCount(), 1);
  EXPECT_EQ(names(), (std::vector<QString>{"Bravo"}));
}

// Only entries on a listed platform survive; an empty list accepts every platform
TEST_F(LibraryEntrySortFilterModelTest, PlatformIdsRestrictRows) {
  m_model.setPlatformIds({7});
  m_model.applyFilters();

  EXPECT_EQ(names(), (std::vector<QString>{"Bravo"}));

  m_model.setPlatformIds({});
  m_model.applyFilters();

  EXPECT_EQ(m_model.getCount(), 3);
}

// createEntry does not persist the flag, so it is set through the model the way
// the heart button does
TEST_F(LibraryEntrySortFilterModelTest, FavoritesOnlyRestrictsRows) {
  m_source->setEntryFavorite(m_model.getEntryIdAt(0), true);
  m_model.setFavoritesOnly(true);
  m_model.applyFilters();

  EXPECT_EQ(names(), (std::vector<QString>{"alpha"}));
}

// Filters combine rather than replacing each other
TEST_F(LibraryEntrySortFilterModelTest, FiltersCompose) {
  m_model.setPlatformIds({3});
  m_model.setFilterText("a");
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

  m_model.setFilterText("a");
  m_model.setSortAscending(false);
  m_model.setPlatformIds({3});

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
  m_model.setFilterText("RAV");
  m_model.applyFilters();

  EXPECT_EQ(resets, 0);
  EXPECT_EQ(removes, 2);
  EXPECT_EQ(names(), (std::vector<QString>{"Bravo"}));
}

TEST_F(LibraryEntrySortFilterModelTest, WideningAFilterInsertsWithoutResetting) {
  m_model.setFilterText("RAV");
  m_model.applyFilters();
  ASSERT_EQ(m_model.getCount(), 1);

  auto resets = 0;
  auto inserts = 0;
  QObject::connect(&m_model, &QAbstractItemModel::modelReset, [&resets] { ++resets; });
  QObject::connect(&m_model, &QAbstractItemModel::rowsInserted, [&inserts] { ++inserts; });

  m_model.setFilterText("");
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

// A group takes one row, and it is the entry standing for the group
TEST_F(LibraryEntrySortFilterModelTest, AGroupShowsAsOneRow) {
  ASSERT_TRUE(groupUsaAndJapan());
  pump();

  EXPECT_EQ(m_model.getCount(), 4);
  const auto shown = names();
  EXPECT_EQ(std::count(shown.begin(), shown.end(), QStringLiteral("Zelda (USA)")), 1);
  EXPECT_EQ(std::count(shown.begin(), shown.end(), QStringLiteral("Zelda (Japan)")), 0);
}

// Turning collapsing off is what the "show every version" option does
TEST_F(LibraryEntrySortFilterModelTest, ShowingEveryVariantBringsTheOthersBack) {
  ASSERT_TRUE(groupUsaAndJapan());
  pump();
  ASSERT_EQ(m_model.getCount(), 4);

  m_model.setCollapseVariants(false);
  pump();

  EXPECT_EQ(m_model.getCount(), 5);
  const auto shown = names();
  EXPECT_EQ(std::count(shown.begin(), shown.end(), QStringLiteral("Zelda (Japan)")), 1);
}

// Searching a folded-away variant by its own name still finds the group
TEST_F(LibraryEntrySortFilterModelTest, AFoldedVariantIsFoundByItsOwnName) {
  ASSERT_TRUE(groupUsaAndJapan());
  pump();

  m_model.setFilterText("japan");
  m_model.applyFilters();
  pump();

  EXPECT_EQ(names(), (std::vector<QString>{"Zelda (USA)"}));
}

// The chips count what the grid shows, not how many files are behind it
TEST_F(LibraryEntrySortFilterModelTest, PlatformCountsFollowTheCollapsedRows) {
  ASSERT_TRUE(groupUsaAndJapan());
  pump();

  EXPECT_EQ(m_model.getCountByPlatform().value("3").toInt(), 3);

  m_model.setCollapseVariants(false);
  pump();

  EXPECT_EQ(m_model.getCountByPlatform().value("3").toInt(), 4);
}

} // namespace firelight::gui

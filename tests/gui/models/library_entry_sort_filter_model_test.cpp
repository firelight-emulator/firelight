#include "gui/models/library_entry_sort_filter_model.hpp"

#include "app/library/gui/entry_list_model.hpp"
#include "sqlite_achievement_repository.hpp"

#include <firelight/achievement_service.hpp>
#include <firelight/activity/sqlite_activity_log.hpp>
#include <firelight/library/sqlite_user_library.hpp>
#include <firelight/library/user_library_service.hpp>
#include <firelight/platforms/platform_service.hpp>

#include <QDir>
#include <QEventLoop>
#include <QTimer>
#include <gtest/gtest.h>

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
  library::UserLibraryService m_service{m_repo, (QDir::tempPath() + "/fl_lesfm_test").toStdString()};
  activity::SqliteActivityLog m_activityLog{":memory:"};
  platforms::PlatformService m_platformService;
  achievements::SqliteAchievementRepository m_achievementRepo{":memory:"};
  achievements::AchievementService m_achievementService{m_achievementRepo};

  void SetUp() override {
    auto charlie = makeEntry("Charlie", "hashC", 3);
    auto alpha = makeEntry("alpha", "hashA", 3, true);
    auto bravo = makeEntry("Bravo", "hashB", 7);

    ASSERT_TRUE(m_repo.createEntry(charlie));
    ASSERT_TRUE(m_repo.createEntry(alpha));
    ASSERT_TRUE(m_repo.createEntry(bravo));

    m_source.emplace(m_service, m_activityLog, m_platformService, m_achievementService);
    m_model.setSourceModel(&m_source.value());
  }

  std::optional<library::EntryListModel> m_source;
  LibraryEntrySortFilterModel m_model;

  std::vector<QString> names() {
    std::vector<QString> result;

    for (auto row = 0; row < m_model.rowCount(QModelIndex()); ++row) {
      result.push_back(m_model.data(m_model.index(row, 0), library::EntryListModel::DisplayName).toString());
    }

    return result;
  }
};

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

// Setting properties stages them; nothing rebuilds until the caller commits, so
// changing several costs one reset rather than one each
TEST_F(LibraryEntrySortFilterModelTest, RebuildsOnlyWhenApplied) {
  auto resets = 0;
  QObject::connect(&m_model, &QAbstractItemModel::modelReset, [&resets] { ++resets; });

  m_model.setFilterText("a");
  m_model.setSortAscending(false);
  m_model.setPlatformIds({3});

  EXPECT_EQ(resets, 0);
  EXPECT_EQ(m_model.getCount(), 3);

  m_model.applyFilters();

  EXPECT_EQ(resets, 1);
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

} // namespace firelight::gui

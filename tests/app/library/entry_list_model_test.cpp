#include "app/library/gui/entry_list_model.hpp"
#include "sqlite_achievement_repository.hpp"

#include <firelight/achievement_service.hpp>
#include <firelight/activity/sqlite_activity_log.hpp>
#include <firelight/library/sqlite_user_library.hpp>
#include <firelight/library/user_library_repository.hpp>
#include <firelight/library/user_library_service.hpp>
#include <firelight/platforms/platform_service.hpp>

#include <QDir>
#include <QEventLoop>
#include <QTimer>
#include <gtest/gtest.h>

// Verifies that EntryListModel stays in sync with the library incrementally:
// EntryCreatedEvent inserts a row, EntryUpdatedEvent removes a now-hidden entry
// and re-inserts an unhidden one -- all without a full model reset. The events
// are delivered via a queued invocation, so each test pumps the event loop.
namespace firelight::library {
namespace {

void pump() {
  QEventLoop loop;
  QTimer::singleShot(100, &loop, &QEventLoop::quit);
  loop.exec();
}

Entry makeEntry(const std::string &name, const std::string &hash,
                unsigned platformId) {
  Entry e;
  e.displayName = name;
  e.contentHash = hash;
  e.platformId = platformId;
  return e;
}

} // namespace

class EntryListModelSyncTest : public testing::Test {
protected:
  SqliteUserLibraryRepository m_repo{":memory:"};
  UserLibraryService m_service{
      m_repo, (QDir::tempPath() + "/fl_elm_test").toStdString()};
  activity::SqliteActivityLog m_activityLog{":memory:"};
  platforms::PlatformService m_platformService;
  achievements::SqliteAchievementRepository m_achievementRepo{":memory:"};
  achievements::AchievementService m_achievementService{m_achievementRepo};
  EntryListModel m_model{m_service, m_activityLog, m_platformService,
                         m_achievementService};

  int rows() { return m_model.rowCount(QModelIndex()); }
};

// A newly-created entry appears as a row without a reset.
TEST_F(EntryListModelSyncTest, CreateEventInsertsRow) {
  ASSERT_EQ(rows(), 0);

  Entry e = makeEntry("Alpha", "hashA", 3);
  ASSERT_TRUE(m_repo.createEntry(e)); // publishes EntryCreatedEvent
  pump();

  EXPECT_EQ(rows(), 1);
}

// Hiding an entry (as its content disappears) removes its row; unhiding it
// re-inserts it.
TEST_F(EntryListModelSyncTest, HidingRemovesAndUnhidingReinserts) {
  Entry e = makeEntry("Beta", "hashB", 3);
  ASSERT_TRUE(m_repo.createEntry(e));
  pump();
  ASSERT_EQ(rows(), 1);

  auto entry = m_repo.getEntryWithContentHash("hashB");
  ASSERT_TRUE(entry.has_value());
  entry->hidden = true;
  ASSERT_TRUE(m_repo.update(*entry)); // publishes EntryUpdatedEvent
  pump();
  EXPECT_EQ(rows(), 0);

  entry->hidden = false;
  ASSERT_TRUE(m_repo.update(*entry));
  pump();
  EXPECT_EQ(rows(), 1);
}

// A visible entry that is updated is refreshed in place, not duplicated.
TEST_F(EntryListModelSyncTest, UpdateOfVisibleEntryDoesNotDuplicate) {
  Entry e = makeEntry("Gamma", "hashC", 3);
  ASSERT_TRUE(m_repo.createEntry(e));
  pump();
  ASSERT_EQ(rows(), 1);

  auto entry = m_repo.getEntryWithContentHash("hashC");
  ASSERT_TRUE(entry.has_value());
  entry->favorite = true;
  ASSERT_TRUE(m_repo.update(*entry));
  pump();

  EXPECT_EQ(rows(), 1);
  EXPECT_EQ(m_model.numFavorites(), 1);
}

} // namespace firelight::library

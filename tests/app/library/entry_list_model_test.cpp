#include "app/library/gui/entry_list_model.hpp"

#include "sqlite_achievement_repository.hpp"

#include <firelight/achievement_service.hpp>
#include <firelight/activity/sqlite_activity_log.hpp>
#include <firelight/library/library_ingest_service.hpp>
#include <firelight/library/sqlite_user_library.hpp>
#include <firelight/library/user_library_repository.hpp>
#include <firelight/library/user_library_service.hpp>
#include <firelight/platforms/platform_service.hpp>
#include <firelight/settings/sqlite_settings_repository.hpp>

#include <QDir>
#include <QEventLoop>
#include <QTimer>
#include <gtest/gtest.h>
#include <library/variant_group_service.hpp>

// Verifies that EntryListModel stays in sync with the library incrementally:
// EntryCreatedEvent inserts a row, EntryUpdatedEvent removes a now-hidden entry
// and re-inserts an unhidden one -- all without a full model reset. The events
// are delivered via a queued invocation, so each test pumps the event loop
namespace firelight::library {
namespace {

void pump() {
  QEventLoop loop;
  QTimer::singleShot(100, &loop, &QEventLoop::quit);
  loop.exec();
}

Entry makeEntry(const std::string &name, const std::string &hash, unsigned platformId) {
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
  LibraryIngestService m_ingest{m_repo};
  UserLibraryService m_service{m_repo, (QDir::tempPath() + "/fl_elm_test").toStdString()};
  activity::SqliteActivityLog m_activityLog{":memory:"};
  platforms::PlatformService m_platformService;
  achievements::SqliteAchievementRepository m_achievementRepo{":memory:"};
  achievements::AchievementService m_achievementService{m_achievementRepo};
  settings::SqliteSettingsRepository m_settingsRepo{":memory:"};
  settings::SettingsService m_settingsService{m_settingsRepo};
  VariantGroupService m_variantGroups{m_service, m_settingsService};
  EntryListModel m_model{m_service,       m_activityLog,    m_platformService, m_achievementService,
                         m_variantGroups, m_settingsService};

  int rows() { return m_model.rowCount(QModelIndex()); }
};

// A newly-created entry appears as a row without a reset
TEST_F(EntryListModelSyncTest, CreateEventInsertsRow) {
  ASSERT_EQ(rows(), 0);

  Entry e = makeEntry("Alpha", "hashA", 3);
  ASSERT_TRUE(m_repo.createEntry(e)); // publishes EntryCreatedEvent
  pump();

  EXPECT_EQ(rows(), 1);
}

// A game whose content disappears keeps its row and says why. Taking the row away is what
// left somebody unable to find out where their game had gone
TEST_F(EntryListModelSyncTest, ContentGoingMissingKeepsTheRowAndReportsIt) {
  ContentFile file{.m_fileSizeBytes = 1024, .m_filePath = "/roms/Beta.gb", .m_platformId = 3, .m_contentHash = "hashB"};
  ASSERT_TRUE(m_repo.create(file));
  pump();
  ASSERT_EQ(rows(), 1);

  ASSERT_TRUE(m_repo.markContentFileMissing(file.m_id));
  pump();

  ASSERT_EQ(rows(), 1);
  const auto index = m_model.index(0, 0);
  EXPECT_FALSE(m_model.data(index, EntryListModel::Playable).toBool());
  EXPECT_TRUE(m_model.data(index, EntryListModel::Problems)
                  .value<QList<int>>()
                  .contains(static_cast<int>(EntryProblem::FilesMissing)));
  EXPECT_FALSE(m_model.data(index, EntryListModel::StatusText).toString().isEmpty());

  ASSERT_TRUE(m_repo.reviveContentFile(file.m_id));
  pump();

  EXPECT_EQ(rows(), 1);
  EXPECT_TRUE(m_model.data(m_model.index(0, 0), EntryListModel::Playable).toBool());
}

// The row surviving is only half of it: keeping the content file row is what lets the sentence say
// where the game was, which is the thing somebody actually needs
TEST_F(EntryListModelSyncTest, AMissingGameSaysWhereItsFilesWere) {
  ContentFile file{
      .m_fileSizeBytes = 2048, .m_filePath = "E:/Games/Gamma.gb", .m_platformId = 3, .m_contentHash = "hashC"};
  ASSERT_TRUE(m_repo.create(file));
  pump();
  ASSERT_EQ(rows(), 1);

  ASSERT_TRUE(m_repo.markContentFileMissing(file.m_id));
  pump();

  ASSERT_EQ(rows(), 1) << "the row went with the file";
  const auto index = m_model.index(0, 0);
  EXPECT_FALSE(m_model.data(index, EntryListModel::Playable).toBool());

  const auto text = m_model.data(index, EntryListModel::StatusText).toString();
  EXPECT_TRUE(text.contains(QStringLiteral("Gamma.gb")))
      << "the status does not say where the game was: " << text.toStdString();
}

// Folding a disc set destroys the entries it absorbs. Without an event naming the id that went, the
// absorbed disc keeps a tile that renders stale data and fails when launched
TEST_F(EntryListModelSyncTest, AnEntryThatIsDeletedLeavesTheGrid) {
  Entry keep = makeEntry("Survivor", "hashS", 3);
  ASSERT_TRUE(m_repo.createEntry(keep));
  Entry drop = makeEntry("Absorbed", "hashD", 3);
  ASSERT_TRUE(m_repo.createEntry(drop));
  pump();
  ASSERT_EQ(rows(), 2);

  ASSERT_TRUE(m_repo.deleteEntry(drop.id));
  pump();

  ASSERT_EQ(rows(), 1) << "the deleted entry kept its row";
  EXPECT_EQ(m_model.data(m_model.index(0, 0), EntryListModel::DisplayName).toString(), QStringLiteral("Survivor"));
}

// A game is only unplayable when nothing can reach it. One copy on a drive that is unplugged says
// nothing while another sits on a disk that is right here
TEST_F(EntryListModelSyncTest, OneCopyOnAnUnpluggedDriveDoesNotCondemnTheOther) {
  ContentDirectory offline{.path = "Z:/NotMounted"};
  ASSERT_TRUE(m_repo.create(offline));
  ContentDirectory here{.path = QDir::tempPath().toStdString()};
  ASSERT_TRUE(m_repo.create(here));

  ContentFile onDrive{
      .m_fileSizeBytes = 2048, .m_filePath = "Z:/NotMounted/Kirby.gb", .m_platformId = 3, .m_contentHash = "hashK"};
  ASSERT_TRUE(m_repo.create(onDrive));
  ContentFile backup{.m_fileSizeBytes = 2048,
                     .m_filePath = QDir::tempPath().toStdString() + "/Kirby.gb",
                     .m_platformId = 3,
                     .m_contentHash = "hashK"};
  ASSERT_TRUE(m_repo.create(backup));
  pump();

  m_model.refreshStatuses();
  ASSERT_EQ(rows(), 1);

  const auto index = m_model.index(0, 0);
  EXPECT_TRUE(m_model.data(index, EntryListModel::Playable).toBool())
      << "an unplugged drive condemned a game that is also sitting on a readable disk: "
      << m_model.data(index, EntryListModel::StatusText).toString().toStdString();
  EXPECT_FALSE(m_model.data(index, EntryListModel::Problems)
                   .value<QList<int>>()
                   .contains(static_cast<int>(EntryProblem::ContentUnavailable)));
}

// The other half: with every copy behind an unplugged drive, that is exactly what to say
TEST_F(EntryListModelSyncTest, EveryCopyBehindAnUnpluggedDriveIsReportedAsUnavailable) {
  ContentDirectory offline{.path = "Z:/NotMounted"};
  ASSERT_TRUE(m_repo.create(offline));

  ContentFile onDrive{
      .m_fileSizeBytes = 2048, .m_filePath = "Z:/NotMounted/Kirby.gb", .m_platformId = 3, .m_contentHash = "hashK"};
  ASSERT_TRUE(m_repo.create(onDrive));
  pump();

  m_model.refreshStatuses();
  ASSERT_EQ(rows(), 1);

  const auto index = m_model.index(0, 0);
  EXPECT_FALSE(m_model.data(index, EntryListModel::Playable).toBool());
  EXPECT_TRUE(m_model.data(index, EntryListModel::Problems)
                  .value<QList<int>>()
                  .contains(static_cast<int>(EntryProblem::ContentUnavailable)));
}

// "Launching this game is not yet supported" leaves somebody guessing which of their systems it
// meant. The platform is known here, so the sentence says it
TEST_F(EntryListModelSyncTest, AnUnsupportedPlatformIsNamedInTheStatus) {
  using PS = platforms::PlatformService;

  // No core runs the Saturn, so every Saturn entry carries the platform problem
  Entry e = makeEntry("Panzer Dragoon", "hashSat", PS::PLATFORM_ID_SEGA_SATURN);
  ASSERT_TRUE(m_repo.createEntry(e));
  pump();
  ASSERT_EQ(rows(), 1);

  const auto platform = m_platformService.getPlatform(PS::PLATFORM_ID_SEGA_SATURN);
  ASSERT_TRUE(platform.has_value());

  const auto text = m_model.data(m_model.index(0, 0), EntryListModel::StatusText).toString();

  EXPECT_FALSE(m_model.data(m_model.index(0, 0), EntryListModel::Playable).toBool());
  EXPECT_TRUE(text.contains(QString::fromStdString(platform->name)))
      << "status did not name the platform: " << text.toStdString();
}

// A visible entry that is updated is refreshed in place, not duplicated
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

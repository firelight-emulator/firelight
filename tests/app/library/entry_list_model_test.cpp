// TODO: NEEDS REVIEW
#include "app/library/gui/entry_list_model.hpp"

#include "sqlite_achievement_repository.hpp"

#include <firelight/achievement_service.hpp>
#include <firelight/activity/sqlite_activity_log.hpp>
#include <firelight/library/disc_set_service.hpp>
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
  DiscSetService m_discSets{m_repo, ""};
  LibraryIngestService m_ingest{m_repo, m_discSets};
  UserLibraryService m_service{m_repo, (QDir::tempPath() + "/fl_elm_test").toStdString()};
  activity::SqliteActivityLog m_activityLog{":memory:"};
  platforms::PlatformService m_platformService;
  achievements::SqliteAchievementRepository m_achievementRepo{":memory:"};
  achievements::AchievementService m_achievementService{m_achievementRepo};
  settings::SqliteSettingsRepository m_settingsRepo{":memory:"};
  settings::SettingsService m_settingsService{m_settingsRepo};
  EntryListModel m_model{m_service, m_activityLog, m_platformService, m_achievementService, m_settingsService};

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

// TODO
// Grouping is unwired, so a group standing in the database is a row of history rather than a rule
// the grid follows. These four say what the grid does with one
class EntryListModelVariantTest : public EntryListModelSyncTest {
protected:
  // TODO
  // Two releases of one game, put in a group the way the service would have
  struct Pair {
    int usaId;
    int japanId;
    int groupId;
  };

  Pair makeGroupedPair() {
    Entry usa = makeEntry("Chrono Trigger (USA)", "hashUSA", 6);
    EXPECT_TRUE(m_repo.createEntry(usa));
    Entry japan = makeEntry("Chrono Trigger (Japan)", "hashJPN", 6);
    EXPECT_TRUE(m_repo.createEntry(japan));

    VariantGroup group{.title = "Chrono Trigger"};
    EXPECT_TRUE(m_repo.createVariantGroup(group));
    EXPECT_TRUE(m_repo.setEntryVariantGroup(usa.id, group.id, false));
    EXPECT_TRUE(m_repo.setEntryVariantGroup(japan.id, group.id, false));

    group.primaryEntryId = usa.id;
    EXPECT_TRUE(m_repo.updateVariantGroup(group));

    m_model.reset();
    pump();

    return {usa.id, japan.id, group.id};
  }

  [[nodiscard]] QModelIndex rowFor(const int entryId) {
    for (auto i = 0; i < rows(); ++i) {
      const auto index = m_model.index(i, 0);

      if (m_model.data(index, EntryListModel::Id).toInt() == entryId) {
        return index;
      }
    }

    return {};
  }
};

// TODO
// Folding a group away is what hides a game somebody owns behind one they do not want
TEST_F(EntryListModelVariantTest, GroupedEntriesEachKeepTheirOwnRow) {
  const auto pair = makeGroupedPair();

  ASSERT_EQ(rows(), 2) << "a grouped release lost its row";
  EXPECT_EQ(m_model.data(rowFor(pair.usaId), EntryListModel::DisplayName).toString(),
            QStringLiteral("Chrono Trigger (USA)"));
  EXPECT_EQ(m_model.data(rowFor(pair.japanId), EntryListModel::DisplayName).toString(),
            QStringLiteral("Chrono Trigger (Japan)"))
      << "a row showed the group's title rather than its own name";
}

// TODO
// Renaming what is on screen should rename what is on screen
TEST_F(EntryListModelVariantTest, RenamingAGroupedEntryRenamesTheEntry) {
  const auto pair = makeGroupedPair();

  ASSERT_TRUE(m_model.setData(rowFor(pair.usaId), QStringLiteral("Chrono Trigger US"), EntryListModel::DisplayName));
  pump();

  const auto renamed = m_repo.getEntry(pair.usaId);
  ASSERT_TRUE(renamed.has_value());
  EXPECT_EQ(renamed->displayName, "Chrono Trigger US") << "the rename went to the group instead of the entry";

  const auto untouched = m_repo.getEntry(pair.japanId);
  ASSERT_TRUE(untouched.has_value());
  EXPECT_EQ(untouched->displayName, "Chrono Trigger (Japan)") << "renaming one release renamed another";
}

// TODO
// A group-wide total told somebody they had played a release they had never launched
TEST_F(EntryListModelVariantTest, PlaytimeIsTheEntrysOwnNotTheGroups) {
  const auto pair = makeGroupedPair();

  activity::PlaySession session;
  session.contentHash = "hashUSA";
  session.startedAt = 1000;
  session.endedAt = 301000;
  session.unpausedDurationMillis = 300000;
  ASSERT_TRUE(m_activityLog.createPlaySession(session));

  m_model.reset();
  pump();

  EXPECT_EQ(m_model.data(rowFor(pair.usaId), EntryListModel::NumSecondsPlayed).toInt(), 300);
  EXPECT_EQ(m_model.data(rowFor(pair.japanId), EntryListModel::NumSecondsPlayed).toInt(), 0)
      << "a release nobody launched reported the group's playtime";
}

// TODO
// Folding sibling names into the search text is only worth it while the siblings are hidden
TEST_F(EntryListModelVariantTest, SearchTextIsJustTheEntrysName) {
  const auto pair = makeGroupedPair();

  const auto searchText = m_model.data(rowFor(pair.usaId), EntryListModel::SearchText).toString();

  EXPECT_TRUE(searchText.contains(QStringLiteral("usa"), Qt::CaseInsensitive)) << searchText.toStdString();
  EXPECT_FALSE(searchText.contains(QStringLiteral("japan"), Qt::CaseInsensitive))
      << "a row carried a sibling's name in its search text: " << searchText.toStdString();
}

// TODO
// The filter predicate reads a record cached on the row rather than one rebuilt per pass, and a
// smart folder's count is that same predicate over the same rows
class EntryListModelFilterFieldsTest : public EntryListModelSyncTest {
protected:
  int makeSmartFolder(const std::string &name, const std::string &filterJson) {
    FolderInfo folder;
    folder.displayName = name;
    folder.type = static_cast<int>(FolderType::Smart);
    folder.filterJson = filterJson;
    EXPECT_TRUE(m_repo.create(folder));
    return folder.id;
  }

  [[nodiscard]] const EntryFields *fieldsAt(const int row) {
    return m_model.data(m_model.index(row, 0), EntryListModel::FilterFields).value<const EntryFields *>();
  }
};

// TODO
// Every dimension the predicate can ask about has to be on the row, or a filter silently
// matches nothing
TEST_F(EntryListModelFilterFieldsTest, FilterFieldsCarryEverythingThePredicateReads) {
  ContentFile file{
      .m_fileSizeBytes = 1024, .m_filePath = "/roms/Metroid.gb", .m_platformId = 3, .m_contentHash = "hashM"};
  ASSERT_TRUE(m_repo.create(file));
  pump();
  ASSERT_EQ(rows(), 1);

  auto entry = m_repo.getEntryWithContentHash("hashM");
  ASSERT_TRUE(entry.has_value());
  entry->displayName = "Metroid II";
  entry->favorite = true;
  ASSERT_TRUE(m_repo.update(*entry));

  GameMetadata metadata;
  metadata.developer = "Nintendo";
  metadata.publisher = "Nintendo";
  metadata.releaseYear = 1991;
  metadata.genres = {"Action"};
  ASSERT_TRUE(m_repo.applyEntryMetadata(
      entry->id, metadata,
      {metadata_fields::DEVELOPER, metadata_fields::PUBLISHER, metadata_fields::RELEASE_YEAR, metadata_fields::GENRES},
      false));
  m_model.reset();
  pump();

  const auto *fields = fieldsAt(0);
  ASSERT_NE(fields, nullptr) << "the row carried no filter fields";
  EXPECT_EQ(fields->platformId, 3);
  EXPECT_TRUE(fields->favorite);
  EXPECT_EQ(fields->developer, "Nintendo");
  EXPECT_EQ(fields->releaseYear, 1991);
  EXPECT_TRUE(fields->playable) << "a game with a file on disk read as unplayable";
  EXPECT_NE(fields->searchText.find("metroid"), std::string::npos)
      << "search text was not carried: " << fields->searchText;
  EXPECT_FALSE(fields->contentPaths.empty());
}

// TODO
// The sidebar's count and the grid's rows come from one predicate, so they cannot disagree
TEST_F(EntryListModelFilterFieldsTest, SmartFolderCountsMatchTheRows) {
  Entry snes = makeEntry("Chrono Trigger", "hashSNES", 3);
  ASSERT_TRUE(m_repo.createEntry(snes));
  Entry genesis = makeEntry("Sonic", "hashGEN", 7);
  ASSERT_TRUE(m_repo.createEntry(genesis));
  pump();
  ASSERT_EQ(rows(), 2);

  const auto folderId = makeSmartFolder("SNES only", R"({"platformIds":[3]})");
  pump();

  EXPECT_EQ(m_model.getCountByFolderId().value(QString::number(folderId)).toInt(), 1);
}

// TODO
// Editing a folder's criteria has to reach the count without the view asking it to
TEST_F(EntryListModelFilterFieldsTest, EditingASmartFoldersCriteriaChangesItsCount) {
  Entry snes = makeEntry("Chrono Trigger", "hashSNES", 3);
  ASSERT_TRUE(m_repo.createEntry(snes));
  Entry genesis = makeEntry("Sonic", "hashGEN", 7);
  ASSERT_TRUE(m_repo.createEntry(genesis));
  pump();

  const auto folderId = makeSmartFolder("Everything", "{}");
  pump();
  ASSERT_EQ(m_model.getCountByFolderId().value(QString::number(folderId)).toInt(), 2);

  auto folder = FolderInfo{};
  for (const auto &f : m_repo.listFolders()) {
    if (f.id == folderId) {
      folder = f;
      break;
    }
  }
  folder.filterJson = R"({"platformIds":[7]})";
  ASSERT_TRUE(m_repo.update(folder));
  pump();

  EXPECT_EQ(m_model.getCountByFolderId().value(QString::number(folderId)).toInt(), 1)
      << "the count kept the criteria the folder no longer has";
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

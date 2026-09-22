// TODO: NEEDS REVIEW
#include "app/library/gui/playlist_item_model.hpp"
#include "app/service_accessor.hpp"

#include <firelight/library/sqlite_user_library.hpp>
#include <firelight/library/user_library_service.hpp>

#include <QDir>
#include <QSignalSpy>
#include <gtest/gtest.h>
#include <optional>

// Reordering collections: what a move does to the list on screen, and what it leaves in the database
namespace firelight::gui {

class FolderListModelTest : public testing::Test {
protected:
  library::SqliteUserLibraryRepository m_repo{":memory:"};
  library::UserLibraryService m_service{m_repo, (QDir::tempPath() + "/fl_folder_model_test").toStdString()};
  std::optional<LibraryFolderListModel> m_model;

  int m_alpha{-1};
  int m_bravo{-1};
  int m_charlie{-1};

  void SetUp() override {
    ServiceAccessor::setLibraryService(&m_service);

    m_alpha = makeFolder("alpha");
    m_bravo = makeFolder("bravo");
    m_charlie = makeFolder("charlie");
    ASSERT_NE(m_alpha, -1);
    ASSERT_NE(m_bravo, -1);
    ASSERT_NE(m_charlie, -1);

    // The model reads the folders once, on construction
    m_model.emplace();
  }

  void TearDown() override { ServiceAccessor::setLibraryService(nullptr); }

  int makeFolder(const std::string &name) {
    library::FolderInfo folder;
    folder.displayName = name;

    return m_repo.create(folder) ? folder.id : -1;
  }

  // The ids in the order the list hands them to a delegate
  std::vector<int> onScreen() {
    std::vector<int> ids;

    for (auto row = 0; row < m_model->rowCount({}); ++row) {
      ids.push_back(m_model->data(m_model->index(row, 0), LibraryFolderListModel::FolderId).toInt());
    }

    return ids;
  }

  // The ids in the order a fresh read gets them back
  std::vector<int> stored() {
    std::vector<int> ids;

    for (const auto &folder : m_repo.listFolders()) {
      ids.push_back(folder.id);
    }

    return ids;
  }

  // TODO
  // One role of the row showing a folder, or an invalid value when no row does
  QVariant shown(const int folderId, const int role) {
    for (auto row = 0; row < m_model->rowCount({}); ++row) {
      const auto index = m_model->index(row, 0);
      if (m_model->data(index, LibraryFolderListModel::FolderId).toInt() == folderId) {
        return m_model->data(index, role);
      }
    }

    return {};
  }

  // TODO
  // The folder as a fresh read gets it back
  std::optional<library::FolderInfo> storedFolder(const int folderId) {
    for (const auto &folder : m_repo.listFolders()) {
      if (folder.id == folderId) {
        return folder;
      }
    }

    return std::nullopt;
  }
};

// Moving down puts the row after the ones it passed, and the database agrees
TEST_F(FolderListModelTest, MovingARowDownReordersTheListAndWhatIsStored) {
  ASSERT_TRUE(m_model->moveRows({}, 0, 1, {}, 3));

  const auto expected = std::vector{m_bravo, m_charlie, m_alpha};
  EXPECT_EQ(onScreen(), expected);
  EXPECT_EQ(stored(), expected);
}

// Moving up puts the row before the ones it passed, and the database agrees
TEST_F(FolderListModelTest, MovingARowUpReordersTheListAndWhatIsStored) {
  ASSERT_TRUE(m_model->moveRows({}, 2, 1, {}, 0));

  const auto expected = std::vector{m_charlie, m_alpha, m_bravo};
  EXPECT_EQ(onScreen(), expected);
  EXPECT_EQ(stored(), expected);
}

// More than one row travels together, keeping their order
TEST_F(FolderListModelTest, MovingSeveralRowsKeepsThemTogether) {
  ASSERT_TRUE(m_model->moveRows({}, 0, 2, {}, 3));

  const auto expected = std::vector{m_charlie, m_alpha, m_bravo};
  EXPECT_EQ(onScreen(), expected);
  EXPECT_EQ(stored(), expected);
}

// A destination inside the block being moved names no new place for it
TEST_F(FolderListModelTest, AMoveThatGoesNowhereIsRefused) {
  const auto before = onScreen();

  EXPECT_FALSE(m_model->moveRows({}, 1, 1, {}, 1));
  EXPECT_FALSE(m_model->moveRows({}, 1, 1, {}, 2));
  EXPECT_FALSE(m_model->moveRows({}, 0, 2, {}, 1));

  EXPECT_EQ(onScreen(), before);
  EXPECT_EQ(stored(), before);
}

// Nothing outside the list can be moved, or moved to
TEST_F(FolderListModelTest, AnOutOfRangeMoveIsRefused) {
  const auto before = onScreen();

  EXPECT_FALSE(m_model->moveRows({}, -1, 1, {}, 0));
  EXPECT_FALSE(m_model->moveRows({}, 0, 0, {}, 2));
  EXPECT_FALSE(m_model->moveRows({}, 2, 2, {}, 0));
  EXPECT_FALSE(m_model->moveRows({}, 0, 1, {}, 4));

  EXPECT_EQ(onScreen(), before);
  EXPECT_EQ(stored(), before);
}

// Positions are numbered within a parent, so a move that would cross one has nothing to write
TEST_F(FolderListModelTest, AMoveAcrossParentsIsRefused) {
  ASSERT_TRUE(m_service.setFolderParent(m_charlie, m_alpha));

  m_model.emplace();
  const auto before = onScreen();

  EXPECT_FALSE(m_model->moveRows({}, 0, 1, {}, 3));
  EXPECT_EQ(onScreen(), before);
  EXPECT_EQ(stored(), before);
}

// The reorder page reads its working copy from here, so the order must match what the list shows
TEST_F(FolderListModelTest, FoldersInParentComeBackInDisplayOrder) {
  const auto folders = m_model->foldersInParent(-1);

  ASSERT_EQ(folders.size(), 3);
  EXPECT_EQ(folders[0].toMap().value("folderId").toInt(), m_alpha);
  EXPECT_EQ(folders[1].toMap().value("folderId").toInt(), m_bravo);
  EXPECT_EQ(folders[2].toMap().value("folderId").toInt(), m_charlie);
  EXPECT_EQ(folders[0].toMap().value("displayName").toString(), QString("alpha"));
}

// A move is visible to the next read, which is what makes reopening the page show the saved order
TEST_F(FolderListModelTest, FoldersInParentFollowsAMove) {
  ASSERT_TRUE(m_model->moveRows({}, 0, 1, {}, 3));

  const auto folders = m_model->foldersInParent(-1);
  ASSERT_EQ(folders.size(), 3);
  EXPECT_EQ(folders[0].toMap().value("folderId").toInt(), m_bravo);
  EXPECT_EQ(folders[2].toMap().value("folderId").toInt(), m_alpha);
}

// Only the scope asked for, so a nested collection never appears in its parent's arrangement
TEST_F(FolderListModelTest, FoldersInParentIsScopedToThatParent) {
  ASSERT_TRUE(m_service.setFolderParent(m_charlie, m_alpha));
  m_model.emplace();

  const auto root = m_model->foldersInParent(-1);
  const auto nested = m_model->foldersInParent(m_alpha);

  EXPECT_EQ(root.size(), 2);
  ASSERT_EQ(nested.size(), 1);
  EXPECT_EQ(nested[0].toMap().value("folderId").toInt(), m_charlie);
}

TEST_F(FolderListModelTest, CreateCollectionRoundTripsEveryFieldThroughTheRoles) {
  const QVariantMap fields{{"displayName", QString("  delta  ")},
                           {"description", QString("Four of them")},
                           {"icon1x1SourceUrl", QString("file:///covers/delta.png")},
                           {"color", QString("#112233")},
                           {"folderType", static_cast<int>(library::FolderType::Manual)},
                           {"parentId", m_alpha},
                           {"sortRole", QString("playTime")},
                           {"sortAscending", false}};

  const auto id = m_model->createCollection(fields);
  ASSERT_NE(id, -1);

  EXPECT_EQ(shown(id, LibraryFolderListModel::DisplayName).toString(), QString("delta"));
  EXPECT_EQ(shown(id, LibraryFolderListModel::Description).toString(), QString("Four of them"));
  EXPECT_EQ(shown(id, LibraryFolderListModel::Icon1x1SourceUrl).toString(), QString("file:///covers/delta.png"));
  EXPECT_EQ(shown(id, LibraryFolderListModel::Color).toString(), QString("#112233"));
  EXPECT_EQ(shown(id, LibraryFolderListModel::FolderType).toInt(), static_cast<int>(library::FolderType::Manual));
  EXPECT_EQ(shown(id, LibraryFolderListModel::FilterJson).toString(), QString());
  EXPECT_EQ(shown(id, LibraryFolderListModel::SortRole).toString(), QString("playTime"));
  EXPECT_FALSE(shown(id, LibraryFolderListModel::SortAscending).toBool());
  EXPECT_EQ(shown(id, LibraryFolderListModel::ParentId).toInt(), m_alpha);
  EXPECT_EQ(shown(id, LibraryFolderListModel::Position).toInt(), 0);

  const auto folder = storedFolder(id);
  ASSERT_TRUE(folder.has_value());
  EXPECT_EQ(folder->displayName, "delta");
  EXPECT_EQ(folder->description, "Four of them");
  EXPECT_EQ(folder->iconSourceUrl, "file:///covers/delta.png");
  EXPECT_EQ(folder->color, "#112233");
  EXPECT_EQ(folder->sortRole, "playTime");
  EXPECT_FALSE(folder->sortAscending);
  EXPECT_EQ(folder->parentId, m_alpha);
}

TEST_F(FolderListModelTest, CreateCollectionMakesASmartCollectionWithItsCriteria) {
  const auto criteria = QString(R"({"platformIds":[7]})");
  const QVariantMap fields{{"displayName", QString("delta")},
                           {"folderType", static_cast<int>(library::FolderType::Smart)},
                           {"filterJson", criteria}};

  const auto id = m_model->createCollection(fields);
  ASSERT_NE(id, -1);

  EXPECT_EQ(shown(id, LibraryFolderListModel::FolderType).toInt(), static_cast<int>(library::FolderType::Smart));
  EXPECT_EQ(shown(id, LibraryFolderListModel::FilterJson).toString(), criteria);

  const auto folder = storedFolder(id);
  ASSERT_TRUE(folder.has_value());
  EXPECT_EQ(folder->type, static_cast<int>(library::FolderType::Smart));
  EXPECT_EQ(folder->filterJson, criteria.toStdString());
}

TEST_F(FolderListModelTest, CreateCollectionFillsAbsentFieldsWithTheDefaults) {
  const auto id = m_model->createCollection(QVariantMap{{"displayName", QString("delta")}});
  ASSERT_NE(id, -1);

  EXPECT_EQ(shown(id, LibraryFolderListModel::Description).toString(), QString());
  EXPECT_EQ(shown(id, LibraryFolderListModel::Icon1x1SourceUrl).toString(), QString());
  EXPECT_EQ(shown(id, LibraryFolderListModel::Color).toString(), QString());
  EXPECT_EQ(shown(id, LibraryFolderListModel::FolderType).toInt(), static_cast<int>(library::FolderType::Manual));
  EXPECT_EQ(shown(id, LibraryFolderListModel::FilterJson).toString(), QString());
  EXPECT_EQ(shown(id, LibraryFolderListModel::SortRole).toString(), QString());
  EXPECT_TRUE(shown(id, LibraryFolderListModel::SortAscending).toBool());
  EXPECT_EQ(shown(id, LibraryFolderListModel::ParentId).toInt(), -1);
  EXPECT_EQ(shown(id, LibraryFolderListModel::Position).toInt(), 3);
}

TEST_F(FolderListModelTest, CreateCollectionKeepsTheListInStoredOrder) {
  ASSERT_TRUE(m_service.setFolderParent(m_charlie, m_alpha));
  m_model.emplace();

  const auto id = m_model->createCollection(QVariantMap{{"displayName", QString("delta")}});
  ASSERT_NE(id, -1);

  const auto expected = std::vector{m_alpha, m_bravo, id, m_charlie};
  EXPECT_EQ(onScreen(), expected);
  EXPECT_EQ(stored(), expected);
}

TEST_F(FolderListModelTest, CreateCollectionReportsTheNewCount) {
  const QSignalSpy spy(&*m_model, &LibraryFolderListModel::countChanged);

  ASSERT_NE(m_model->createCollection(QVariantMap{{"displayName", QString("delta")}}), -1);

  EXPECT_EQ(spy.count(), 1);
  EXPECT_EQ(m_model->getCount(), 4);
}

TEST_F(FolderListModelTest, CreateCollectionRefusesANameAlreadyTaken) {
  const auto before = onScreen();
  const QSignalSpy spy(&*m_model, &LibraryFolderListModel::countChanged);

  EXPECT_EQ(m_model->createCollection(QVariantMap{{"displayName", QString("alpha")}}), -1);
  EXPECT_EQ(m_model->createCollection(QVariantMap{{"displayName", QString("  alpha  ")}}), -1);

  EXPECT_EQ(spy.count(), 0);
  EXPECT_EQ(onScreen(), before);
  EXPECT_EQ(stored(), before);
}

TEST_F(FolderListModelTest, CreateCollectionRefusesAnEmptyName) {
  const auto before = onScreen();
  const QSignalSpy spy(&*m_model, &LibraryFolderListModel::countChanged);

  EXPECT_EQ(m_model->createCollection(QVariantMap{}), -1);
  EXPECT_EQ(m_model->createCollection(QVariantMap{{"displayName", QString()}}), -1);
  EXPECT_EQ(m_model->createCollection(QVariantMap{{"displayName", QString("   ")}}), -1);

  EXPECT_EQ(spy.count(), 0);
  EXPECT_EQ(onScreen(), before);
  EXPECT_EQ(stored(), before);
}

TEST_F(FolderListModelTest, HasFolderNamedMatchesTheTrimmedNameExactly) {
  EXPECT_TRUE(m_model->hasFolderNamed("alpha"));
  EXPECT_TRUE(m_model->hasFolderNamed("  alpha  "));

  EXPECT_FALSE(m_model->hasFolderNamed("delta"));
  EXPECT_FALSE(m_model->hasFolderNamed("Alpha"));
  EXPECT_FALSE(m_model->hasFolderNamed("alph"));
  EXPECT_FALSE(m_model->hasFolderNamed(""));
}

TEST_F(FolderListModelTest, HasFolderNamedAgreesWithWhatCreateCollectionAccepts) {
  ASSERT_FALSE(m_model->hasFolderNamed("Alpha"));
  EXPECT_NE(m_model->createCollection(QVariantMap{{"displayName", QString("Alpha")}}), -1);
  EXPECT_TRUE(m_model->hasFolderNamed("Alpha"));

  ASSERT_TRUE(m_model->hasFolderNamed("bravo"));
  EXPECT_EQ(m_model->createCollection(QVariantMap{{"displayName", QString("bravo")}}), -1);
}

} // namespace firelight::gui

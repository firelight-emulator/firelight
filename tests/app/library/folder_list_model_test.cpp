// TODO: NEEDS REVIEW
#include "app/library/gui/playlist_item_model.hpp"
#include "app/service_accessor.hpp"

#include <firelight/library/sqlite_user_library.hpp>
#include <firelight/library/user_library_service.hpp>

#include <QDir>
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

} // namespace firelight::gui

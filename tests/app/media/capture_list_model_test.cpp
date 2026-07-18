#include "app/media/gui/capture_list_model.hpp"

#include <firelight/library/sqlite_user_library.hpp>
#include <firelight/library/user_library_service.hpp>
#include <firelight/media/sqlite_game_capture_repository.hpp>

#include <QDir>
#include <gtest/gtest.h>

namespace firelight::gui {
namespace {
media::GameCapture makeCapture(const std::string &hash, media::CaptureType type, const std::string &path) {
  media::GameCapture c;
  c.contentHash = hash;
  c.type = type;
  c.filePath = path;
  c.thumbnailPath = path;
  c.timestamp = 100;
  return c;
}
} // namespace

class CaptureListModelTest : public testing::Test {
protected:
  media::SqliteGameCaptureRepository m_captures{":memory:"};
  library::SqliteUserLibraryRepository m_libRepo{":memory:"};
  library::UserLibraryService m_library{m_libRepo, (QDir::tempPath() + "/fl_clm_test").toStdString()};
};

TEST_F(CaptureListModelTest, ExposesRolesAndResolvesGameName) {
  library::Entry entry;
  entry.displayName = "Cool Game";
  entry.contentHash = "gameX";
  entry.platformId = 3;
  ASSERT_TRUE(m_libRepo.createEntry(entry));

  auto shot = makeCapture("gameX", media::CaptureType::Screenshot, "/a/1.png");
  auto clip = makeCapture("gameX", media::CaptureType::Clip, "/a/2.mp4");
  ASSERT_TRUE(m_captures.add(shot));
  ASSERT_TRUE(m_captures.add(clip));

  CaptureListModel model(m_captures, m_library);
  ASSERT_EQ(model.rowCount(QModelIndex()), 2);

  const auto idx = model.index(0, 0);
  EXPECT_EQ(model.data(idx, CaptureListModel::GameName).toString(), QStringLiteral("Cool Game"));
  const auto type = model.data(idx, CaptureListModel::CaptureTypeName).toString();
  EXPECT_TRUE(type == "clip" || type == "screenshot");
  EXPECT_TRUE(model.data(idx, CaptureListModel::ThumbnailUrl).toString().startsWith("file:"));
}

TEST_F(CaptureListModelTest, FavoriteSetDataPersists) {
  auto shot = makeCapture("gameY", media::CaptureType::Screenshot, "/b/1.png");
  ASSERT_TRUE(m_captures.add(shot));

  CaptureListModel model(m_captures, m_library);
  ASSERT_EQ(model.rowCount(QModelIndex()), 1);

  const auto idx = model.index(0, 0);
  EXPECT_FALSE(model.data(idx, CaptureListModel::Favorite).toBool());

  ASSERT_TRUE(model.setData(idx, true, CaptureListModel::Favorite));
  EXPECT_TRUE(model.data(idx, CaptureListModel::Favorite).toBool());

  const auto stored = m_captures.getById(shot.id);
  ASSERT_TRUE(stored.has_value());
  EXPECT_TRUE(stored->favorite);
}

TEST_F(CaptureListModelTest, RemoveCaptureDropsRow) {
  auto shot = makeCapture("gameZ", media::CaptureType::Screenshot, "/c/1.png");
  ASSERT_TRUE(m_captures.add(shot));

  CaptureListModel model(m_captures, m_library);
  ASSERT_EQ(model.rowCount(QModelIndex()), 1);

  model.removeCapture(shot.id);
  EXPECT_EQ(model.rowCount(QModelIndex()), 0);
  EXPECT_FALSE(m_captures.getById(shot.id).has_value());
}

} // namespace firelight::gui

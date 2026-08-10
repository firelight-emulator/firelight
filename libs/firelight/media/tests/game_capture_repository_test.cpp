#include <firelight/media/sqlite_game_capture_repository.hpp>

#include <QTemporaryDir>
#include <gtest/gtest.h>
#include <memory>
#include <string>

namespace firelight::media {
namespace {
GameCapture makeCapture(const std::string &hash, CaptureType type, const std::string &path) {
  GameCapture c;
  c.contentHash = hash;
  c.type = type;
  c.filePath = path;
  c.thumbnailPath = path;
  c.capturedAt = 1000;
  return c;
}
} // namespace

class GameCaptureRepositoryTest : public testing::Test {
protected:
  QTemporaryDir dir;
  std::unique_ptr<SqliteGameCaptureRepository> repo;

  void SetUp() override {
    ASSERT_TRUE(dir.isValid());
    repo = std::make_unique<SqliteGameCaptureRepository>((dir.path() + "/captures.db").toStdString());
  }
};

TEST_F(GameCaptureRepositoryTest, AddAssignsIdAndLists) {
  auto c = makeCapture("game1", CaptureType::Screenshot, "/a/1.png");
  ASSERT_TRUE(repo->add(c));
  EXPECT_GT(c.id, 0);

  const auto all = repo->listAll();
  ASSERT_EQ(all.size(), 1u);
  EXPECT_EQ(all[0].contentHash, "game1");
  EXPECT_EQ(all[0].type, CaptureType::Screenshot);
}

TEST_F(GameCaptureRepositoryTest, AddIsIdempotentOnFilePath) {
  auto c1 = makeCapture("game1", CaptureType::Screenshot, "/a/1.png");
  auto c2 = makeCapture("game1", CaptureType::Screenshot, "/a/1.png");
  ASSERT_TRUE(repo->add(c1));
  ASSERT_TRUE(repo->add(c2)); // same path -> ignored, id resolved

  EXPECT_EQ(c1.id, c2.id);
  EXPECT_EQ(repo->listAll().size(), 1u);
  EXPECT_TRUE(repo->existsForPath("/a/1.png"));
  EXPECT_FALSE(repo->existsForPath("/a/2.png"));
}

TEST_F(GameCaptureRepositoryTest, ListForGameFilters) {
  auto a = makeCapture("game1", CaptureType::Screenshot, "/a/1.png");
  auto b = makeCapture("game2", CaptureType::Clip, "/b/1.mp4");
  repo->add(a);
  repo->add(b);

  const auto g1 = repo->listForGame("game1");
  ASSERT_EQ(g1.size(), 1u);
  EXPECT_EQ(g1[0].filePath, "/a/1.png");
}

TEST_F(GameCaptureRepositoryTest, SetFavoritePersists) {
  auto c = makeCapture("game1", CaptureType::Screenshot, "/a/1.png");
  repo->add(c);

  ASSERT_TRUE(repo->setFavorite(c.id, true));
  const auto got = repo->getById(c.id);
  ASSERT_TRUE(got.has_value());
  EXPECT_TRUE(got->favorite);
}

TEST_F(GameCaptureRepositoryTest, RemoveDeletes) {
  auto c = makeCapture("game1", CaptureType::Screenshot, "/a/1.png");
  repo->add(c);

  ASSERT_TRUE(repo->remove(c.id));
  EXPECT_TRUE(repo->listAll().empty());
  EXPECT_FALSE(repo->getById(c.id).has_value());
}

} // namespace firelight::media

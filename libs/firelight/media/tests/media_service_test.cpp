#include <firelight/media/media_service.hpp>
#include <firelight/media/sqlite_game_capture_repository.hpp>

#include <QImage>
#include <filesystem>
#include <gtest/gtest.h>
#include <memory>

namespace firelight::media {
namespace {

QString uniqueTempDir(const std::string &name) {
  return QString::fromStdString((std::filesystem::temp_directory_path() / name).string());
}

QImage solidImage(const QColor &color) {
  QImage image(8, 8, QImage::Format_RGBA8888);
  image.fill(color);
  return image;
}

// A MediaService backed by a fresh captures.db in a scratch directory
struct Fixture {
  QString base;
  std::unique_ptr<SqliteGameCaptureRepository> repo;
  std::unique_ptr<MediaService> service;

  explicit Fixture(const QString &dir) : base(dir) {
    std::filesystem::remove_all(base.toStdString());
    std::filesystem::create_directories(base.toStdString());
    repo = std::make_unique<SqliteGameCaptureRepository>((base + "/captures.db").toStdString());
    service = std::make_unique<MediaService>(base, *repo);
  }

  ~Fixture() {
    service.reset();
    repo.reset();
    std::filesystem::remove_all(base.toStdString());
  }
};

} // namespace

TEST(MediaServiceTest, SavesPngUnderContentDir) {
  Fixture f(uniqueTempDir("fl_media_ok"));
  const auto path = f.service->saveScreenshot("abc123", solidImage(Qt::red));

  ASSERT_TRUE(path.has_value());
  EXPECT_TRUE(path->contains("abc123"));
  EXPECT_TRUE(path->contains("/screenshots/"));
  EXPECT_TRUE(path->endsWith(".png"));
  EXPECT_TRUE(std::filesystem::exists(path->toStdString()));
  EXPECT_GT(std::filesystem::file_size(path->toStdString()), 0u);

  QImage reloaded;
  ASSERT_TRUE(reloaded.load(*path));
  EXPECT_EQ(reloaded.width(), 8);
  EXPECT_EQ(reloaded.height(), 8);
}

TEST(MediaServiceTest, IndexesSavedScreenshot) {
  Fixture f(uniqueTempDir("fl_media_index"));
  const auto path = f.service->saveScreenshot("abc123", solidImage(Qt::red));
  ASSERT_TRUE(path.has_value());

  const auto rows = f.repo->listForGame("abc123");
  ASSERT_EQ(rows.size(), 1u);
  EXPECT_EQ(rows[0].type, CaptureType::Screenshot);
  EXPECT_EQ(rows[0].filePath, path->toStdString());
  EXPECT_EQ(rows[0].thumbnailPath, path->toStdString());
}

TEST(MediaServiceTest, RejectsNullImage) {
  Fixture f(uniqueTempDir("fl_media_null"));
  EXPECT_FALSE(f.service->saveScreenshot("abc", QImage{}).has_value());
  EXPECT_TRUE(f.repo->listAll().empty());
}

TEST(MediaServiceTest, RejectsEmptyContentHash) {
  Fixture f(uniqueTempDir("fl_media_empty_hash"));
  EXPECT_FALSE(f.service->saveScreenshot("", solidImage(Qt::blue)).has_value());
}

TEST(MediaServiceTest, SeparateContentHashesGetSeparateDirs) {
  Fixture f(uniqueTempDir("fl_media_multi"));
  const auto a = f.service->saveScreenshot("gameA", solidImage(Qt::green));
  const auto b = f.service->saveScreenshot("gameB", solidImage(Qt::yellow));

  ASSERT_TRUE(a.has_value());
  ASSERT_TRUE(b.has_value());
  EXPECT_TRUE(a->contains("gameA"));
  EXPECT_TRUE(b->contains("gameB"));
  EXPECT_NE(*a, *b);
}

TEST(MediaServiceTest, ReconcileIndexesOrphansAndPrunesMissing) {
  Fixture f(uniqueTempDir("fl_media_reconcile"));

  // A screenshot placed on disk without going through saveScreenshot
  const std::string gameDir = (f.base + "/screenshots/gameX").toStdString();
  std::filesystem::create_directories(gameDir);
  const QString file = QString::fromStdString(gameDir) + "/12345.png";
  ASSERT_TRUE(solidImage(Qt::red).save(file, "PNG"));

  f.service->reconcile();
  auto rows = f.repo->listForGame("gameX");
  ASSERT_EQ(rows.size(), 1u);
  EXPECT_EQ(rows[0].timestamp, 12345);

  // Delete the file behind the app's back; reconcile prunes the row
  std::filesystem::remove(file.toStdString());
  f.service->reconcile();
  EXPECT_TRUE(f.repo->listAll().empty());
}

} // namespace firelight::media

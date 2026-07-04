#include <firelight/media/media_service.hpp>

#include <QImage>
#include <filesystem>
#include <gtest/gtest.h>

namespace firelight::media {
namespace {

QString uniqueTempDir(const std::string &name) {
  return QString::fromStdString(
      (std::filesystem::temp_directory_path() / name).string());
}

QImage solidImage(const QColor &color) {
  QImage image(8, 8, QImage::Format_RGBA8888);
  image.fill(color);
  return image;
}

} // namespace

TEST(MediaServiceTest, SavesPngUnderContentDir) {
  const auto base = uniqueTempDir("fl_media_ok");
  std::filesystem::remove_all(base.toStdString());

  MediaService service(base);
  const auto path = service.saveScreenshot("abc123", solidImage(Qt::red));

  ASSERT_TRUE(path.has_value());
  EXPECT_TRUE(path->contains("abc123"));
  EXPECT_TRUE(path->endsWith(".png"));
  EXPECT_TRUE(std::filesystem::exists(path->toStdString()));
  EXPECT_GT(std::filesystem::file_size(path->toStdString()), 0u);

  // The saved PNG reloads to the same dimensions.
  QImage reloaded;
  ASSERT_TRUE(reloaded.load(*path));
  EXPECT_EQ(reloaded.width(), 8);
  EXPECT_EQ(reloaded.height(), 8);

  std::filesystem::remove_all(base.toStdString());
}

TEST(MediaServiceTest, RejectsNullImage) {
  MediaService service(uniqueTempDir("fl_media_null"));
  EXPECT_FALSE(service.saveScreenshot("abc", QImage{}).has_value());
}

TEST(MediaServiceTest, RejectsEmptyContentHash) {
  MediaService service(uniqueTempDir("fl_media_empty_hash"));
  EXPECT_FALSE(service.saveScreenshot("", solidImage(Qt::blue)).has_value());
}

TEST(MediaServiceTest, SeparateContentHashesGetSeparateDirs) {
  const auto base = uniqueTempDir("fl_media_multi");
  std::filesystem::remove_all(base.toStdString());

  MediaService service(base);
  const auto a = service.saveScreenshot("gameA", solidImage(Qt::green));
  const auto b = service.saveScreenshot("gameB", solidImage(Qt::yellow));

  ASSERT_TRUE(a.has_value());
  ASSERT_TRUE(b.has_value());
  EXPECT_TRUE(a->contains("gameA"));
  EXPECT_TRUE(b->contains("gameB"));
  EXPECT_NE(*a, *b);

  std::filesystem::remove_all(base.toStdString());
}

} // namespace firelight::media

#include "gui/filesystem_utils.hpp"

#include <QTemporaryFile>
#include <QUrl>
#include <gtest/gtest.h>

// Covers FilesystemUtils::readTextFile, which loads bundled help articles for
// the in-app Help section (and any other text resource). The qrc:/ path is
// exercised by the app at runtime (the fl_test binary has no resources linked)
namespace firelight::gui {

// A plain filesystem path round-trips
TEST(FilesystemUtilsTest, ReadsPlainFilePath) {
  QTemporaryFile file;
  ASSERT_TRUE(file.open());
  file.write("hello world");
  file.flush();
  EXPECT_EQ(FilesystemUtils::readTextFile(file.fileName()), QStringLiteral("hello world"));
}

// A file:// URL is resolved to a local path (notably file:///C:/... on Windows)
TEST(FilesystemUtilsTest, ReadsFileUrl) {
  QTemporaryFile file;
  ASSERT_TRUE(file.open());
  file.write("hello world");
  file.flush();
  const QString url = QUrl::fromLocalFile(file.fileName()).toString();
  EXPECT_EQ(FilesystemUtils::readTextFile(url), QStringLiteral("hello world"));
}

// Missing files (plain path or URL) return an empty string, never throw
TEST(FilesystemUtilsTest, MissingFileReturnsEmpty) {
  EXPECT_TRUE(FilesystemUtils::readTextFile("/no/such/file_xyz.md").isEmpty());
  EXPECT_TRUE(FilesystemUtils::readTextFile("file:///no/such/file_xyz.md").isEmpty());
}

} // namespace firelight::gui

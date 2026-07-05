#include "cli/rom_launch.hpp"

#include <firelight/library/sqlite_user_library.hpp>
#include <firelight/library/user_library_service.hpp>

#include <QDir>
#include <QFile>
#include <gtest/gtest.h>

namespace firelight::cli {
namespace {

// A UserLibraryService over an in-memory library (empty), so every resolution
// misses and returns -1 regardless of which branch it takes.
library::UserLibraryService makeService(
    library::SqliteUserLibraryRepository &repo) {
  return library::UserLibraryService(
      repo, (QDir::tempPath() + "/fl_romlaunch_test").toStdString());
}

TEST(RomLaunchTest, EmptyPathReturnsMinusOne) {
  library::SqliteUserLibraryRepository repo(":memory:");
  auto svc = makeService(repo);
  EXPECT_EQ(resolveRomEntryId("", svc), -1);
}

TEST(RomLaunchTest, NonexistentPathReturnsMinusOne) {
  library::SqliteUserLibraryRepository repo(":memory:");
  auto svc = makeService(repo);
  EXPECT_EQ(resolveRomEntryId("/definitely/not/a/real/path.gba", svc), -1);
}

TEST(RomLaunchTest, FileNotInLibraryReturnsMinusOne) {
  // A real (junk) file that isn't in the empty library resolves to -1.
  const QString path = QDir::tempPath() + "/fl_romlaunch_junk.gba";
  QFile file(path);
  ASSERT_TRUE(file.open(QIODevice::WriteOnly));
  file.write(QByteArray(64, '\0'));
  file.close();

  library::SqliteUserLibraryRepository repo(":memory:");
  auto svc = makeService(repo);
  EXPECT_EQ(resolveRomEntryId(path.toStdString(), svc), -1);

  QFile::remove(path);
}

} // namespace
} // namespace firelight::cli

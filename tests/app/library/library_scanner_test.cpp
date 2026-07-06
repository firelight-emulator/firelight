#include <firelight/library/library_scanner2.hpp>

#include <firelight/library/sqlite_user_library.hpp>
#include <firelight/platforms/platform_service.hpp>

#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QTemporaryDir>
#include <QTimer>
#include <archive.h>
#include <archive_entry.h>
#include <gtest/gtest.h>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

// End-to-end scan of a real temp directory: constructs the scanner over an
// in-memory repository, points it at a temp content directory, runs a full scan
// through the QtConcurrent worker + event loop, then asserts on what was
// catalogued. Exercises the directory walk, extension routing, disc-track
// skipping, archive descent, and patch/unknown-file handling together.
namespace firelight::library {

namespace {
QByteArray romBytes(int size, char seed) {
  QByteArray b(size, Qt::Uninitialized);
  for (int i = 0; i < size; ++i) {
    b[i] = static_cast<char>((i * 31 + seed) & 0xFF);
  }
  return b;
}

void writeFile(const QString &path, const QByteArray &bytes) {
  QFile f(path);
  ASSERT_TRUE(f.open(QIODevice::WriteOnly));
  f.write(bytes);
  f.close();
}

bool endsWith(const std::string &s, const std::string &suffix) {
  return s.size() >= suffix.size() &&
         s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0;
}
} // namespace

class LibraryScannerTest : public testing::Test {
protected:
  // An in-memory DB is fine even though the scanner hashes/creates on a worker
  // thread: the repository holds a single SQLite connection shared across threads
  // (serialized by its internal recursive mutex), so the worker sees the same
  // database as the main thread.
  QTemporaryDir tempDir;
  platforms::PlatformService m_platformService;
  std::unique_ptr<SqliteUserLibraryRepository> m_repo;

  void SetUp() override {
    ASSERT_TRUE(tempDir.isValid());
    m_repo = std::make_unique<SqliteUserLibraryRepository>(QString(":memory:"));
  }

  QString path(const QString &rel) { return tempDir.filePath(rel); }

  // Registers the temp dir as a content directory and runs one full scan,
  // blocking on a local event loop until the scanner reports completion.
  void scanTempDir() {
    ContentDirectory dir;
    dir.path = tempDir.path().toStdString();
    ASSERT_TRUE(m_repo->create(dir));

    LibraryScanner2 scanner(*m_repo, m_platformService);
    QEventLoop loop;
    QObject::connect(&scanner, &LibraryScanner2::scanFinished, &loop,
                     &QEventLoop::quit);
    // Safety valve so a hung scan fails the test instead of blocking forever.
    QTimer::singleShot(20000, &loop, &QEventLoop::quit);
    scanner.scanAll();
    loop.exec();
  }

  std::string makeZipWithRom(const QString &zipName, const std::string &entry,
                             const QByteArray &bytes) {
    const std::string zipPath = path(zipName).toStdString();
    archive *a = archive_write_new();
    archive_write_set_format_zip(a);
    EXPECT_EQ(archive_write_open_filename(a, zipPath.c_str()), ARCHIVE_OK);
    archive_entry *e = archive_entry_new();
    archive_entry_set_pathname(e, entry.c_str());
    archive_entry_set_size(e, bytes.size());
    archive_entry_set_filetype(e, AE_IFREG);
    archive_entry_set_perm(e, 0644);
    archive_write_header(a, e);
    archive_write_data(a, bytes.constData(), bytes.size());
    archive_entry_free(e);
    archive_write_close(a);
    archive_write_free(a);
    return zipPath;
  }
};

// Loose cartridge ROMs are catalogued, including those nested in subdirectories;
// unrelated files are ignored.
TEST_F(LibraryScannerTest, DetectsCartridgesRecursively) {
  ASSERT_TRUE(tempDir.isValid());
  writeFile(path("game.gb"), romBytes(1024, 1));
  ASSERT_TRUE(QDir(tempDir.path()).mkpath("sub"));
  writeFile(path("sub/other.gb"), romBytes(1024, 2));
  writeFile(path("notes.txt"), QByteArray("not a rom"));

  scanTempDir();

  const auto files = m_repo->getContentFiles();
  ASSERT_EQ(files.size(), 2u);
  for (const auto &f : files) {
    EXPECT_EQ(f.m_platformId,
              platforms::PlatformService::PLATFORM_ID_GAMEBOY);
    EXPECT_EQ(f.m_type, ContentType::Cartridge);
    EXPECT_FALSE(f.m_inArchive);
    EXPECT_FALSE(endsWith(f.m_filePath, "notes.txt"));
  }
}

// A raw disc track (.bin) sitting next to its cue sheet is pulled in via the
// sheet, so the lone track is never catalogued on its own.
TEST_F(LibraryScannerTest, SkipsRawTrackWhenSheetPresent) {
  ASSERT_TRUE(tempDir.isValid());
  ASSERT_TRUE(QDir(tempDir.path()).mkpath("disc"));
  writeFile(path("disc/game.bin"), romBytes(2048, 5));
  writeFile(path("disc/game.cue"),
            QByteArray("FILE \"game.bin\" BINARY\n  TRACK 01 MODE1/2048\n"
                       "    INDEX 01 00:00:00\n"));

  scanTempDir();

  for (const auto &f : m_repo->getContentFiles()) {
    EXPECT_FALSE(endsWith(f.m_filePath, "game.bin"))
        << "raw track should not be catalogued while its cue sheet exists";
  }
}

// A ROM stored inside a .zip is catalogued as archived content, recording the
// inner entry name and the archive path.
TEST_F(LibraryScannerTest, DetectsRomInsideArchive) {
  ASSERT_TRUE(tempDir.isValid());
  const std::string zipPath =
      makeZipWithRom("games.zip", "inner.gb", romBytes(1500, 9));

  scanTempDir();

  const auto files = m_repo->getContentFiles();
  ASSERT_EQ(files.size(), 1u);
  EXPECT_TRUE(files[0].m_inArchive);
  EXPECT_EQ(files[0].m_filePath, "inner.gb");
  EXPECT_EQ(files[0].m_archivePathName, zipPath);
  EXPECT_EQ(files[0].m_platformId,
            platforms::PlatformService::PLATFORM_ID_GAMEBOY);
}

// Unknown extensions and patch files are not catalogued as content (a patch is
// stored separately via the patch path, never as a ContentFile).
TEST_F(LibraryScannerTest, IgnoresUnknownAndPatchFiles) {
  ASSERT_TRUE(tempDir.isValid());
  writeFile(path("readme.txt"), QByteArray("hello"));
  writeFile(path("game.srm"), romBytes(256, 3));
  writeFile(path("hack.ips"), QByteArray("PATCH", 5) + QByteArray("EOF", 3));

  scanTempDir();

  EXPECT_TRUE(m_repo->getContentFiles().empty());
}

} // namespace firelight::library

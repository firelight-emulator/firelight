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
#include <cstdint>
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>

// End-to-end scan of a real temp directory: constructs the scanner over an
// in-memory repository, points it at a temp content directory, runs a full scan
// through the QtConcurrent worker + event loop, then asserts on what was
// catalogued. Exercises the directory walk, extension routing, disc-track
// skipping, archive descent, and patch/unknown-file handling together
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
  return s.size() >= suffix.size() && s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0;
}
} // namespace

class LibraryScannerTest : public testing::Test {
protected:
  // An in-memory DB is fine even though the scanner hashes/creates on a worker
  // thread: the repository holds a single SQLite connection shared across threads
  // (serialized by its internal recursive mutex), so the worker sees the same
  // database as the main thread
  QTemporaryDir tempDir;
  platforms::PlatformService m_platformService;
  std::unique_ptr<SqliteUserLibraryRepository> m_repo;

  void SetUp() override {
    ASSERT_TRUE(tempDir.isValid());
    m_repo = std::make_unique<SqliteUserLibraryRepository>(QString(":memory:"));
  }

  QString path(const QString &rel) { return tempDir.filePath(rel); }

  // Runs a scan and blocks until the worker goes idle, whether or not it changed
  // anything. scanFinished only fires on changes, so wait on the scanning flag
  // returning to false instead (the worker's DB writes are complete by then)
  static void waitForScanIdle(LibraryScanner2 &scanner) {
    QEventLoop loop;
    QObject::connect(&scanner, &LibraryScanner2::scanningChanged, &loop, [&] {
      if (!scanner.isScanning()) {
        loop.quit();
      }
    });
    // Safety valve so a hung scan fails the test instead of blocking forever
    QTimer::singleShot(20000, &loop, &QEventLoop::quit);
    scanner.scanAll();
    loop.exec();
  }

  // Registers the temp dir as a content directory and runs one full scan
  void scanTempDir() {
    ContentDirectory dir;
    dir.path = tempDir.path().toStdString();
    ASSERT_TRUE(m_repo->create(dir));

    LibraryScanner2 scanner(*m_repo, m_platformService);
    waitForScanIdle(scanner);
  }

  std::unique_ptr<LibraryScanner2> makeScanner() {
    ContentDirectory dir;
    dir.path = tempDir.path().toStdString();
    m_repo->create(dir);
    return std::make_unique<LibraryScanner2>(*m_repo, m_platformService);
  }

  // Runs a scan and blocks until scanFinished (which only fires when the scan
  // changed something). Use only when the scan is expected to add/remove content
  static void waitForScanFinished(LibraryScanner2 &scanner) {
    QEventLoop loop;
    QObject::connect(&scanner, &LibraryScanner2::scanFinished, &loop, &QEventLoop::quit);
    QTimer::singleShot(20000, &loop, &QEventLoop::quit);
    scanner.scanAll();
    loop.exec();
  }

  // Pumps the event loop for a fixed spell (to let a background scan run when we
  // expect it NOT to emit scanFinished, or not to run at all)
  static void pumpEvents(int ms) {
    QEventLoop loop;
    QTimer::singleShot(ms, &loop, &QEventLoop::quit);
    loop.exec();
  }

  std::string makeZipWithRom(const QString &zipName, const std::string &entry, const QByteArray &bytes) {
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
// unrelated files are ignored
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
    EXPECT_EQ(f.m_platformId, platforms::PlatformService::PLATFORM_ID_GAMEBOY);
    EXPECT_EQ(f.m_type, ContentType::Cartridge);
    EXPECT_FALSE(f.m_inArchive);
    EXPECT_FALSE(endsWith(f.m_filePath, "notes.txt"));
  }
}

// A raw disc track (.bin) named by a cue sheet beside it is reached through the sheet, so the
// lone track is not catalogued on its own. The disc has to actually identify, or the assertion
// below passes by finding nothing at all
TEST_F(LibraryScannerTest, SkipsRawTrackTheSheetNames) {
  ASSERT_TRUE(tempDir.isValid());
  ASSERT_TRUE(QDir(tempDir.path()).mkpath("disc"));

  auto image = romBytes(65536, 5);
  image.replace(0, 16, QByteArray("SEGADISCSYSTEM  "));
  writeFile(path("disc/game.bin"), image);
  writeFile(path("disc/game.cue"), QByteArray("FILE \"game.bin\" BINARY\n  TRACK 01 MODE1/2048\n"
                                              "    INDEX 01 00:00:00\n"));

  scanTempDir();

  const auto files = m_repo->getContentFiles();
  ASSERT_EQ(files.size(), 1u) << "the disc did not identify, so this proves nothing about suppression";
  EXPECT_TRUE(endsWith(files.front().m_filePath, "game.cue"));
}

// Suppression used to fire when any sheet existed in the directory rather than when one named
// the file, so a cartridge sharing a folder with an unrelated disc was hidden
TEST_F(LibraryScannerTest, KeepsARawFileNoSheetNames) {
  ASSERT_TRUE(tempDir.isValid());
  ASSERT_TRUE(QDir(tempDir.path()).mkpath("mixed"));

  auto image = romBytes(65536, 5);
  image.replace(0, 16, QByteArray("SEGADISCSYSTEM  "));
  writeFile(path("mixed/game.bin"), image);
  writeFile(path("mixed/game.cue"), QByteArray("FILE \"game.bin\" BINARY\n  TRACK 01 MODE1/2048\n"
                                               "    INDEX 01 00:00:00\n"));

  // A Mega Drive cartridge that the cue says nothing about
  auto cartridge = romBytes(524288, 9);
  cartridge.replace(0x100, 16, QByteArray("SEGA MEGA DRIVE "));
  writeFile(path("mixed/Sonic.bin"), cartridge);

  scanTempDir();

  const auto files = m_repo->getContentFiles();
  ASSERT_EQ(files.size(), 2u) << "the cartridge was hidden by a cue sheet that never mentioned it";

  auto sawCartridge = false;
  for (const auto &file : files) {
    sawCartridge = sawCartridge || endsWith(file.m_filePath, "Sonic.bin");
  }
  EXPECT_TRUE(sawCartridge);
}

// A ROM stored inside a .zip is catalogued as archived content, recording the
// inner entry name and the archive path
TEST_F(LibraryScannerTest, DetectsRomInsideArchive) {
  ASSERT_TRUE(tempDir.isValid());
  const std::string zipPath = makeZipWithRom("games.zip", "inner.gb", romBytes(1500, 9));

  scanTempDir();

  const auto files = m_repo->getContentFiles();
  ASSERT_EQ(files.size(), 1u);
  EXPECT_TRUE(files[0].m_inArchive);
  EXPECT_EQ(files[0].m_filePath, "inner.gb");
  EXPECT_EQ(files[0].m_archivePathName, zipPath);
  EXPECT_EQ(files[0].m_platformId, platforms::PlatformService::PLATFORM_ID_GAMEBOY);
}

// Unknown extensions and patch files are not catalogued as content (a patch is
// stored separately via the patch path, never as a ContentFile)
TEST_F(LibraryScannerTest, IgnoresUnknownAndPatchFiles) {
  ASSERT_TRUE(tempDir.isValid());
  writeFile(path("readme.txt"), QByteArray("hello"));
  writeFile(path("game.srm"), romBytes(256, 3));
  writeFile(path("hack.ips"), QByteArray("PATCH", 5) + QByteArray("EOF", 3));

  scanTempDir();

  EXPECT_TRUE(m_repo->getContentFiles().empty());
}

// Nested folders that hold games are watched (so future adds/removes there are
// caught instantly), along with the content root -- but folders with no games
// are not, so the watch count scales with game folders, not the whole tree
TEST_F(LibraryScannerTest, WatchesGameFoldersAndRootButNotEmptyFolders) {
  ASSERT_TRUE(tempDir.isValid());
  ASSERT_TRUE(QDir(tempDir.path()).mkpath("snes"));
  writeFile(path("snes/game.sfc"), romBytes(1024, 1));
  ASSERT_TRUE(QDir(tempDir.path()).mkpath("empty/sub")); // holds no games

  auto scanner = makeScanner();
  waitForScanFinished(*scanner); // adds a ROM -> fires after watches are applied

  const auto watched = scanner->watchedDirectories();
  bool rootWatched = false, snesWatched = false, emptyWatched = false;
  for (const auto &d : watched) {
    if (d == tempDir.path()) {
      rootWatched = true;
    }
    if (endsWith(d.toStdString(), "/snes")) {
      snesWatched = true;
    }
    if (d.contains("/empty")) {
      emptyWatched = true;
    }
  }
  EXPECT_TRUE(rootWatched);
  EXPECT_TRUE(snesWatched);
  EXPECT_FALSE(emptyWatched);
}

// A rescan with no filesystem change must not report a change: scanFinished
// (which refreshes/resets the library UI) stays silent, and nothing is
// duplicated. This is what keeps the periodic safety scan from flickering the UI
TEST_F(LibraryScannerTest, NoOpRescanDoesNotEmitScanFinished) {
  ASSERT_TRUE(tempDir.isValid());
  writeFile(path("game.gb"), romBytes(1024, 4));

  auto scanner = makeScanner();
  waitForScanFinished(*scanner); // first scan catalogs the ROM
  ASSERT_EQ(m_repo->getContentFiles().size(), 1u);

  bool finishedAgain = false;
  QObject::connect(scanner.get(), &LibraryScanner2::scanFinished, [&] { finishedAgain = true; });
  scanner->scanAll();
  pumpEvents(1500);

  EXPECT_FALSE(finishedAgain);
  EXPECT_EQ(m_repo->getContentFiles().size(), 1u);
}

// While suspended (e.g. a game is running) no scan runs, even if directories
// change; resuming processes the deferred work
TEST_F(LibraryScannerTest, SuspendedScannerDefersUntilResumed) {
  ASSERT_TRUE(tempDir.isValid());
  writeFile(path("game.gb"), romBytes(1024, 6));

  auto scanner = makeScanner();
  scanner->setScanningSuspended(true);
  scanner->scanAll();
  pumpEvents(500);

  EXPECT_TRUE(m_repo->getContentFiles().empty());
  EXPECT_FALSE(scanner->isScanning());

  // Resume -> the deferred scan runs and finds the ROM
  bool finished = false;
  QObject::connect(scanner.get(), &LibraryScanner2::scanFinished, [&] { finished = true; });
  QEventLoop loop;
  QObject::connect(scanner.get(), &LibraryScanner2::scanFinished, &loop, &QEventLoop::quit);
  QTimer::singleShot(20000, &loop, &QEventLoop::quit);
  scanner->setScanningSuspended(false);
  loop.exec();

  EXPECT_TRUE(finished);
  EXPECT_EQ(m_repo->getContentFiles().size(), 1u);
}

//****************
// unreachable roots
//****************

// An unmounted drive answers exactly as a deleted file does, so a sweep that believes it wipes the
// catalogue of everything on that drive: every content file, every way in, every disc membership,
// unattended, on the periodic rescan
TEST_F(LibraryScannerTest, AnUnreachableRootLosesNothing) {
  const auto gamesDir = path("Games");
  ASSERT_TRUE(QDir().mkpath(gamesDir));
  writeFile(gamesDir + "/One.gb", romBytes(32768, 1));
  writeFile(gamesDir + "/Two.gb", romBytes(32768, 2));

  ContentDirectory dir;
  dir.path = gamesDir.toStdString();
  ASSERT_TRUE(m_repo->create(dir));

  LibraryScanner2 scanner(*m_repo, m_platformService);
  waitForScanIdle(scanner);

  const auto catalogued = m_repo->getContentFiles();
  ASSERT_EQ(catalogued.size(), 2u) << "nothing was catalogued, so the assertion below proves nothing";

  // The whole root goes, which is what an unplugged drive looks like from here
  ASSERT_TRUE(QDir(gamesDir).removeRecursively());

  waitForScanIdle(scanner);

  // Everything hangs off the content file: deleting one takes its run configurations and disc
  // memberships with it, so the row surviving is what the rest survives by
  const auto after = m_repo->getContentFiles();
  EXPECT_EQ(after.size(), 2u) << "the catalogue was swept away with the drive";

  for (const auto &file : catalogued) {
    EXPECT_TRUE(std::ranges::any_of(after, [&](const ContentFile &kept) { return kept.m_id == file.m_id; }))
        << file.m_filePath << " was deleted";
  }
}

// The other half, and the one that keeps the fix honest: "never notice anything" would pass the
// test above on its own. The row stays so the path can be named, but it stops counting as content
TEST_F(LibraryScannerTest, AMissingFileUnderALiveRootIsMarkedMissing) {
  writeFile(path("One.gb"), romBytes(32768, 1));
  writeFile(path("Two.gb"), romBytes(32768, 2));

  scanTempDir();
  ASSERT_EQ(m_repo->getContentFiles().size(), 2u);

  ASSERT_TRUE(QFile::remove(path("One.gb")));

  LibraryScanner2 scanner(*m_repo, m_platformService);
  waitForScanIdle(scanner);

  const auto present = m_repo->getPresentContentFiles();
  ASSERT_EQ(present.size(), 1u) << "a file that really did go still counts as content";
  EXPECT_TRUE(endsWith(present.front().m_filePath, "Two.gb"));

  const auto all = m_repo->getContentFiles();
  ASSERT_EQ(all.size(), 2u) << "the row was destroyed, so nothing can say where the file was";

  const auto gone =
      std::ranges::find_if(all, [](const ContentFile &file) { return endsWith(file.m_filePath, "One.gb"); });
  ASSERT_NE(gone, all.end());
  EXPECT_NE(gone->m_missingSince, 0);
}

// Putting the file back is the other direction, and nothing else clears the mark
TEST_F(LibraryScannerTest, AFileThatComesBackStopsBeingMissing) {
  writeFile(path("One.gb"), romBytes(32768, 1));
  scanTempDir();

  const auto originalId = m_repo->getContentFiles().front().m_id;
  ASSERT_TRUE(QFile::remove(path("One.gb")));

  {
    LibraryScanner2 scanner(*m_repo, m_platformService);
    waitForScanIdle(scanner);
  }
  ASSERT_TRUE(m_repo->getPresentContentFiles().empty());

  writeFile(path("One.gb"), romBytes(32768, 1));

  LibraryScanner2 scanner(*m_repo, m_platformService);
  waitForScanIdle(scanner);

  const auto present = m_repo->getPresentContentFiles();
  ASSERT_EQ(present.size(), 1u) << "the file came back and nothing noticed";
  EXPECT_EQ(present.front().m_id, originalId) << "it was re-catalogued as a new row instead of revived";
}

// Nothing vouches for a file belonging to no content directory, so a root cannot shelter it
TEST_F(LibraryScannerTest, AFileUnderNoContentDirectoryIsStillJudged) {
  writeFile(path("One.gb"), romBytes(32768, 1));
  scanTempDir();
  ASSERT_EQ(m_repo->getPresentContentFiles().size(), 1u);

  ContentFile orphan;
  orphan.m_filePath = tempDir.filePath("Gone.gb").toStdString();
  orphan.m_contentHash = "orphanhash";
  orphan.m_fileSizeBytes = 32768;
  orphan.m_contentDirectoryId = -1;
  ASSERT_TRUE(m_repo->create(orphan));
  ASSERT_EQ(m_repo->getPresentContentFiles().size(), 2u);

  LibraryScanner2 scanner(*m_repo, m_platformService);
  waitForScanIdle(scanner);

  EXPECT_EQ(m_repo->getPresentContentFiles().size(), 1u) << "a file belonging to no directory was passed over";
}

} // namespace firelight::library

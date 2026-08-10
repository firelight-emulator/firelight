#include <firelight/library/library_ingest_service.hpp>
#include <firelight/library/library_scanner2.hpp>
#include <firelight/library/sqlite_user_library.hpp>
#include <firelight/platforms/platform_service.hpp>

#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QTimer>
#include <algorithm>
#include <archive.h>
#include <archive_entry.h>
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>

// Files on disk all the way to library entries, through the real scanner and the real ingest
// service. The layer tests either side of this seam both pass with hand-built inputs, so this
// is the only place the pipeline is exercised as one thing
namespace firelight::library {

namespace {
// Sector-0 bytes rcheevos recognises as a Sega CD disc, so a synthetic image identifies as a
// real disc instead of falling through every fingerprint
QByteArray seededSegaCdImage(const char seed) {
  QByteArray bytes(65536, Qt::Uninitialized);
  for (int i = 0; i < bytes.size(); ++i) {
    bytes[i] = static_cast<char>((i * 17 + seed) & 0xFF);
  }
  const QByteArray magic("SEGADISCSYSTEM  ");
  bytes.replace(0, magic.size(), magic);
  return bytes;
}

QByteArray discImageBytes(const QByteArray &image) { return image; }

QByteArray romBytes(const int size, const char seed) {
  QByteArray bytes(size, Qt::Uninitialized);
  for (int i = 0; i < size; ++i) {
    bytes[i] = static_cast<char>((i * 31 + seed) & 0xFF);
  }
  return bytes;
}
} // namespace

class LibraryEndToEndTest : public testing::Test {
protected:
  QTemporaryDir m_tempDir;
  platforms::PlatformService m_platformService;
  std::unique_ptr<SqliteUserLibraryRepository> m_repo;
  // Entries exist only because this is alive: the scanner writes content files and nothing
  // else. Its absence is why no other test reaches an entry from a real file
  std::unique_ptr<LibraryIngestService> m_ingest;

  void SetUp() override {
    ASSERT_TRUE(m_tempDir.isValid());
    m_repo = std::make_unique<SqliteUserLibraryRepository>(QString(":memory:"));
    m_ingest = std::make_unique<LibraryIngestService>(*m_repo);
  }

  QString path(const QString &relative) const { return m_tempDir.filePath(relative); }

  void write(const QString &relative, const QByteArray &bytes) const {
    const QFileInfo info(path(relative));
    QDir().mkpath(info.absolutePath());
    QFile file(path(relative));
    ASSERT_TRUE(file.open(QIODevice::WriteOnly));
    file.write(bytes);
    file.close();
  }

  void remove(const QString &relative) const { ASSERT_TRUE(QFile::remove(path(relative))); }

  // scanFinished only fires when something changed, so an idempotent rescan would wait forever
  // on it. The scanning flag returning to false is true either way, and the worker's writes are
  // complete by then
  static void waitForScanIdle(LibraryScanner2 &scanner) {
    QEventLoop loop;
    QObject::connect(&scanner, &LibraryScanner2::scanningChanged, &loop, [&] {
      if (!scanner.isScanning()) {
        loop.quit();
      }
    });
    QTimer::singleShot(20000, &loop, &QEventLoop::quit);
    scanner.scanAll();
    loop.exec();
  }

  // One scanner per scan, so the periodic rescan timer and the watcher debounce cannot fire
  // into a later assertion
  void scan() {
    LibraryScanner2 scanner(*m_repo, m_platformService);
    waitForScanIdle(scanner);
  }

  void registerContentDirectory() {
    ContentDirectory directory;
    directory.path = m_tempDir.path().toStdString();
    ASSERT_TRUE(m_repo->create(directory));
  }

  std::vector<Entry> entries() { return m_repo->getEntries(0, -1); }

  std::optional<Entry> entryNamed(const std::string &displayName) {
    for (const auto &entry : entries()) {
      if (entry.displayName == displayName) {
        return entry;
      }
    }
    return std::nullopt;
  }

  // The whole shape in one line, which is what catches a second content file or a stray way in
  std::string shape() {
    return std::to_string(entries().size()) + " entries, " + std::to_string(m_repo->getContentFiles().size()) +
           " files, " + std::to_string(runConfigurationCount()) + " configs";
  }

  size_t runConfigurationCount() {
    size_t total = 0;
    for (const auto &entry : entries()) {
      total += m_repo->getRunConfigurations(entry.contentHash).size();
    }
    return total;
  }
};

// The seam nothing covered: a real file becomes a row a user can see, named after itself
TEST_F(LibraryEndToEndTest, ALooseRomOnDiskBecomesAVisibleEntry) {
  write("Tetris.gb", romBytes(32768, 3));
  registerContentDirectory();

  scan();

  ASSERT_EQ(entries().size(), 1u);
  const auto entry = entries().front();
  EXPECT_EQ(entry.displayName, "Tetris.gb");
  EXPECT_FALSE(entry.hidden);
  EXPECT_FALSE(entry.contentHash.empty());
  EXPECT_EQ(entry.platformId, platforms::PlatformService::PLATFORM_ID_GAMEBOY);
  EXPECT_EQ(m_repo->getRunConfigurations(entry.contentHash).size(), 1u);
}

// A rescan runs constantly once folder watching is on, so churn here is churn a user sees
TEST_F(LibraryEndToEndTest, RescanningChangesNothing) {
  write("Tetris.gb", romBytes(32768, 3));
  write("games/Zelda.gb", romBytes(65536, 9));
  registerContentDirectory();

  scan();
  const auto afterFirst = shape();
  const auto firstIds = entries().front().id;

  scan();
  scan();

  EXPECT_EQ(shape(), afterFirst);
  EXPECT_EQ(entries().front().id, firstIds) << "the entry was recreated rather than left alone";
}

// The tenet, from disk rather than from hand-published events: what a user built on an entry
// has to survive the file moving
TEST_F(LibraryEndToEndTest, MovingAFileKeepsTheEntryAndEverythingOnIt) {
  write("Chrono Trigger.sfc", romBytes(131072, 11));
  registerContentDirectory();
  scan();

  auto entry = entries().front();
  const auto originalId = entry.id;
  const auto originalHash = entry.contentHash;
  entry.displayName = "Chrono Trigger";
  entry.nameUserSet = true;
  entry.favorite = true;
  entry.rating = 5;
  ASSERT_TRUE(m_repo->update(entry));

  // The same bytes, somewhere else entirely
  const auto bytes = romBytes(131072, 11);
  remove("Chrono Trigger.sfc");
  write("rpgs/moved/Chrono Trigger.sfc", bytes);
  scan();

  const auto moved = m_repo->getEntryWithContentHash(originalHash);
  ASSERT_TRUE(moved.has_value());
  EXPECT_EQ(moved->id, originalId);
  EXPECT_EQ(moved->displayName, "Chrono Trigger");
  EXPECT_TRUE(moved->favorite);
  EXPECT_EQ(moved->rating, 5u);
  EXPECT_FALSE(moved->hidden) << "the game is on disk, so it must not be hidden";
  EXPECT_EQ(entries().size(), 1u) << "the move created a second entry";
}

// A game whose file goes away keeps its row and everything on it, and comes back to the same
// one. The hidden flag is how that is signalled today; when availability lands, this assertion
// moves to it and hidden goes back to meaning only what a user chose
TEST_F(LibraryEndToEndTest, AMissingFileKeepsItsEntryAndReaddingRestoresIt) {
  write("Metroid.nes", romBytes(40960, 5));
  registerContentDirectory();
  scan();

  const auto originalId = entries().front().id;

  remove("Metroid.nes");
  scan();

  ASSERT_EQ(entries().size(), 1u) << "the entry was deleted rather than hidden";
  EXPECT_TRUE(entries().front().hidden);

  write("Metroid.nes", romBytes(40960, 5));
  scan();

  ASSERT_EQ(entries().size(), 1u);
  EXPECT_FALSE(entries().front().hidden);
  EXPECT_EQ(entries().front().id, originalId);
}

// Two dumps of one game in two folders are one game, because identity is the content
TEST_F(LibraryEndToEndTest, TheSameRomInTwoFoldersIsOneEntryWithTwoWaysIn) {
  const auto bytes = romBytes(65536, 21);
  write("us/Kirby.gb", bytes);
  write("backup/Kirby.gb", bytes);
  registerContentDirectory();

  scan();

  ASSERT_EQ(entries().size(), 1u);
  EXPECT_EQ(m_repo->getContentFiles().size(), 2u);
  EXPECT_EQ(m_repo->getRunConfigurations(entries().front().contentHash).size(), 2u);
}

// A ROM inside an archive still becomes a playable-looking entry
TEST_F(LibraryEndToEndTest, ARomInsideAnArchiveBecomesAnEntry) {
  const auto inner = romBytes(32768, 7);
  const auto zipPath = path("games.zip").toStdString();

  {
    archive *writer = archive_write_new();
    archive_write_set_format_zip(writer);
    ASSERT_EQ(archive_write_open_filename(writer, zipPath.c_str()), ARCHIVE_OK);
    archive_entry *entry = archive_entry_new();
    archive_entry_set_pathname(entry, "Pokemon Red.gb");
    archive_entry_set_size(entry, inner.size());
    archive_entry_set_filetype(entry, AE_IFREG);
    archive_entry_set_perm(entry, 0644);
    archive_write_header(writer, entry);
    archive_write_data(writer, inner.constData(), inner.size());
    archive_entry_free(entry);
    archive_write_close(writer);
    archive_write_free(writer);
  }

  registerContentDirectory();
  scan();

  ASSERT_EQ(entries().size(), 1u);
  EXPECT_EQ(entries().front().displayName, "Pokemon Red.gb");
  EXPECT_FALSE(entries().front().hidden);
}

// A file nothing can identify must not become a game, and must not stop the ones that can
TEST_F(LibraryEndToEndTest, UnknownFilesAreNotCatalogued) {
  write("readme.txt", QByteArray("notes"));
  write("save.srm", romBytes(2048, 1));
  write("patch.ips", QByteArray("PATCH"));
  write("Tetris.gb", romBytes(32768, 3));
  registerContentDirectory();

  scan();

  ASSERT_EQ(entries().size(), 1u);
  EXPECT_EQ(entries().front().displayName, "Tetris.gb");
}

// rcheevos has no handler for these, and its fallback is a whole-file Game Boy hash. Filing a
// compressed PSP image as a Game Boy game scatters somebody's library into the wrong platform
// and gives it a hash that means nothing. One test per format, so a failure names it
class UnreadableDiscFormatTest : public LibraryEndToEndTest, public testing::WithParamInterface<const char *> {};

TEST_P(UnreadableDiscFormatTest, IsNotFiledAsGameBoy) {
  write(QString("Game.") + GetParam(), romBytes(65536, 13));
  registerContentDirectory();

  scan();

  for (const auto &entry : entries()) {
    EXPECT_NE(entry.platformId, platforms::PlatformService::PLATFORM_ID_GAMEBOY)
        << entry.displayName << " was filed as a Game Boy game";
  }
}

INSTANTIATE_TEST_SUITE_P(DiscFormatsRcheevosCannotRead, UnreadableDiscFormatTest,
                         testing::Values("cso", "ccd", "img", "mdf", "nrg"));

// A .sbi sits beside a LibCrypt-protected PS1 disc. It is data the emulator needs, not a game
TEST_F(LibraryEndToEndTest, DiscSidecarFilesAreNotCataloguedAsGames) {
  write("Ape Escape.sbi", romBytes(1024, 23));
  write("Ape Escape.lsd", romBytes(1024, 29));
  write("Tetris.gb", romBytes(32768, 3));
  registerContentDirectory();

  scan();

  ASSERT_EQ(entries().size(), 1u) << "a sidecar became a library entry";
  EXPECT_EQ(entries().front().displayName, "Tetris.gb");
}

// Identity is the content, not the name. Renaming a dump, or keeping a second copy under a
// different name, must not split one game into two rows with two sets of saves
TEST_F(LibraryEndToEndTest, TheSameBytesUnderDifferentNamesAreOneGame) {
  const auto bytes = romBytes(65536, 33);
  write("Super Mario Land.gb", bytes);
  write("smb (headered dump).gb", bytes);
  registerContentDirectory();

  scan();

  ASSERT_EQ(entries().size(), 1u);
  EXPECT_EQ(m_repo->getContentFiles().size(), 2u);
}

// A cue+bin pair is one disc, not a disc plus a mystery ROM. The raw track is suppressed
// because the sheet beside it is what addresses it
TEST_F(LibraryEndToEndTest, ACueAndItsTrackAreOneDiscNotTwoEntries) {
  write("Sonic CD (Track 1).bin", discImageBytes(seededSegaCdImage(41)));
  write("Sonic CD.cue", QByteArray("FILE \"Sonic CD (Track 1).bin\" BINARY\n  TRACK 01 MODE1/2048\n"
                                   "    INDEX 01 00:00:00\n"));
  registerContentDirectory();

  scan();

  ASSERT_EQ(entries().size(), 1u) << "the raw track was catalogued as its own game";
  const auto files = m_repo->getContentFiles();
  ASSERT_EQ(files.size(), 1u);
  EXPECT_EQ(files.front().m_type, ContentType::Disc);
}

// The link nothing covered: the scanner parses the disc number off a real filename, and the
// disc grouper reads it from the content file. Both halves are tested; the join was not
TEST_F(LibraryEndToEndTest, DiscNumbersAreParsedFromRealFilenamesOntoContentFiles) {
  write("Final Fantasy VII (USA) (Disc 1).cue",
        QByteArray("FILE \"ff7d1.bin\" BINARY\n  TRACK 01 MODE1/2048\n    INDEX 01 00:00:00\n"));
  write("ff7d1.bin", discImageBytes(seededSegaCdImage(51)));
  write("Final Fantasy VII (USA) (Disc 2).cue",
        QByteArray("FILE \"ff7d2.bin\" BINARY\n  TRACK 01 MODE1/2048\n    INDEX 01 00:00:00\n"));
  write("ff7d2.bin", discImageBytes(seededSegaCdImage(52)));
  registerContentDirectory();

  scan();

  std::vector<int> discNumbers;
  for (const auto &file : m_repo->getContentFiles()) {
    discNumbers.push_back(file.m_discNumber);
  }
  std::ranges::sort(discNumbers);

  EXPECT_EQ(discNumbers, (std::vector<int>{1, 2})) << "the scanner did not carry the disc tag onto the content file";
}

} // namespace firelight::library

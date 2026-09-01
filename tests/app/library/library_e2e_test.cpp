// TODO: NEEDS REVIEW
#include <firelight/library/content_identifier.hpp>
#include <firelight/library/content_loader.hpp>
#include <firelight/library/disc_set_service.hpp>
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
#include <cstring>
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

// A Mega Drive cartridge carries its console name at 0x100. rcheevos has no handler that reads
// this, which is why a .bin of one used to fall through to a whole-file fallback
QByteArray genesisCartridgeBytes(const char *consoleName, const char seed) {
  QByteArray bytes(524288, Qt::Uninitialized);
  for (int i = 0; i < bytes.size(); ++i) {
    bytes[i] = static_cast<char>((i * 13 + seed) & 0xFF);
  }
  bytes.replace(0x100, static_cast<int>(std::strlen(consoleName)), consoleName);
  return bytes;
}

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

  // TODO
  // Where a set's playlist is written, kept out of the scanned folder so the artifact a launch
  // needs is never itself scanned back in as content
  QTemporaryDir m_appDataDir;

  platforms::PlatformService m_platformService;
  std::unique_ptr<SqliteUserLibraryRepository> m_repo;
  // Entries exist only because this is alive: the scanner writes content files and nothing
  // else. Its absence is why no other test reaches an entry from a real file
  std::unique_ptr<DiscSetService> m_discSets;
  std::unique_ptr<LibraryIngestService> m_ingest;

  void SetUp() override {
    ASSERT_TRUE(m_tempDir.isValid());
    ASSERT_TRUE(m_appDataDir.isValid());
    m_repo = std::make_unique<SqliteUserLibraryRepository>(QString(":memory:"));
    m_discSets = std::make_unique<DiscSetService>(*m_repo, m_appDataDir.path().toStdString());
    m_ingest = std::make_unique<LibraryIngestService>(*m_repo, *m_discSets);
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

  // TODO
  // A one-entry zip, which is what every archive case here needs
  void writeArchive(const QString &relative, const std::string &entryName, const QByteArray &bytes) const {
    const auto archivePath = path(relative).toStdString();
    archive *writer = archive_write_new();
    archive_write_set_format_zip(writer);
    ASSERT_EQ(archive_write_open_filename(writer, archivePath.c_str()), ARCHIVE_OK);

    archive_entry *entry = archive_entry_new();
    archive_entry_set_pathname(entry, entryName.c_str());
    archive_entry_set_size(entry, bytes.size());
    archive_entry_set_filetype(entry, AE_IFREG);
    archive_entry_set_perm(entry, 0644);
    archive_write_header(writer, entry);
    archive_write_data(writer, bytes.constData(), bytes.size());
    archive_entry_free(entry);
    archive_write_close(writer);
    archive_write_free(writer);
  }

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
    LibraryScanner2 scanner(*m_repo, m_platformService, nullptr, m_discSets.get());
    waitForScanIdle(scanner);
  }

  void registerContentDirectory() {
    ContentDirectory directory;
    directory.path = m_tempDir.path().toStdString();
    ASSERT_TRUE(m_repo->create(directory));
  }

  std::vector<Entry> entries() { return m_repo->getEntries(); }

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

  // TODO
  // A disc as it sits on disk: a sheet naming the raw track beside it, which is what the walk
  // has to reach through
  void writeDisc(const QString &baseName, const char seed) const {
    write(baseName + ".bin", discImageBytes(seededSegaCdImage(seed)));

    const auto trackName = QFileInfo(baseName).fileName() + ".bin";
    write(baseName + ".cue", QByteArray("FILE \"") + trackName.toUtf8() +
                                 QByteArray("\" BINARY\n  TRACK 01 MODE1/2048\n    INDEX 01 00:00:00\n"));
  }

  // Where the set launching under this identity keeps its playlist
  std::string playlistFor(const std::string &contentHash) const {
    return playlistPathFor(contentHash, m_appDataDir.path().toStdString());
  }

  // TODO
  // What a playlist names, without the line saying the file is ours
  static std::vector<std::string> discLinesOf(const std::string &playlistPath) {
    QFile file(QString::fromStdString(playlistPath));

    if (!file.open(QIODevice::ReadOnly)) {
      return {};
    }

    std::vector<std::string> lines;

    for (const auto &line : QString::fromUtf8(file.readAll()).split('\n')) {
      const auto trimmed = line.trimmed();

      if (!trimmed.isEmpty() && !trimmed.startsWith('#')) {
        lines.push_back(trimmed.toStdString());
      }
    }

    return lines;
  }

  // TODO
  // The discs of the set behind an entry, in the order membership holds them
  std::vector<DiscSetMember> membersOf(const Entry &entry) {
    return entry.discSetId.has_value() ? m_repo->getDiscSetMembers(*entry.discSetId) : std::vector<DiscSetMember>{};
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

  ASSERT_EQ(entries().size(), 1u) << "the entry was deleted rather than kept";
  EXPECT_FALSE(entries().front().isContentAvailable);
  EXPECT_FALSE(entries().front().hidden) << "losing a file put the entry away on the user's behalf";

  write("Metroid.nes", romBytes(40960, 5));
  scan();

  ASSERT_EQ(entries().size(), 1u);
  EXPECT_TRUE(entries().front().isContentAvailable);
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

// TODO
// The archive copy of the cataloguing pipeline never called parseFilenameTags, so a zipped release
// lost the region and disc number its name carries while the identical loose file kept them
TEST_F(LibraryEndToEndTest, AZippedCartridgeCarriesTheRegionFromItsName) {
  writeArchive("games.zip", "Sonic (USA).bin", genesisCartridgeBytes("SEGA MEGA DRIVE ", 9));
  registerContentDirectory();

  scan();

  const auto files = m_repo->getContentFiles();
  ASSERT_EQ(files.size(), 1u) << shape();
  EXPECT_EQ(files.front().m_regions, (std::vector<std::string>{"US"}))
      << "a zipped release lost the region its name carries";
}

// TODO
// Only the loose path cleared a stale drop before identifying, so an archived entry that failed
// once kept being reported even after it started identifying
TEST_F(LibraryEndToEndTest, AnArchivedEntrysDropIsClearedOnceItIdentifies) {
  writeArchive("games.zip", "Sonic.bin", QByteArray(4096, 0x11));
  registerContentDirectory();
  scan();
  ASSERT_EQ(m_repo->getScanDrops().size(), 1u) << "unreadable bytes should have been recorded";

  writeArchive("games.zip", "Sonic.bin", genesisCartridgeBytes("SEGA MEGA DRIVE ", 9));
  scan();

  EXPECT_TRUE(m_repo->getScanDrops().empty()) << "the drop outlived the thing it was reporting";
}

// TODO
// The size cap ran before the acceptance gate, so a file nothing accepts became a row of its own
// rather than one tally, which is the opposite of what the drop table is for
TEST_F(LibraryEndToEndTest, AFileNothingAcceptsIsCountedRatherThanDropped) {
  write("save.srm", QByteArray(1024, 0x22));
  registerContentDirectory();

  scan();

  EXPECT_TRUE(m_repo->getScanDrops().empty()) << "an unaccepted file became a per-path row";

  const auto counted = m_repo->getUnrecognizedExtensions();
  ASSERT_EQ(counted.size(), 1u);
  EXPECT_EQ(counted.front().extension, "srm");
}

// TODO
// There was no patch handling inside an archive at all, so a zipped patch was tallied as an
// extension nothing accepts instead of being catalogued
TEST_F(LibraryEndToEndTest, APatchInsideAnArchiveIsCatalogued) {
  writeArchive("patches.zip", "Translation.ips", QByteArray("PATCH") + QByteArray(64, 0x33) + QByteArray("EOF"));
  registerContentDirectory();

  scan();

  for (const auto &row : m_repo->getUnrecognizedExtensions()) {
    EXPECT_NE(row.extension, "ips") << "an archived patch was tallied rather than catalogued";
  }
}

// TODO
// The archive path took its extension from the whole entry path, so a dot anywhere above the file
// made the text after it the extension
TEST_F(LibraryEndToEndTest, AnArchiveEntryUnderADottedDirectoryIsReadCorrectly) {
  writeArchive("games.zip", "v1.0/Sonic.bin", genesisCartridgeBytes("SEGA MEGA DRIVE ", 5));
  registerContentDirectory();

  scan();

  for (const auto &row : m_repo->getUnrecognizedExtensions()) {
    EXPECT_EQ(row.extension.find('/'), std::string::npos)
        << "a directory name was read as an extension: " << row.extension;
  }
  EXPECT_EQ(m_repo->getContentFiles().size(), 1u) << shape();
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

// Inside an archive, any raw track extension was skipped whether or not a sheet named it, so a
// zipped cartridge with a .bin name could never be catalogued at all
TEST_F(LibraryEndToEndTest, AZippedCartridgeNamedBinIsCatalogued) {
  const auto inner = genesisCartridgeBytes("SEGA MEGA DRIVE ", 4);
  const auto zipPath = path("games.zip").toStdString();

  {
    archive *writer = archive_write_new();
    archive_write_set_format_zip(writer);
    ASSERT_EQ(archive_write_open_filename(writer, zipPath.c_str()), ARCHIVE_OK);
    archive_entry *entry = archive_entry_new();
    archive_entry_set_pathname(entry, "Sonic.bin");
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

  ASSERT_EQ(entries().size(), 1u) << "a zipped cartridge named .bin never reached the library";
  EXPECT_EQ(entries().front().displayName, "Sonic.bin");
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

// A format we accept and nothing can read. The file is taken in, fails, and is recorded per
// path rather than dropped. One test per format, so a failure names it
class AcceptedUnreadableFormatTest : public LibraryEndToEndTest, public testing::WithParamInterface<const char *> {};

// An equality on a value that always exists, so it cannot pass by finding nothing
TEST_P(AcceptedUnreadableFormatTest, DoesNotIdentify) {
  const auto filePath = path(QString("Game.") + GetParam());
  write(QString("Game.") + GetParam(), romBytes(65536, 13));

  const ContentIdentifier identifier(m_platformService);
  const auto identified = identifier.identify(filePath.toStdString());

  EXPECT_FALSE(identified.isIdentified());
  EXPECT_NE(identified.platformId, platforms::PlatformService::PLATFORM_ID_GAMEBOY)
      << "the whole-file Game Boy fallback was taken";
}

// The scanner half, with a game beside it that must survive. Without the control an empty
// library would satisfy every assertion here
TEST_P(AcceptedUnreadableFormatTest, IsRecordedRatherThanDropped) {
  write(QString("Game.") + GetParam(), romBytes(65536, 13));
  write("Tetris.gb", romBytes(32768, 7));
  registerContentDirectory();

  scan();

  ASSERT_EQ(entries().size(), 1u) << "the readable game beside it did not survive the scan";
  EXPECT_EQ(entries().front().displayName, "Tetris.gb");

  // The whole point: the file is gone from the library but not from the record
  const auto drops = m_repo->getScanDrops();
  ASSERT_EQ(drops.size(), 1u) << "a file we accepted and could not read went unrecorded";
  EXPECT_EQ(drops.front().extension, GetParam());
  EXPECT_TRUE(drops.front().archivePath.empty());
}

// cso has no handler at all. img and mdf hold raw sector data and are asked outright, so junk
// bytes reach them and fail on the content rather than on the format
INSTANTIATE_TEST_SUITE_P(FormatsWeAcceptAndCannotRead, AcceptedUnreadableFormatTest,
                         testing::Values("cso", "img", "mdf"));

// cso is the honest gap: a real PSP library holds these, so accepting and reporting them beats
// an invisible library. Nothing has read one yet, which is a different thing from a bad dump
TEST_F(LibraryEndToEndTest, ACompressedPspImageSaysNothingCanReadIt) {
  const auto filePath = path("Game.cso");
  write("Game.cso", romBytes(65536, 13));

  const ContentIdentifier identifier(m_platformService);

  EXPECT_EQ(identifier.identify(filePath.toStdString()).outcome, IdentifyOutcome::NoIdentifier);
}

// ccd is a text descriptor we never parse and nrg carries layouts nothing reads, so neither is
// accepted at all now. They are counted, not recorded per path
class RejectedFormatTest : public LibraryEndToEndTest, public testing::WithParamInterface<const char *> {};

TEST_P(RejectedFormatTest, IsCountedRatherThanAccepted) {
  write(QString("Game.") + GetParam(), romBytes(65536, 13));
  write("Tetris.gb", romBytes(32768, 7));
  registerContentDirectory();

  scan();

  ASSERT_EQ(entries().size(), 1u) << "the readable game beside it did not survive the scan";
  EXPECT_TRUE(m_repo->getScanDrops().empty()) << "a format we do not accept became a per-path row";

  const auto extensions = m_repo->getUnrecognizedExtensions();
  ASSERT_EQ(extensions.size(), 1u) << "the format nobody accepts was not counted";
  EXPECT_EQ(extensions.front().extension, GetParam());
}

INSTANTIATE_TEST_SUITE_P(FormatsWeNoLongerAccept, RejectedFormatTest, testing::Values("ccd", "nrg"));

// A Genesis ROM named .bin was typed as a disc, which meant the loader handed the core no bytes
// and it could never launch. The bytes decide the content type now, not the extension
TEST_F(LibraryEndToEndTest, AGenesisCartridgeNamedBinIsACartridge) {
  write("Sonic.bin", genesisCartridgeBytes("SEGA MEGA DRIVE ", 4));
  registerContentDirectory();

  scan();

  ASSERT_EQ(entries().size(), 1u) << "a Genesis cartridge named .bin did not reach the library";
  EXPECT_EQ(entries().front().platformId, platforms::PlatformService::PLATFORM_ID_SEGA_GENESIS);

  const auto files = m_repo->getContentFiles();
  ASSERT_EQ(files.size(), 1u);
  EXPECT_EQ(files.front().m_type, ContentType::Cartridge) << "typed as a disc, so the loader would hand over no bytes";
}

// The hash a scan records has to be the one the loader recomputes at launch, or saves are
// written under a key nothing will look for again
TEST_F(LibraryEndToEndTest, TheCartridgeHashIsTheOneTheLoaderWillCompute) {
  write("Sonic.bin", genesisCartridgeBytes("SEGA MEGA DRIVE ", 4));
  registerContentDirectory();
  scan();

  const auto files = m_repo->getContentFiles();
  ASSERT_EQ(files.size(), 1u);

  const ContentLoader loader;
  const auto loaded = loader.load(files.front());

  EXPECT_TRUE(loaded.valid);
  EXPECT_EQ(loaded.contentHash, files.front().m_contentHash) << "the launch hash and the catalogued hash disagree";
}

// The false acceptance: rcheevos offers Mega Drive as a whole-file fallback for any .bin, so
// junk used to become a Genesis game under a hash that means nothing
TEST_F(LibraryEndToEndTest, AnUnreadableBinIsReportedRatherThanFiledAsGenesis) {
  write("mystery.bin", romBytes(65536, 23));
  write("Tetris.gb", romBytes(32768, 7));
  registerContentDirectory();

  scan();

  ASSERT_EQ(entries().size(), 1u) << "the .bin was accepted as a game";
  EXPECT_EQ(entries().front().displayName, "Tetris.gb");

  const auto drops = m_repo->getScanDrops();
  ASSERT_EQ(drops.size(), 1u) << "the unreadable .bin was neither catalogued nor recorded";
  EXPECT_EQ(drops.front().extension, "bin");
}

// A file nothing even accepts is counted rather than listed, so a folder of save files is one
// row instead of thousands
TEST_F(LibraryEndToEndTest, ExtensionsNothingAcceptsAreCountedNotListed) {
  for (auto i = 0; i < 5; ++i) {
    write(QString("save%1.srm").arg(i), romBytes(512, static_cast<char>(i)));
  }
  write("Tetris.gb", romBytes(32768, 7));
  registerContentDirectory();

  scan();

  ASSERT_EQ(entries().size(), 1u) << "the game beside them did not survive";
  EXPECT_TRUE(m_repo->getScanDrops().empty()) << "a file nothing accepts must not become a per-path row";

  const auto extensions = m_repo->getUnrecognizedExtensions();
  ASSERT_EQ(extensions.size(), 1u);
  EXPECT_EQ(extensions.front().extension, "srm");
  EXPECT_EQ(extensions.front().count, 5);
}

// The record describes what is still wrong, so deleting the file it names has to end it. The
// same sweep that prunes missing content files does this
TEST_F(LibraryEndToEndTest, ADropStopsBeingReportedOnceItsFileIsGone) {
  write("Game.cso", romBytes(65536, 21));
  write("Tetris.gb", romBytes(32768, 7));
  registerContentDirectory();
  scan();

  ASSERT_EQ(m_repo->getScanDrops().size(), 1u) << "the unreadable file was not recorded";

  remove("Game.cso");
  scan();

  EXPECT_TRUE(m_repo->getScanDrops().empty()) << "the drop outlived the file it described";
  EXPECT_EQ(entries().size(), 1u) << "pruning drops disturbed the library";
}

// Rescanning the same unreadable file must not pile up rows or keep announcing itself
TEST_F(LibraryEndToEndTest, RescanningDoesNotRepeatADrop) {
  write("Game.cso", romBytes(65536, 21));
  registerContentDirectory();

  scan();
  scan();
  scan();

  const auto drops = m_repo->getScanDrops();
  ASSERT_EQ(drops.size(), 1u) << "each rescan recorded the same file again";
  EXPECT_GE(drops.front().lastSeenAt, drops.front().firstSeenAt);
}

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
TEST_F(LibraryEndToEndTest, DiscNumbersAndRegionsAreParsedFromRealFilenamesOntoContentFiles) {
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

    EXPECT_EQ(file.m_regions, (std::vector<std::string>{"US"}))
        << "the scanner did not carry the region tag onto the content file: " << file.m_filePath;
  }
  std::ranges::sort(discNumbers);

  EXPECT_EQ(discNumbers, (std::vector<int>{1, 2})) << "the scanner did not carry the disc tag onto the content file";
}

// TODO
// The whole path, on real files: two discs and a playlist naming them. The playlist is read as a
// statement about which discs belong together rather than catalogued as a game of its own
TEST_F(LibraryEndToEndTest, APlaylistBesideItsDiscsIsReadRatherThanCatalogued) {
  write("Lunar (USA) (Disc 1).cue",
        QByteArray("FILE \"lunard1.bin\" BINARY\n  TRACK 01 MODE1/2048\n    INDEX 01 00:00:00\n"));
  write("lunard1.bin", discImageBytes(seededSegaCdImage(61)));
  write("Lunar (USA) (Disc 2).cue",
        QByteArray("FILE \"lunard2.bin\" BINARY\n  TRACK 01 MODE1/2048\n    INDEX 01 00:00:00\n"));
  write("lunard2.bin", discImageBytes(seededSegaCdImage(62)));
  write("Lunar.m3u", QByteArray("Lunar (USA) (Disc 1).cue\nLunar (USA) (Disc 2).cue\n"));
  registerContentDirectory();

  scan();

  // The playlist is not a game, and the two discs are one
  EXPECT_EQ(entries().size(), 1u) << "the playlist was catalogued as a game of its own";

  for (const auto &file : m_repo->getContentFiles()) {
    EXPECT_FALSE(file.m_filePath.ends_with(".m3u")) << "the playlist was catalogued as content";
  }

  const auto game = entries().front();
  ASSERT_TRUE(game.discSetId.has_value()) << "the discs the playlist named did not become a set";

  const auto members = m_repo->getDiscSetMembers(*game.discSetId);
  ASSERT_EQ(members.size(), 2u);
  EXPECT_EQ(members[0].m_discNumber, 1);
  EXPECT_EQ(members[1].m_discNumber, 2);

  for (const auto &member : members) {
    EXPECT_EQ(member.m_source, DiscSource::PlaylistFile) << "the playlist's word was not what placed the disc";
  }

  // The line count is what says how many discs the game came on
  EXPECT_EQ(m_repo->getDiscSet(*game.discSetId)->discCount, 2);
}

// TODO
// One folder per disc, with a playlist naming them by relative path. Reducing each line to its
// file name loses every set laid out this way, and the playlist is then catalogued as a game of
// its own that renders itself as one of the discs
// TODO
// A playlist says which discs a game has, not which of them are here. A line naming a file nobody
// has yet holds its number, so the discs after it do not slide up one
TEST_F(LibraryEndToEndTest, APlaylistLineForAMissingDiscHoldsItsNumber) {
  write("FF7 (Disc 1).cue",
        QByteArray("FILE \"FF7 (Disc 1).bin\" BINARY\n  TRACK 01 MODE1/2048\n    INDEX 01 00:00:00\n"));
  write("FF7 (Disc 1).bin", discImageBytes(seededSegaCdImage(81)));
  write("FF7 (Disc 3).cue",
        QByteArray("FILE \"FF7 (Disc 3).bin\" BINARY\n  TRACK 01 MODE1/2048\n    INDEX 01 00:00:00\n"));
  write("FF7 (Disc 3).bin", discImageBytes(seededSegaCdImage(83)));

  // Disc 2 is named but nobody has it
  write("FF7.m3u", QByteArray("FF7 (Disc 1).cue\nFF7 (Disc 2).cue\nFF7 (Disc 3).cue\n"));
  registerContentDirectory();

  scan();

  ASSERT_EQ(entries().size(), 1u) << "the discs did not become one game";

  const auto game = entries().front();
  ASSERT_TRUE(game.discSetId.has_value());

  const auto members = m_repo->getDiscSetMembers(*game.discSetId);
  ASSERT_EQ(members.size(), 3u) << "the line for the disc nobody has was dropped";

  EXPECT_EQ(members[0].m_discNumber, 1);
  EXPECT_EQ(members[1].m_discNumber, 2);
  EXPECT_EQ(members[2].m_discNumber, 3);

  // The one nobody has is holding a place rather than standing for a file
  EXPECT_TRUE(members[0].m_contentFileId.has_value());
  EXPECT_FALSE(members[1].m_contentFileId.has_value());
  EXPECT_TRUE(members[2].m_contentFileId.has_value());

  EXPECT_TRUE(members[2].m_memberPath.ends_with("FF7 (Disc 3).cue")) << "disc 3 slid up into disc 2's place";
}

TEST_F(LibraryEndToEndTest, APlaylistNamesDiscsInTheirOwnFolders) {
  write("FF7 (Disc 1)/FF7 (Disc 1).cue",
        QByteArray("FILE \"FF7 (Disc 1).bin\" BINARY\n  TRACK 01 MODE1/2048\n    INDEX 01 00:00:00\n"));
  write("FF7 (Disc 1)/FF7 (Disc 1).bin", discImageBytes(seededSegaCdImage(71)));
  write("FF7 (Disc 2)/FF7 (Disc 2).cue",
        QByteArray("FILE \"FF7 (Disc 2).bin\" BINARY\n  TRACK 01 MODE1/2048\n    INDEX 01 00:00:00\n"));
  write("FF7 (Disc 2)/FF7 (Disc 2).bin", discImageBytes(seededSegaCdImage(72)));
  write("FF7.m3u", QByteArray("FF7 (Disc 1)/FF7 (Disc 1).cue\nFF7 (Disc 2)/FF7 (Disc 2).cue\n"));
  registerContentDirectory();

  scan();

  for (const auto &file : m_repo->getContentFiles()) {
    EXPECT_FALSE(file.m_filePath.ends_with(".m3u")) << "the playlist was catalogued as a disc";
  }

  ASSERT_EQ(entries().size(), 1u) << "the discs in their own folders did not become one game";

  const auto game = entries().front();
  ASSERT_TRUE(game.discSetId.has_value());

  const auto members = m_repo->getDiscSetMembers(*game.discSetId);
  ASSERT_EQ(members.size(), 2u) << "a playlist line naming a subdirectory was not resolved";

  for (const auto &member : members) {
    EXPECT_EQ(member.m_source, DiscSource::PlaylistFile);
    EXPECT_FALSE(member.m_memberPath.ends_with(".m3u"));
  }

  EXPECT_EQ(m_repo->getDiscSet(*game.discSetId)->discCount, 2);
}

// TODO
// A sheet sitting above its tracks. Matching by file name alone cannot reach into a subdirectory,
// so the track was catalogued as a game of its own however clearly the sheet spoke for it
TEST_F(LibraryEndToEndTest, ASheetSpeaksForATrackInASubdirectory) {
  write("Sonic CD.cue",
        QByteArray("FILE \"tracks/Sonic CD (Track 1).bin\" BINARY\n  TRACK 01 MODE1/2048\n    INDEX 01 00:00:00\n"));
  write("tracks/Sonic CD (Track 1).bin", discImageBytes(seededSegaCdImage(81)));
  registerContentDirectory();

  scan();

  ASSERT_EQ(entries().size(), 1u) << "the track under the sheet was catalogued as its own game";

  const auto files = m_repo->getContentFiles();
  ASSERT_EQ(files.size(), 1u);
  EXPECT_TRUE(files.front().m_filePath.ends_with(".cue"));
}

// TODO
// Two numbered discs with nothing else to go on collapse into one game. This is the filename rung
// of the placement ladder, which is what fires for the overwhelming majority of real libraries
TEST_F(LibraryEndToEndTest, TwoDiscsWithNoPlaylistBecomeOneGame) {
  writeDisc("Lunar (Disc 1)", 41);
  writeDisc("Lunar (Disc 2)", 42);
  registerContentDirectory();

  scan();

  ASSERT_EQ(entries().size(), 1u) << shape();

  const auto game = entries().front();
  ASSERT_TRUE(game.discSetId.has_value()) << "two discs of one game did not form a set";

  const auto members = membersOf(game);
  ASSERT_EQ(members.size(), 2u);
  EXPECT_EQ(members[0].m_discNumber, 1);
  EXPECT_EQ(members[1].m_discNumber, 2);

  for (const auto &member : members) {
    EXPECT_EQ(member.m_source, DiscSource::Filename);
  }

  // One game is one way in, on the disc the set is identified by
  EXPECT_EQ(runConfigurationCount(), 1u) << shape();
}

// TODO
// Disc 1 turning up after the others moves what the game is keyed on, in place. The row is the
// same row, so everything a person put on it is still there
TEST_F(LibraryEndToEndTest, DiscOneArrivingLastMovesTheGameOntoItWithoutLosingIt) {
  writeDisc("Xenogears (Disc 2)", 52);
  writeDisc("Xenogears (Disc 3)", 53);
  registerContentDirectory();

  scan();

  ASSERT_EQ(entries().size(), 1u) << shape();
  const auto before = entries().front();

  // Something only the person could have done, which has to survive the identity moving
  auto edited = before;
  edited.rating = 5;
  edited.favorite = true;
  ASSERT_TRUE(m_repo->update(edited));

  writeDisc("Xenogears (Disc 1)", 51);
  scan();

  ASSERT_EQ(entries().size(), 1u) << "a second game appeared instead of the identity moving: " + shape();

  const auto after = entries().front();
  EXPECT_EQ(after.id, before.id) << "the row was replaced rather than re-keyed";
  EXPECT_NE(after.contentHash, before.contentHash) << "the identity did not move onto disc 1";
  EXPECT_EQ(after.rating, 5) << "the rating did not survive the re-key";
  EXPECT_TRUE(after.favorite) << "the favourite did not survive the re-key";

  ASSERT_EQ(membersOf(after).size(), 3u);
  EXPECT_EQ(membersOf(after).front().m_discNumber, 1);
}

// TODO
// A playlist is a statement about which discs belong together and in what order, so its order is
// what the set takes even when the filenames say something else
TEST_F(LibraryEndToEndTest, APlaylistsOrderOutranksTheNumbersInTheFilenames) {
  writeDisc("Riven (Disc 1)", 61);
  writeDisc("Riven (Disc 2)", 62);
  write("Riven.m3u", QByteArray("Riven (Disc 2).cue\nRiven (Disc 1).cue\n"));
  registerContentDirectory();

  scan();

  ASSERT_EQ(entries().size(), 1u) << shape();

  const auto members = membersOf(entries().front());
  ASSERT_EQ(members.size(), 2u);

  EXPECT_TRUE(members[0].m_memberPath.ends_with("Riven (Disc 2).cue")) << "the playlist's order was ignored";
  EXPECT_TRUE(members[1].m_memberPath.ends_with("Riven (Disc 1).cue"));

  for (const auto &member : members) {
    EXPECT_EQ(member.m_source, DiscSource::PlaylistFile);
  }
}

// TODO
// A claim holds a place for a disc nobody has yet. The disc turning up fills that place rather
// than starting a set of its own
TEST_F(LibraryEndToEndTest, TheDiscAClaimWasHoldingAPlaceForArrivesLater) {
  writeDisc("Koudelka (Disc 1)", 71);
  write("Koudelka.m3u", QByteArray("Koudelka (Disc 1).cue\nKoudelka (Disc 2).cue\n"));
  registerContentDirectory();

  scan();

  ASSERT_EQ(entries().size(), 1u) << shape();
  auto members = membersOf(entries().front());
  ASSERT_EQ(members.size(), 2u);
  EXPECT_FALSE(members[1].m_contentFileId.has_value()) << "a disc nobody has read as a real file";

  writeDisc("Koudelka (Disc 2)", 72);
  scan();

  ASSERT_EQ(entries().size(), 1u) << "the arriving disc started a game of its own: " + shape();

  members = membersOf(entries().front());
  ASSERT_EQ(members.size(), 2u) << "the arriving disc was added beside the place held for it";
  EXPECT_TRUE(members[1].m_contentFileId.has_value()) << "the place held for disc 2 was never filled";
  EXPECT_EQ(members[1].m_discNumber, 2);
}

// TODO
// Membership is not presence. A disc going away leaves the game, the set and the membership row
// exactly where they were, so putting the file back needs no repair
TEST_F(LibraryEndToEndTest, LosingOneDiscKeepsTheSetAndItsMembership) {
  writeDisc("Grandia (Disc 1)", 81);
  writeDisc("Grandia (Disc 2)", 82);
  registerContentDirectory();

  scan();

  ASSERT_EQ(entries().size(), 1u) << shape();
  const auto before = entries().front();
  const auto setId = *before.discSetId;

  remove("Grandia (Disc 2).cue");
  remove("Grandia (Disc 2).bin");
  scan();

  ASSERT_EQ(entries().size(), 1u) << "losing a disc took the game with it: " + shape();

  const auto after = entries().front();
  EXPECT_EQ(after.id, before.id);
  EXPECT_EQ(after.contentHash, before.contentHash) << "losing disc 2 moved what the game is keyed on";
  ASSERT_TRUE(after.discSetId.has_value());
  EXPECT_EQ(*after.discSetId, setId) << "the set was dissolved";

  EXPECT_EQ(m_repo->getDiscSetMembers(setId).size(), 2u) << "membership followed the bytes";
  EXPECT_EQ(m_repo->getPresentDiscsInSet(setId).size(), 1u) << "a missing disc still counts as present";
}

// TODO
// Two regional releases share a title and differ in region, which is the whole of what keeps them
// apart. Folding them would put one release's saves on the other
TEST_F(LibraryEndToEndTest, TwoRegionalReleasesInOneFolderStayTwoGames) {
  writeDisc("Suikoden II (USA) (Disc 1)", 91);
  writeDisc("Suikoden II (Japan) (Disc 1)", 92);
  registerContentDirectory();

  scan();

  ASSERT_EQ(entries().size(), 2u) << "two regional releases were folded into one game: " + shape();

  for (const auto &entry : entries()) {
    ASSERT_TRUE(entry.discSetId.has_value());
    EXPECT_EQ(membersOf(entry).size(), 1u) << "a release took the other's disc into its set";
  }
}

// TODO
// A playlist naming raw disc images rather than sheets still forms a set. Reading it as a sheet
// that speaks for its lines would leave the game with nothing to launch through
TEST_F(LibraryEndToEndTest, APlaylistNamingRawDiscsStillYieldsAGame) {
  write("Panzer (Disc 1).iso", discImageBytes(seededSegaCdImage(101)));
  write("Panzer (Disc 2).iso", discImageBytes(seededSegaCdImage(102)));
  write("Panzer.m3u", QByteArray("Panzer (Disc 1).iso\nPanzer (Disc 2).iso\n"));
  registerContentDirectory();

  scan();

  ASSERT_EQ(entries().size(), 1u) << "a playlist of raw discs left no game behind: " + shape();

  const auto game = entries().front();
  ASSERT_TRUE(game.discSetId.has_value());
  EXPECT_EQ(membersOf(game).size(), 2u);
  EXPECT_TRUE(game.hasRunConfiguration) << "the game has nothing to launch through";
}

// TODO
// A playlist whose first line names a disc nobody has still groups the ones they do have. The
// absent disc keeps the number the playlist gave it
TEST_F(LibraryEndToEndTest, APlaylistWhoseFirstDiscIsAbsentStillGroupsTheRest) {
  writeDisc("Lunar 2 (Disc 2)", 112);
  writeDisc("Lunar 2 (Disc 3)", 113);
  write("Lunar 2.m3u", QByteArray("Lunar 2 (Disc 1).cue\nLunar 2 (Disc 2).cue\nLunar 2 (Disc 3).cue\n"));
  registerContentDirectory();

  scan();

  ASSERT_EQ(entries().size(), 1u) << shape();

  const auto members = membersOf(entries().front());
  ASSERT_EQ(members.size(), 3u) << "the discs that are here did not group";

  EXPECT_FALSE(members[0].m_contentFileId.has_value()) << "a disc nobody has read as a real file";
  EXPECT_TRUE(members[1].m_contentFileId.has_value());
  EXPECT_TRUE(members[2].m_contentFileId.has_value());

  // The set launches from the lowest disc it actually has
  EXPECT_TRUE(entries().front().hasRunConfiguration);
}

// TODO
// A playlist carries its first disc's content hash. Catalogued as content it would hand that
// disc's game a second way in and name itself among the game's own files
TEST_F(LibraryEndToEndTest, APlaylistIsNeitherAWayInNorAContentPath) {
  writeDisc("Riven (Disc 1)", 121);
  writeDisc("Riven (Disc 2)", 122);
  write("Riven.m3u", QByteArray("Riven (Disc 1).cue\nRiven (Disc 2).cue\n"));
  registerContentDirectory();

  scan();

  ASSERT_EQ(entries().size(), 1u) << shape();
  const auto game = entries().front();

  EXPECT_EQ(m_repo->getRunConfigurations(game.contentHash).size(), 1u)
      << "the playlist handed the game a second way in";

  for (const auto &path : game.contentPaths) {
    EXPECT_FALSE(path.ends_with(".m3u")) << "the playlist read as somewhere the game's content lives";
  }

  for (const auto &file : m_repo->getContentFilesWithContentHash(game.contentHash)) {
    EXPECT_FALSE(file.m_filePath.ends_with(".m3u")) << "the playlist read as a copy of disc 1";
  }
}

// TODO
// A set launches through a playlist of ours under the app data directory, never through anything
// written beside the discs
TEST_F(LibraryEndToEndTest, ADiscSetLaunchesThroughAPlaylistWeWrote) {
  writeDisc("Riven (Disc 1)", 131);
  writeDisc("Riven (Disc 2)", 132);
  registerContentDirectory();

  scan();

  ASSERT_EQ(entries().size(), 1u) << shape();
  const auto game = entries().front();

  const auto rendered = playlistFor(game.contentHash);
  ASSERT_TRUE(QFile::exists(QString::fromStdString(rendered))) << "the set has no playlist to launch through";

  const auto lines = discLinesOf(rendered);
  ASSERT_EQ(lines.size(), 2u);
  EXPECT_TRUE(lines[0].ends_with("Riven (Disc 1).cue")) << "the playlist does not start with the disc it is named for";
  EXPECT_TRUE(lines[1].ends_with("Riven (Disc 2).cue"));

  // Nothing of ours is written in among somebody's ROMs
  for (const auto &name : QDir(m_tempDir.path()).entryList(QDir::Files)) {
    EXPECT_FALSE(name.endsWith(".m3u") && name.contains(QString::fromStdString(game.contentHash)))
        << "a generated playlist was written into the ROM folder";
  }
}

// TODO
// A game whose discs have all gone stays in the library as something that cannot be played, rather
// than disappearing and taking everything a person put on it with it
TEST_F(LibraryEndToEndTest, ASetWhoseDiscsAreAllGoneStaysInTheLibrary) {
  writeDisc("Grandia (Disc 1)", 141);
  writeDisc("Grandia (Disc 2)", 142);
  registerContentDirectory();

  scan();

  ASSERT_EQ(entries().size(), 1u) << shape();
  const auto before = entries().front();

  remove("Grandia (Disc 1).cue");
  remove("Grandia (Disc 1).bin");
  remove("Grandia (Disc 2).cue");
  remove("Grandia (Disc 2).bin");
  scan();

  ASSERT_EQ(entries().size(), 1u) << "the game left the library when its files did";

  const auto after = entries().front();
  EXPECT_EQ(after.id, before.id);
  EXPECT_FALSE(after.isContentAvailable) << "a game with no files on disk read as playable";
  EXPECT_FALSE(after.contentPaths.empty()) << "the answer to where the game went was thrown away";
}

// TODO
// One folder per disc is a layout people use, and the discs in it have to find each other with
// nothing but their names to go on
TEST_F(LibraryEndToEndTest, DiscsInTheirOwnFoldersFindEachOtherWithoutAPlaylist) {
  writeDisc("FF9 (Disc 1)/FF9 (Disc 1)", 151);
  writeDisc("FF9 (Disc 2)/FF9 (Disc 2)", 152);
  registerContentDirectory();

  scan();

  ASSERT_EQ(entries().size(), 1u) << "discs in their own folders did not become one game: " + shape();
  EXPECT_EQ(membersOf(entries().front()).size(), 2u);
}

} // namespace firelight::library

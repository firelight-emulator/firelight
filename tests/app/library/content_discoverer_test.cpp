// TODO: NEEDS REVIEW
#include <firelight/library/content_discoverer.hpp>
#include <firelight/library/content_extensions.hpp>
#include <firelight/library/content_identifier.hpp>
#include <firelight/platforms/platform_service.hpp>

#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <archive.h>
#include <archive_entry.h>
#include <gtest/gtest.h>

// What the walk decides, one rule at a time. No repository and no scan worker: the discoverer
// holds neither, which is the whole point of it being its own type
namespace firelight::library {
namespace {

QByteArray bytesOf(const int size, const char seed) {
  QByteArray bytes(size, seed);
  for (auto i = 0; i < size; ++i) {
    bytes[i] = static_cast<char>((i * 31 + seed) & 0xFF);
  }
  return bytes;
}

} // namespace

class ContentDiscovererTest : public testing::Test {
protected:
  QTemporaryDir m_tempDir;
  platforms::PlatformService m_platformService;

  void SetUp() override { ASSERT_TRUE(m_tempDir.isValid()); }

  [[nodiscard]] QString path(const QString &relative) const { return m_tempDir.filePath(relative); }

  void write(const QString &relative, const QByteArray &bytes) const {
    const QFileInfo info(path(relative));
    ASSERT_TRUE(QDir().mkpath(info.absolutePath()));

    QFile file(path(relative));
    ASSERT_TRUE(file.open(QIODevice::WriteOnly));
    ASSERT_EQ(file.write(bytes), bytes.size());
  }

  void writeArchive(const QString &relative, const std::vector<std::pair<std::string, QByteArray>> &entries) const {
    const auto archivePath = path(relative).toStdString();
    archive *writer = archive_write_new();
    archive_write_set_format_zip(writer);
    ASSERT_EQ(archive_write_open_filename(writer, archivePath.c_str()), ARCHIVE_OK);

    for (const auto &[name, bytes] : entries) {
      archive_entry *entry = archive_entry_new();
      archive_entry_set_pathname(entry, name.c_str());
      archive_entry_set_size(entry, bytes.size());
      archive_entry_set_filetype(entry, AE_IFREG);
      archive_entry_set_perm(entry, 0644);
      archive_write_header(writer, entry);
      archive_write_data(writer, bytes.constData(), bytes.size());
      archive_entry_free(entry);
    }

    archive_write_close(writer);
    archive_write_free(writer);
  }

  // Everything one walk of the temp directory found
  struct Walked {
    std::vector<DiscoveredFile> files;
    ContentDiscoverer::Skipped skipped;
  };

  [[nodiscard]] Walked walk(const int64_t maxBytes = ContentDiscoverer::MAX_UNRECOGNIZED_FILE_BYTES,
                            const bool includeFiles = true) const {
    Walked walked;
    const ContentDiscoverer discoverer(m_platformService, maxBytes);
    walked.skipped = discoverer.walk(
        m_tempDir.path().toStdString(), [&](const DiscoveredFile &file) { walked.files.push_back(file); },
        [] { return true; }, includeFiles);
    return walked;
  }

  [[nodiscard]] static const DiscoveredFile *named(const Walked &walked, const std::string &suffix) {
    for (const auto &file : walked.files) {
      if (file.path.size() >= suffix.size() &&
          file.path.compare(file.path.size() - suffix.size(), suffix.size(), suffix) == 0) {
        return &file;
      }
    }
    return nullptr;
  }
};

// TODO
// The rule that made this type worth extracting: where the bytes live must change nothing about
// what is discovered
TEST_F(ContentDiscovererTest, ALooseFileAndAnArchivedOneAreDiscoveredIdentically) {
  const auto bytes = bytesOf(32768, 3);
  write("Sonic (USA) (Disc 1).gb", bytes);
  writeArchive("games.zip", {{"Sonic (USA) (Disc 1).gb", bytes}});

  const auto walked = walk();
  ASSERT_EQ(walked.files.size(), 2u);

  const auto *loose = walked.files[0].isInArchive() ? &walked.files[1] : &walked.files[0];
  const auto *archived = walked.files[0].isInArchive() ? &walked.files[0] : &walked.files[1];

  EXPECT_EQ(loose->extension, archived->extension);
  EXPECT_EQ(loose->sizeBytes, archived->sizeBytes);
  EXPECT_EQ(loose->isDisc, archived->isDisc);
  EXPECT_EQ(loose->tags.regions, archived->tags.regions) << "one of them lost the region its name carries";
  EXPECT_EQ(loose->tags.discNumber, archived->tags.discNumber) << "one of them lost its disc number";
}

// TODO
// The extension is a fact about the file's own name; a dot in a directory above it is not
TEST_F(ContentDiscovererTest, AnExtensionComesFromTheFilesOwnName) {
  writeArchive("games.zip", {{"v1.0/Sonic.gb", bytesOf(32768, 4)}});

  const auto walked = walk();

  ASSERT_EQ(walked.files.size(), 1u);
  EXPECT_EQ(walked.files.front().extension, "gb");
  EXPECT_TRUE(walked.skipped.unrecognizedExtensions.empty()) << "a directory name was counted as an extension";
}

// TODO
// A raw track is reached through the sheet naming it, and that has to be true wherever the sheet is
TEST_F(ContentDiscovererTest, ASheetSpeaksForItsTracksOnDiskAndInAnArchive) {
  const QByteArray sheet("FILE \"Game (Track 1).bin\" BINARY\r\n  TRACK 01 MODE1/2352\r\n");
  write("Game.cue", sheet);
  write("Game (Track 1).bin", bytesOf(4096, 5));

  const auto loose = walk();

  const auto *track = named(loose, "Game (Track 1).bin");
  ASSERT_NE(track, nullptr) << "a track its sheet names was left out rather than told apart";
  EXPECT_EQ(track->role, ContentRole::Track) << "a track its sheet names was offered as a game";

  const auto *sheetFile = named(loose, "Game.cue");
  ASSERT_NE(sheetFile, nullptr) << "the sheet itself should still be discovered";
  EXPECT_EQ(sheetFile->role, ContentRole::Dump);
}

TEST_F(ContentDiscovererTest, ASheetInsideAnArchiveSpeaksForItsTracksToo) {
  const QByteArray sheet("FILE \"Game (Track 1).bin\" BINARY\r\n  TRACK 01 MODE1/2352\r\n");
  writeArchive("game.zip", {{"Game.cue", sheet}, {"Game (Track 1).bin", bytesOf(4096, 5)}});

  const auto walked = walk();

  const auto *track = named(walked, "Game (Track 1).bin");
  ASSERT_NE(track, nullptr) << "an archived track its sheet names was left out rather than told apart";
  EXPECT_EQ(track->role, ContentRole::Track) << "an archived track its sheet names was offered as a game";
}

// TODO
// A .ccd is not a format anything here reads, but a track it names is still spoken for
TEST_F(ContentDiscovererTest, ACcdSpeaksForItsTracksWithoutBeingContentItself) {
  write("Game.ccd", QByteArray("[CloneCD]\r\nVersion=3\r\n"));
  write("Game.img", bytesOf(4096, 6));

  const auto walked = walk();

  EXPECT_EQ(named(walked, "Game.ccd"), nullptr) << "a format nothing reads was offered as content";
}

// TODO
// A cartridge whose extension is also a raw-track extension is a file in its own right when no
// sheet names it. Skipping those is how a zipped cartridge went missing
TEST_F(ContentDiscovererTest, AnUnreferencedTrackExtensionIsStillDiscovered) {
  write("Sonic.bin", bytesOf(4096, 7));

  const auto walked = walk();

  const auto *cartridge = named(walked, "Sonic.bin");
  ASSERT_NE(cartridge, nullptr) << "a .bin nothing speaks for was skipped";
  EXPECT_EQ(cartridge->role, ContentRole::Dump) << "a .bin nothing speaks for read as somebody's track";
}

// TODO
// A playlist is a statement about which discs belong together, so it is told apart from the discs
// it names rather than being offered as one of them
TEST_F(ContentDiscovererTest, APlaylistIsToldApartFromContent) {
  write("Game.m3u", QByteArray("Game (Disc 1).cue\r\n"));

  const auto walked = walk();

  const auto *playlist = named(walked, "Game.m3u");
  ASSERT_NE(playlist, nullptr);
  EXPECT_EQ(playlist->role, ContentRole::Playlist);
}

// TODO
// The cap exists so an enormous file is not read; the injectable limit is what lets that be tested
// without writing a gigabyte
TEST_F(ContentDiscovererTest, AFileOverTheCapIsSetAsideRatherThanDiscovered) {
  write("Huge.gb", bytesOf(4096, 8));

  const auto walked = walk(1024);

  EXPECT_TRUE(walked.files.empty()) << "an oversized file was offered for identifying";
  ASSERT_EQ(walked.skipped.tooLarge.size(), 1u);
  EXPECT_EQ(walked.skipped.tooLarge.front().extension, "gb");
}

// TODO
// The cap sits after the acceptance gate, so a file nothing accepts is one tally however large it
// is rather than a row of its own
TEST_F(ContentDiscovererTest, AnUnacceptedFileIsCountedEvenWhenItIsOverTheCap) {
  write("save.srm", bytesOf(4096, 9));

  const auto walked = walk(1024);

  EXPECT_TRUE(walked.skipped.tooLarge.empty()) << "an unaccepted file became an oversize row";
  ASSERT_EQ(walked.skipped.unrecognizedExtensions.size(), 1u);
  EXPECT_EQ(walked.skipped.unrecognizedExtensions.begin()->first, "srm");
  EXPECT_EQ(walked.skipped.unrecognizedExtensions.begin()->second, 1);
}

// TODO
// A disc image is legitimately this large, so the cap must not reach it
TEST_F(ContentDiscovererTest, ADiscImageIsExemptFromTheCap) {
  write("Game.iso", bytesOf(4096, 10));

  const auto walked = walk(1024);

  EXPECT_TRUE(walked.skipped.tooLarge.empty()) << "a disc image was set aside for its size";
  EXPECT_NE(named(walked, "Game.iso"), nullptr);
}

// TODO
// A patch is catalogued by its bytes rather than identified, so it leaves the pipeline early --
// and that is true inside an archive as well, which it never used to be
TEST_F(ContentDiscovererTest, PatchesAreSetAsideWhereverTheyAre) {
  write("Translation.ips", QByteArray("PATCH...EOF"));
  writeArchive("patches.zip", {{"Other.ips", QByteArray("PATCH...EOF")}});

  const auto walked = walk();

  ASSERT_EQ(walked.skipped.patches.size(), 2u) << "an archived patch was not set aside";
  EXPECT_TRUE(walked.files.empty());
  EXPECT_TRUE(walked.skipped.unrecognizedExtensions.empty()) << "a patch was tallied as unrecognized";
}

// TODO
// Anything set aside is read after the walk, by which time an archive's own reader is long gone
TEST_F(ContentDiscovererTest, AnArchivedPatchCanStillBeReadAfterTheWalk) {
  writeArchive("patches.zip", {{"Translation.ips", QByteArray("PATCH-CONTENT")}});

  const auto walked = walk();

  ASSERT_EQ(walked.skipped.patches.size(), 1u);
  const auto bytes = walked.skipped.patches.front().readBytes();
  EXPECT_EQ(QByteArray(reinterpret_cast<const char *>(bytes.data()), static_cast<qsizetype>(bytes.size())),
            QByteArray("PATCH-CONTENT"));
}

// TODO
// A file with no extension says nothing about which formats people own, so it is not tallied
TEST_F(ContentDiscovererTest, AFileWithNoExtensionIsNeitherDiscoveredNorCounted) {
  write("README", QByteArray("nothing here"));

  const auto walked = walk();

  EXPECT_TRUE(walked.files.empty());
  EXPECT_TRUE(walked.skipped.unrecognizedExtensions.empty());
}

// TODO
// Every file carrying an extension nothing accepts adds to that extension's tally rather than
// becoming a row of its own
TEST_F(ContentDiscovererTest, UnacceptedExtensionsAreTalliedPerExtension) {
  write("one.srm", QByteArray("a"));
  write("two.srm", QByteArray("b"));
  write("notes.txt", QByteArray("c"));

  const auto walked = walk();

  ASSERT_EQ(walked.skipped.unrecognizedExtensions.size(), 2u);
  EXPECT_EQ(walked.skipped.unrecognizedExtensions.at("srm"), 2);
  EXPECT_EQ(walked.skipped.unrecognizedExtensions.at("txt"), 1);
}

// TODO
// Subdirectories are handed back rather than descended into, because the scanner queues them so a
// deep tree cannot recurse the stack away
TEST_F(ContentDiscovererTest, SubdirectoriesAreHandedBackRatherThanEntered) {
  write("games/Sonic.gb", bytesOf(32768, 11));

  const auto walked = walk();

  EXPECT_TRUE(walked.files.empty()) << "the walk descended instead of handing the directory back";
  ASSERT_EQ(walked.skipped.subdirectories.size(), 1u);
  EXPECT_TRUE(walked.skipped.subdirectories.front().ends_with("games"));
}

// TODO
// A directory whose timestamp has not moved still has to be descended into, because a subdirectory
// can change without its parent's timestamp moving
TEST_F(ContentDiscovererTest, SkippingFilesStillCollectsSubdirectories) {
  write("Sonic.gb", bytesOf(32768, 12));
  write("games/Zelda.gb", bytesOf(32768, 13));

  const auto walked = walk(ContentDiscoverer::MAX_UNRECOGNIZED_FILE_BYTES, false);

  EXPECT_TRUE(walked.files.empty()) << "files were examined when they were meant to be skipped";
  EXPECT_EQ(walked.skipped.subdirectories.size(), 1u) << "the descent was skipped along with the files";
}

// TODO
// A walk that was stopped saw only part of the directory, and the caller has to be able to tell
TEST_F(ContentDiscovererTest, AStoppedWalkSaysItDidNotFinish) {
  write("one.gb", bytesOf(32768, 14));
  write("two.gb", bytesOf(32768, 15));

  const ContentDiscoverer discoverer(m_platformService);
  auto seen = 0;
  const auto skipped = discoverer.walk(
      m_tempDir.path().toStdString(), [&](const DiscoveredFile &) { ++seen; }, [&] { return seen < 1; });

  EXPECT_FALSE(skipped.completed);
  EXPECT_EQ(seen, 1);
}

// TODO
// The mapping every catalogue row goes through. One place to assert every field is what stops one
// being set for a loose file and forgotten for an archived one
TEST(ToContentFileTest, CarriesEveryFieldFromWhatWasDiscoveredAndIdentified) {
  DiscoveredFile file;
  file.path = "Sonic (USA) (Disc 2).bin";
  file.archivePath = "C:/roms/games.zip";
  file.extension = "bin";
  file.sizeBytes = 4096;
  file.isDisc = true;
  file.tags.regions = {"US"};
  file.tags.discNumber = 2;

  IdentifiedContent identified;
  identified.outcome = IdentifyOutcome::Identified;
  identified.isDisc = true;
  identified.platformId = 6;
  identified.contentHash = "abc123";
  identified.fileMd5 = "def456";
  identified.fileSizeBytes = 4096;

  const auto row = toContentFile(file, identified);

  EXPECT_EQ(row.m_type, ContentType::Disc);
  EXPECT_EQ(row.m_filePath, "Sonic (USA) (Disc 2).bin");
  EXPECT_EQ(row.m_archivePathName, "C:/roms/games.zip");
  EXPECT_TRUE(row.m_inArchive);
  EXPECT_EQ(row.m_platformId, 6);
  EXPECT_EQ(row.m_contentHash, "abc123");
  EXPECT_EQ(row.m_fileMd5, "def456");
  EXPECT_EQ(row.m_fileSizeBytes, 4096u);
  EXPECT_EQ(row.m_discNumber, 2) << "the disc number the file name carries was dropped";
  EXPECT_EQ(row.m_regions, (std::vector<std::string>{"US"})) << "the region the file name carries was dropped";
}

// TODO
// A loose file is not in an archive, and the row has to say so or the launcher looks for it in one
TEST(ToContentFileTest, ALooseFileIsNotMarkedAsArchived) {
  DiscoveredFile file;
  file.path = "C:/roms/Tetris.gb";
  file.extension = "gb";

  IdentifiedContent identified;
  identified.outcome = IdentifyOutcome::Identified;
  identified.isDisc = false;

  const auto row = toContentFile(file, identified);

  EXPECT_EQ(row.m_type, ContentType::Cartridge);
  EXPECT_FALSE(row.m_inArchive);
  EXPECT_TRUE(row.m_archivePathName.empty());
}

// TODO
// A playlist names discs, not tracks. Speaking for them would leave every disc it lists out of the
// library and the game with nothing to launch through
TEST_F(ContentDiscovererTest, APlaylistDoesNotSpeakForTheDiscsItNames) {
  write("Game.m3u", QByteArray("Game (Disc 1).bin\nGame (Disc 2).bin\n"));
  write("Game (Disc 1).bin", bytesOf(4096, 11));
  write("Game (Disc 2).bin", bytesOf(4096, 12));

  const auto walked = walk();

  for (const auto *name : {"Game (Disc 1).bin", "Game (Disc 2).bin"}) {
    const auto *disc = named(walked, name);
    ASSERT_NE(disc, nullptr) << name << " was left out because a playlist named it";
    EXPECT_EQ(disc->role, ContentRole::Dump) << name << " was offered as somebody's track";
  }

  EXPECT_FALSE(namesTracks("m3u")) << "a playlist counts as a sheet that speaks for tracks";
  EXPECT_TRUE(namesTracks("cue"));
  EXPECT_TRUE(namesTracks("gdi"));
  EXPECT_TRUE(namesTracks("ccd"));
}

} // namespace firelight::library

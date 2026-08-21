#include <firelight/library/content_identifier.hpp>
#include <firelight/library/disc_inspector.hpp>
#include <firelight/platforms/platform_service.hpp>

#include <QByteArray>
#include <QDir>
#include <QTemporaryDir>
#include <archive.h>
#include <archive_entry.h>
#include <gtest/gtest.h>

namespace firelight::library {
class ContentIdentifierTest : public testing::Test {
protected:
  QTemporaryDir tempDir;
  platforms::PlatformService platformService;
  ContentIdentifier identifier{platformService};

  // Writes a synthetic single-track .iso whose first sector begins with the
  // given 16-byte magic, padded out so the disc readers have data to work with
  QString writeMagicIso(const QString &name, const QByteArray &magic) {
    QByteArray data = magic;
    data.append(64 * 1024 - magic.size(), '\0');

    const QString path = tempDir.filePath(name);
    QFile file(path);
    EXPECT_TRUE(file.open(QIODevice::WriteOnly));
    file.write(data);
    file.close();
    return path;
  }

  // Writes a .zip containing a single entry with the given magic-prefixed data
  QString writeMagicIsoZip(const QString &zipName, const QString &entryName, const QByteArray &magic) {
    QByteArray data = magic;
    data.append(64 * 1024 - magic.size(), '\0');

    const QString zipPath = tempDir.filePath(zipName);

    archive *a = archive_write_new();
    archive_write_set_format_zip(a);
    EXPECT_EQ(archive_write_open_filename(a, zipPath.toStdString().c_str()), ARCHIVE_OK);

    archive_entry *entry = archive_entry_new();
    archive_entry_set_pathname(entry, entryName.toStdString().c_str());
    archive_entry_set_size(entry, data.size());
    archive_entry_set_filetype(entry, AE_IFREG);
    archive_entry_set_perm(entry, 0644);
    archive_write_header(a, entry);
    archive_write_data(a, data.constData(), data.size());
    archive_entry_free(entry);

    archive_write_close(a);
    archive_write_free(a);
    return zipPath;
  }
};

// A disc image's platform must be determined from contents, not the (ambiguous)
// .iso extension. The Sega CD magic at sector 0 should classify as Sega CD
TEST_F(ContentIdentifierTest, DetectsSegaCdFromMagic) {
  ASSERT_TRUE(tempDir.isValid());
  const QString path = writeMagicIso("segacd.iso", "SEGADISCSYSTEM  ");

  const auto result = identifier.identify(path.toStdString());

  ASSERT_TRUE(result.isIdentified());
  ASSERT_EQ(result.platformId, firelight::platforms::PlatformService::PLATFORM_ID_SEGA_CD);
  ASSERT_FALSE(result.contentHash.empty());
}

// rcheevos folds Saturn into the Sega CD hashing umbrella; identification must
// disambiguate via the sector-0 magic so Saturn isn't mislabeled as Sega CD
TEST_F(ContentIdentifierTest, DetectsSaturnFromMagic) {
  ASSERT_TRUE(tempDir.isValid());
  const QString path = writeMagicIso("saturn.iso", "SEGA SEGASATURN ");

  const auto result = identifier.identify(path.toStdString());

  ASSERT_TRUE(result.isIdentified());
  ASSERT_EQ(result.platformId, firelight::platforms::PlatformService::PLATFORM_ID_SEGA_SATURN);
  ASSERT_FALSE(result.contentHash.empty());
}

// A disc image stored inside an archive must be extracted to temp and detected
// the same way (exercises the two-pass, extract-only-what's-needed path)
TEST_F(ContentIdentifierTest, DetectsDiscInsideArchive) {
  ASSERT_TRUE(tempDir.isValid());
  const QString zipPath = writeMagicIsoZip("saturn.zip", "saturn.iso", "SEGA SEGASATURN ");

  // Mirrors how LibraryScanner2 identifies in-archive disc entries: no buffer,
  // detection re-opens the archive itself
  const auto result = identifier.identifyInArchive("saturn.iso", {}, 0, zipPath.toStdString());

  ASSERT_TRUE(result.isIdentified());
  ASSERT_EQ(result.platformId, firelight::platforms::PlatformService::PLATFORM_ID_SEGA_SATURN);
  ASSERT_FALSE(result.contentHash.empty());
}

// A loose multi-file disc set (a .cue sheet + its .bin track) must be detected
// via the sheet, and the track recorded as a disc member
TEST_F(ContentIdentifierTest, ReportsDiscMembersForCueBinSet) {
  ASSERT_TRUE(tempDir.isValid());

  // A Saturn data track, and a cue sheet that references it
  writeMagicIso("game.bin", "SEGA SEGASATURN ");
  const QString cuePath = tempDir.filePath("game.cue");
  QFile cue(cuePath);
  ASSERT_TRUE(cue.open(QIODevice::WriteOnly));
  cue.write("FILE \"game.bin\" BINARY\n  TRACK 01 MODE1/2048\n"
            "    INDEX 01 00:00:00\n");
  cue.close();

  const auto result = identifier.identify(cuePath.toStdString());

  ASSERT_TRUE(result.isIdentified());
  ASSERT_EQ(result.platformId, firelight::platforms::PlatformService::PLATFORM_ID_SEGA_SATURN);
  ASSERT_EQ(result.discMembers.size(), 1u);
  ASSERT_TRUE(result.discMembers[0].path.ends_with("game.bin"));
  ASSERT_EQ(result.discMembers[0].role, "track");
}

// A disc image we can't identify must not be added to the library
TEST_F(ContentIdentifierTest, UnidentifiableDiscIsInvalid) {
  ASSERT_TRUE(tempDir.isValid());
  const QString path = writeMagicIso("garbage.iso", "NOT A REAL DISC!");

  const auto result = identifier.identify(path.toStdString());

  ASSERT_FALSE(result.isIdentified());
}

// rcheevos has no handler for a raw sector image, so these consoles are asked outright. Every one
// has to land on a platform, or a disc identifies here and then has nowhere to go — which is
// exactly what 3DO did, hidden by a platform id invented on the spot for anything unmodeled
TEST(DiscInspectorProbeTest, EveryConsoleTheProbeAsksAboutHasAPlatform) {
  const firelight::platforms::PlatformService service;

  ASSERT_GT(std::size(DiscInspector::RAW_SECTOR_CONSOLES), 0u);

  for (const auto rcConsole : DiscInspector::RAW_SECTOR_CONSOLES) {
    EXPECT_NE(service.platformIdForRcConsole(rcConsole), firelight::platforms::PlatformService::PLATFORM_ID_UNKNOWN)
        << rc_console_name(rcConsole) << " is probed but has no platform";
  }
}
} // namespace firelight::library

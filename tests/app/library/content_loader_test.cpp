#include <firelight/library/content_loader.hpp>

#include <firelight/library/content_hasher.hpp>
#include <firelight/library/patch_file.hpp>
#include <firelight/platforms/platform_service.hpp>

#include <QDir>
#include <QTemporaryDir>
#include <archive.h>
#include <archive_entry.h>
#include <gtest/gtest.h>

#include <cstdint>
#include <fstream>
#include <vector>

namespace firelight::library {

namespace {
using PS = firelight::platforms::PlatformService;

std::vector<uint8_t> makeRom(size_t size, uint8_t seed) {
  std::vector<uint8_t> bytes(size);
  for (size_t i = 0; i < size; ++i) {
    bytes[i] = static_cast<uint8_t>((i * 31 + seed) & 0xFF);
  }
  return bytes;
}

void writeFile(const std::string &path, const std::vector<uint8_t> &bytes) {
  std::ofstream out(path, std::ios::binary);
  out.write(reinterpret_cast<const char *>(bytes.data()),
            static_cast<std::streamsize>(bytes.size()));
}
} // namespace

class ContentLoaderTest : public testing::Test {
protected:
  QTemporaryDir tempDir;
  ContentLoader loader;

  ContentFile cartridgeOnDisk(const std::string &path, int platformId) {
    ContentFile cf;
    cf.m_type = ContentType::Cartridge;
    cf.m_filePath = path;
    cf.m_platformId = platformId;
    return cf;
  }

  // Writes a single-entry .zip and returns its path.
  std::string writeZip(const std::string &zipName, const std::string &entryName,
                       const std::vector<uint8_t> &bytes) {
    const std::string zipPath =
        tempDir.filePath(QString::fromStdString(zipName)).toStdString();
    archive *a = archive_write_new();
    archive_write_set_format_zip(a);
    EXPECT_EQ(archive_write_open_filename(a, zipPath.c_str()), ARCHIVE_OK);
    archive_entry *entry = archive_entry_new();
    archive_entry_set_pathname(entry, entryName.c_str());
    archive_entry_set_size(entry, static_cast<la_int64_t>(bytes.size()));
    archive_entry_set_filetype(entry, AE_IFREG);
    archive_entry_set_perm(entry, 0644);
    archive_write_header(a, entry);
    archive_write_data(a, bytes.data(), bytes.size());
    archive_entry_free(entry);
    archive_write_close(a);
    archive_write_free(a);
    return zipPath;
  }
};

// A loose cartridge on disk is read in full and hashed with the platform's
// normalization (Game Boy = whole-file MD5, bytes unchanged).
TEST_F(ContentLoaderTest, LoadsLooseCartridgeAndHashesIt) {
  ASSERT_TRUE(tempDir.isValid());
  const auto rom = makeRom(4096, 1);
  const std::string path = tempDir.filePath("game.gb").toStdString();
  writeFile(path, rom);

  const auto loaded = loader.load(cartridgeOnDisk(path, PS::PLATFORM_ID_GAMEBOY));

  ASSERT_TRUE(loaded.valid);
  EXPECT_EQ(loaded.contentBytes, rom);
  EXPECT_EQ(loaded.contentHash, ContentHasher::md5(rom.data(), rom.size()));
}

// The same bytes inside an archive load to the same content hash as on disk, so
// a ROM is identified identically however it is stored.
TEST_F(ContentLoaderTest, InArchiveMatchesLooseFile) {
  ASSERT_TRUE(tempDir.isValid());
  const auto rom = makeRom(2048, 7);
  const std::string loosePath = tempDir.filePath("loose.gb").toStdString();
  writeFile(loosePath, rom);
  const std::string zipPath = writeZip("game.zip", "inner.gb", rom);

  const auto loose = loader.load(cartridgeOnDisk(loosePath, PS::PLATFORM_ID_GAMEBOY));

  ContentFile archived;
  archived.m_type = ContentType::Cartridge;
  archived.m_inArchive = true;
  archived.m_archivePathName = zipPath;
  archived.m_filePath = "inner.gb";
  archived.m_platformId = PS::PLATFORM_ID_GAMEBOY;
  const auto fromArchive = loader.load(archived);

  ASSERT_TRUE(loose.valid);
  ASSERT_TRUE(fromArchive.valid);
  EXPECT_EQ(fromArchive.contentHash, loose.contentHash);
  EXPECT_EQ(fromArchive.contentBytes, loose.contentBytes);
}

// NES iNES-header stripping: the 16-byte header is excluded from the hash, so a
// headered and header-stripped dump of the same PRG/CHR data hash identically.
TEST_F(ContentLoaderTest, NesHeaderIsStrippedFromHash) {
  ASSERT_TRUE(tempDir.isValid());
  const auto body = makeRom(1024, 3);
  std::vector<uint8_t> headered = {'N', 'E', 'S', 0x1A};
  headered.resize(16, 0); // 16-byte iNES header
  headered.insert(headered.end(), body.begin(), body.end());

  const std::string path = tempDir.filePath("game.nes").toStdString();
  writeFile(path, headered);

  const auto loaded = loader.load(cartridgeOnDisk(path, PS::PLATFORM_ID_NES));

  ASSERT_TRUE(loaded.valid);
  // Bytes are handed to the core unchanged (header included)...
  EXPECT_EQ(loaded.contentBytes, headered);
  // ...but the canonical hash is over the header-stripped body only.
  EXPECT_EQ(loaded.contentHash, ContentHasher::md5(body.data(), body.size()));
}

// A missing file yields an invalid result with no bytes (never a bogus hash).
TEST_F(ContentLoaderTest, MissingFileIsInvalid) {
  const auto loaded = loader.load(
      cartridgeOnDisk(tempDir.filePath("nope.gb").toStdString(),
                      PS::PLATFORM_ID_GAMEBOY));
  EXPECT_FALSE(loaded.valid);
  EXPECT_TRUE(loaded.contentBytes.empty());
  EXPECT_TRUE(loaded.contentHash.empty());
}

// Disc content is opened from its path by the core, so load() only forwards the
// scan-time content hash and reads no bytes.
TEST_F(ContentLoaderTest, DiscForwardsScanTimeHashWithoutReadingBytes) {
  ContentFile disc;
  disc.m_type = ContentType::Disc;
  disc.m_filePath = "/does/not/matter.cue";
  disc.m_contentHash = "abc123";

  const auto loaded = loader.load(disc);

  EXPECT_TRUE(loaded.valid);
  EXPECT_EQ(loaded.contentHash, "abc123");
  EXPECT_TRUE(loaded.contentBytes.empty());
}

TEST_F(ContentLoaderTest, DiscWithoutHashIsInvalid) {
  ContentFile disc;
  disc.m_type = ContentType::Disc;
  disc.m_contentHash = "";
  const auto loaded = loader.load(disc);
  EXPECT_FALSE(loaded.valid);
}

// applyPatch on content with no bytes is a no-op (nothing to patch), leaving the
// content untouched rather than producing a hash of empty bytes.
TEST_F(ContentLoaderTest, ApplyPatchOnEmptyContentIsNoOp) {
  LoadedContent content; // no bytes, invalid
  content.contentHash = "original";
  PatchFile patch; // unloaded

  loader.applyPatch(content, PS::PLATFORM_ID_GAMEBOY, patch);

  EXPECT_TRUE(content.contentBytes.empty());
  EXPECT_EQ(content.contentHash, "original");
}

// A real IPS patch is applied to loaded bytes and the content hash is recomputed
// over the patched result.
TEST_F(ContentLoaderTest, ApplyIpsPatchRewritesBytesAndHash) {
  ASSERT_TRUE(tempDir.isValid());
  const auto rom = makeRom(64, 9);
  const std::string romPath = tempDir.filePath("base.gb").toStdString();
  writeFile(romPath, rom);

  // Minimal IPS: overwrite 3 bytes at offset 4.
  const std::vector<uint8_t> newBytes = {0xAA, 0xBB, 0xCC};
  std::vector<uint8_t> ips = {'P', 'A', 'T', 'C', 'H'};
  ips.insert(ips.end(), {0x00, 0x00, 0x04});       // offset = 4
  ips.insert(ips.end(), {0x00, 0x03});             // size = 3
  ips.insert(ips.end(), newBytes.begin(), newBytes.end());
  ips.insert(ips.end(), {'E', 'O', 'F'});
  const std::string ipsPath = tempDir.filePath("patch.ips").toStdString();
  writeFile(ipsPath, ips);

  PatchFile patch;
  patch.m_filePath = ipsPath;
  ASSERT_TRUE(patch.load());
  ASSERT_EQ(patch.getType(), PatchFile::IPS);

  auto content = loader.load(cartridgeOnDisk(romPath, PS::PLATFORM_ID_GAMEBOY));
  ASSERT_TRUE(content.valid);

  std::vector<uint8_t> expected = rom;
  expected[4] = 0xAA;
  expected[5] = 0xBB;
  expected[6] = 0xCC;

  loader.applyPatch(content, PS::PLATFORM_ID_GAMEBOY, patch);

  EXPECT_EQ(content.contentBytes, expected);
  EXPECT_EQ(content.contentHash,
            ContentHasher::md5(expected.data(), expected.size()));
  EXPECT_NE(content.contentHash, ContentHasher::md5(rom.data(), rom.size()));
}

} // namespace firelight::library

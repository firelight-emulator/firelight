
#include "../../../src/app/patching/bps_patch.hpp"
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
// #include <openssl/crypto.h>
//
#include <zlib.h>
class BPSPatchTest : public testing::Test {};

TEST_F(BPSPatchTest, ConstructorTest) {
  const firelight::patching::BPSPatch patch("test_resources/wellformatted.bps");
  ASSERT_TRUE(patch.isValid());

  ASSERT_EQ(patch.getInputFileSize(), 41943040);
  ASSERT_EQ(patch.getOutputFileSize(), 42467200);
  ASSERT_EQ(patch.getMetadata(), "");
  ASSERT_EQ(patch.getActions().size(), 122728);
  ASSERT_EQ(patch.getInputFileCRC32Checksum(), 0xA7F5CD7E);
  ASSERT_EQ(patch.getOutputFileCRC32Checksum(), 0x56438441);
  ASSERT_EQ(patch.getPatchFileCRC32Checksum(), 0x5EB8D13F);
}

TEST_F(BPSPatchTest, PatchRomTest) {
  const firelight::patching::BPSPatch patch("test_resources/wellformatted.bps");

  const auto romPath = "test_resources/testrom.z64";
  const auto romSize = std::filesystem::file_size(romPath);

  std::ifstream romFile(romPath, std::ios::binary);
  std::vector<uint8_t> romData(romSize);
  romFile.read(reinterpret_cast<char *>(romData.data()), romSize);

  romFile.close();

  const auto patchedData = patch.patchRom(romData);
  uint32_t crc = crc32(0L, nullptr, 0);
  crc = crc32(crc, patchedData.data(), patchedData.size());

  ASSERT_EQ(crc, patch.getOutputFileCRC32Checksum());
}

#include "../../../src/app/patching/bps_patch.hpp"
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
//
#include <zlib.h>

/**
 * @brief Test fixture for BPS (Binary Patch System) patch functionality
 * 
 * Tests the BPS patch format implementation, including patch parsing, metadata
 * extraction, CRC32 validation, and ROM patching operations. BPS is a more
 * advanced binary patch format with built-in error detection and metadata support.
 */
class BPSPatchTest : public testing::Test {};

/**
 * @brief Test BPS patch constructor and metadata parsing
 * 
 * Verifies that a BPS patch file is correctly parsed, extracting input/output
 * file sizes, metadata, patch actions, and CRC32 checksums. Tests against a
 * well-formatted BPS file with expected file sizes and 122,728 patch actions.
 */
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

/**
 * @brief Test BPS patch application and CRC32 validation
 * 
 * Verifies that a BPS patch can be successfully applied to ROM data and that
 * the resulting patched data matches the expected CRC32 checksum. Tests both
 * the patching functionality and integrity validation using a test ROM file.
 */
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
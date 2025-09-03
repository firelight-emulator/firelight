
#include "../../../src/app/patching/ups_patch.hpp"
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

#include <zlib.h>

/**
 * @brief Test fixture for UPS (Universal Patching System) patch functionality
 * 
 * Tests the UPS patch format implementation, including patch parsing, record
 * extraction, CRC32 validation, and ROM patching operations. UPS is a universal
 * binary patch format that supports bidirectional patching and integrity checks.
 */
class UPSPatchTest : public testing::Test {
protected:
  // Common setup code that runs before each test case
  void SetUp() override {
    // Initialize any common data or resources here
  }

  // Common cleanup code that runs after each test case
  void TearDown() override {
    // Clean up any common data or resources here
  }

  // You can add member variables here if needed
};

/**
 * @brief Test UPS patch constructor and record validation
 * 
 * Verifies that a UPS patch file is correctly parsed, extracting patch records,
 * input/output file sizes, and CRC32 checksums. Tests against a well-formatted
 * UPS file with 1,069,392 patch records and validates input ROM CRC32 checksum.
 */
TEST_F(UPSPatchTest, ConstructorTest) {
  const firelight::patching::UPSPatch patch("test_resources/wellformatted.ups");
  ASSERT_TRUE(patch.isValid());

  const auto romPath = "test_resources/testrom.gba";
  const auto romSize = std::filesystem::file_size(romPath);

  std::ifstream romFile(romPath, std::ios::binary);
  std::vector<uint8_t> romData(romSize);
  romFile.read(reinterpret_cast<char *>(romData.data()), romSize);

  romFile.close();

  ASSERT_EQ(patch.getRecords().size(), 1069392);
  ASSERT_EQ(patch.getInputFileSize(), 16777216);
  ASSERT_EQ(patch.getOutputFileSize(), 33554432);
  ASSERT_EQ(patch.getInputFileCRC32Checksum(), 0xDD88761C);
  ASSERT_EQ(patch.getOutputFileCRC32Checksum(), 0xFBA55DD8);
  ASSERT_EQ(patch.getPatchFileCRC32Checksum(), 0x69D43AD1);

  uint32_t crc = crc32(0L, nullptr, 0);
  crc = crc32(crc, romData.data(), romData.size());
  ASSERT_EQ(crc, patch.getInputFileCRC32Checksum());
}

/**
 * @brief Test UPS patch application and output validation
 * 
 * Verifies that a UPS patch can be successfully applied to ROM data and that
 * the resulting patched data matches the expected output CRC32 checksum. Tests
 * the core patching functionality using a GBA ROM file.
 */
TEST_F(UPSPatchTest, PatchRomTest) {
  const firelight::patching::UPSPatch patch("test_resources/wellformatted.ups");

  const auto romPath = "test_resources/testrom.gba";
  const auto romSize = std::filesystem::file_size(romPath);

  std::ifstream romFile(romPath, std::ios::binary);
  std::vector<uint8_t> romData(romSize);
  romFile.read(reinterpret_cast<char *>(romData.data()), romSize);

  romFile.close();

  auto patchedData = patch.patchRom(romData);
  uint32_t crc = crc32(0L, nullptr, 0);
  crc = crc32(crc, patchedData.data(), patchedData.size());

  ASSERT_EQ(crc, patch.getOutputFileCRC32Checksum());
}

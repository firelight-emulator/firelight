
#include "../../../src/app/patching/ups_patch.hpp"
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

#include <zlib.h>

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

#include "../../../src/app/patching/pm_star_rod_mod_patch.hpp"
#include "../../../src/app/patching/yay_0_codec.hpp"
#include "gtest/gtest.h"
#include <filesystem>
#include <fstream>

/**
 * @brief Test fixture for Paper Mario Star Rod mod patch functionality
 * 
 * Tests the PM Star Rod mod patch format implementation, including parsing
 * of compressed .mod files, Yay0 decompression, patch record extraction,
 * and ROM patching operations specific to Paper Mario ROM hacks.
 */
class PMStarRodModPatchTest : public testing::Test {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

/**
 * @brief Test PM Star Rod mod patch constructor and record parsing
 * 
 * Verifies that a compressed .mod file can be loaded, decompressed using Yay0,
 * and parsed to extract patch records. Tests against a well-formatted mod file
 * with an expected 8,204 patch records.
 */
TEST_F(PMStarRodModPatchTest, ConstructorTest) {
  const auto path = "test_resources/wellformatted.mod";
  const auto size = std::filesystem::file_size(path);

  std::ifstream file(path, std::ios::binary);

  std::vector<uint8_t> data(size);
  file.read(reinterpret_cast<char *>(data.data()), size);

  file.close();

  const auto decompressed =
      firelight::patching::Yay0Codec::decompress(data.data());

  auto patch = firelight::patching::PMStarRodModPatch(decompressed);
  ASSERT_EQ(patch.getRecords().size(), 8204);
}

/**
 * @brief Test PM Star Rod mod patch application to Paper Mario ROM
 * 
 * Verifies that a PM Star Rod mod patch can be successfully applied to a
 * Paper Mario ROM file, producing the expected output size. Tests the complete
 * workflow of loading, decompressing, and applying a mod patch.
 */
TEST_F(PMStarRodModPatchTest, PatchRomTest) {
  const auto path = "test_resources/wellformatted.mod";
  const auto size = std::filesystem::file_size(path);

  std::ifstream file(path, std::ios::binary);

  std::vector<uint8_t> data(size);
  file.read(reinterpret_cast<char *>(data.data()), size);

  file.close();

  const auto decompressed =
      firelight::patching::Yay0Codec::decompress(data.data());

  auto patch = firelight::patching::PMStarRodModPatch(decompressed);

  const auto romPath = "test_resources/testrom.z64";
  const auto romSize = std::filesystem::file_size(romPath);

  std::ifstream romFile(romPath, std::ios::binary);

  std::vector<uint8_t> romData(romSize);
  romFile.read(reinterpret_cast<char *>(romData.data()), romSize);

  romFile.close();

  auto result = patch.patchRom(romData);
  ASSERT_EQ(result.size(), 48369024);
}
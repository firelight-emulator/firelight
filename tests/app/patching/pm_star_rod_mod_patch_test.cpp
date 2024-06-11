#include "../../../src/app/patching/pm_star_rod_mod_patch.hpp"
#include "../../../src/app/patching/yay_0_codec.hpp"
#include "gtest/gtest.h"
#include <filesystem>
#include <fstream>

class PMStarRodModPatchTest : public testing::Test {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

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

#include "../../../src/app/patching/bps_patch.hpp"
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
// #include <openssl/crypto.h>
//
#include <zlib.h>
class BPSPatchTest : public testing::Test {};

TEST_F(BPSPatchTest, ConstructorTest) {
  const auto path = "test_resources/wellformatted.bps";
  const auto size = std::filesystem::file_size(path);

  std::ifstream file(path, std::ios::binary);

  std::vector<uint8_t> data(size);
  file.read(reinterpret_cast<char *>(data.data()), size);

  file.close();

  firelight::patching::BPSPatch patch(data);

  ASSERT_EQ(patch.getInputFileSize(), 41943040);
  ASSERT_EQ(patch.getOutputFileSize(), 42467200);
  ASSERT_EQ(patch.getMetadata(), "");
  ASSERT_EQ(patch.getActions().size(), 122728);
  ASSERT_EQ(patch.getInputFileCRC32Checksum(), 0xA7F5CD7E);
  ASSERT_EQ(patch.getOutputFileCRC32Checksum(), 0x56438441);
  ASSERT_EQ(patch.getPatchFileCRC32Checksum(), 0x5EB8D13F);
  //
  // uint32_t crc = crc32(0L, nullptr, 0);
  // crc = crc32(crc, romData.data(), romData.size());
  // ASSERT_EQ(crc, patch.getInputFileCRC32Checksum());
  //
  auto crc = crc32(0L, nullptr, 0);
  crc = crc32(crc, data.data(), data.size() - 4);
  ASSERT_EQ(crc, patch.getPatchFileCRC32Checksum());
}

TEST_F(BPSPatchTest, PatchRomTest) {
  const auto path = "test_resources/wellformatted.bps";
  const auto size = std::filesystem::file_size(path);

  std::ifstream file(path, std::ios::binary);

  std::vector<uint8_t> data(size);
  file.read(reinterpret_cast<char *>(data.data()), size);

  file.close();

  firelight::patching::BPSPatch patch(data);

  const auto romPath = "test_resources/testrom.z64";
  const auto romSize = std::filesystem::file_size(romPath);

  std::ifstream romFile(romPath, std::ios::binary);
  std::vector<uint8_t> romData(romSize);
  romFile.read(reinterpret_cast<char *>(romData.data()), romSize);

  romFile.close();

  auto patchedData = patch.patchRom(romData);
  uint32_t crc = crc32(0L, nullptr, 0);
  crc = crc32(crc, patchedData.data(), patchedData.size());

  // std::ofstream patchedFile("test_resources/patched.z64", std::ios::binary);
  // patchedFile.write(reinterpret_cast<const char *>(patchedData.data()),
  //                   patchedData.size());
  // patchedFile.close();

  ASSERT_EQ(crc, patch.getOutputFileCRC32Checksum());
}
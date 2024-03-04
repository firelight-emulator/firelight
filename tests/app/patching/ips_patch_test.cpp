//
// Created by alexs on 1/11/2024.
//

#include "../../../src/app/patching/ips_patch.hpp"

#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <spdlog/spdlog.h>
class IPSPatchTest : public testing::Test {
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

// Test case for PMStarRodModPatch constructor
TEST_F(IPSPatchTest, ConstructorTest) {
  // Each one is offset, size, repeat
  std::vector<std::tuple<uint32_t, uint32_t, uint32_t>> expectedValues = {
      {47006, 4, 1},      {48398, 4, 1},      {48412, 4, 1},
      {48518, 4, 1},      {58582, 1, 1},      {65495, 9, 1},
      {98516, 119, 1},    {101395, 157, 1},   {102111, 4, 1},
      {102164, 4, 1},     {102721, 4, 1},     {106027, 4, 1},
      {117013, 40, 1},    {117300, 23, 1},    {117384, 173, 1},
      {122001, 52, 1},    {122325, 13, 1},    {124561, 1, 1},
      {124601, 1, 1},     {124657, 1, 1},     {124803, 1, 1},
      {125330, 4, 1},     {125713, 4, 1},     {125787, 60, 1},
      {125859, 45, 1},    {125934, 3, 1},     {125949, 27, 1},
      {130003, 1, 1},     {130971, 96, 1},    {318140, 4, 1},
      {318633, 1, 1},     {318653, 6, 1},     {325729, 81, 1},
      {325831, 34, 1},    {368970, 13, 1},    {368992, 92, 1},
      {369084, 1, 16},    {376502, 13, 1},    {377437, 13, 1},
      {377467, 98, 1},    {377565, 1, 32},    {378888, 24, 1},
      {379008, 24, 1},    {379072, 32, 1},    {379204, 28, 1},
      {379264, 8, 1},     {379336, 8, 1},     {379464, 8, 1},
      {379480, 8, 1},     {379520, 30, 1},    {379592, 8, 1},
      {379608, 8, 1},     {379648, 16, 1},    {379712, 2, 1},
      {379784, 24, 1},    {379840, 32, 1},    {379976, 8, 1},
      {379992, 16, 1},    {380104, 10, 1},    {380120, 8, 1},
      {380928, 105, 1},   {381056, 224, 1},   {381312, 102, 1},
      {381440, 105, 1},   {381568, 114, 1},   {381696, 87, 1},
      {381824, 63, 1},    {381955, 117, 1},   {382080, 105, 1},
      {382208, 99, 1},    {382336, 114, 1},   {382467, 96, 1},
      {382592, 99, 1},    {382720, 93, 1},    {382848, 227, 1},
      {383104, 102, 1},   {383232, 117, 1},   {383360, 90, 1},
      {524288, 817, 1},   {525105, 1, 131},   {525236, 125, 1},
      {525361, 1, 770},   {526131, 126, 1},   {526257, 1, 900},
      {527157, 124, 1},   {527281, 1, 1408},  {528689, 172, 1},
      {528861, 1, 2900},  {531761, 120, 1},   {531881, 1, 3976},
      {535857, 166, 1},   {536023, 1, 8026},  {544049, 114, 1},
      {544163, 1, 3982},  {548145, 96, 1},    {548241, 1, 24},
      {548265, 1, 1},     {548266, 1, 18},    {548284, 407, 1},
      {548691, 1, 151},   {548842, 1, 16},    {548858, 1, 16},
      {548874, 1, 16},    {548890, 98, 1},    {548988, 1, 16},
      {549004, 46, 1},    {549050, 1, 48},    {549098, 1, 16},
      {549114, 1, 16},    {549130, 1, 16},    {549146, 132, 1},
      {549278, 1, 12},    {549290, 1, 64},    {549354, 1, 16},
      {549370, 1, 16},    {549386, 1, 16},    {549402, 32, 1},
      {549434, 1, 14},    {549448, 2, 1},     {549450, 1, 14},
      {549464, 2, 1},     {549466, 1, 14},    {549480, 25, 1},
      {549505, 1, 13},    {549518, 1, 92},    {549610, 1, 16},
      {549626, 1, 16},    {549642, 1, 16},    {549658, 62, 1},
      {549720, 1, 21},    {549741, 59, 1},    {549800, 1, 66},
      {549866, 1, 16},    {549882, 1, 16},    {549898, 1, 16},
      {549914, 20, 1},    {549934, 1, 19},    {549953, 73, 1},
      {550026, 1, 20},    {550046, 1, 9},     {550055, 3, 1},
      {550058, 1, 64},    {550122, 1, 16},    {550138, 1, 16},
      {550154, 1, 16},    {550170, 42, 1},    {550212, 1, 50},
      {550262, 62, 1},    {550324, 1, 54},    {550378, 1, 16},
      {550394, 1, 16},    {550410, 1, 16},    {550426, 28, 1},
      {550454, 1, 180},   {550634, 1, 16},    {550650, 1, 16},
      {550666, 1, 16},    {550682, 36791, 1}, {587473, 1, 2351},
      {589824, 55941, 1}, {645765, 1, 65535}, {711300, 1, 65535},
      {776835, 1, 9597}};

  const auto path = "test_resources/wellformatted.ips";
  const auto size = std::filesystem::file_size(path);

  std::ifstream file(path, std::ios::binary);

  std::vector<uint8_t> data(size);
  file.read(reinterpret_cast<char *>(data.data()), size);

  file.close();

  firelight::patching::IPSPatch patch(data);

  constexpr int expectedNumRecords = 163;
  ASSERT_EQ(patch.getRecords().size(), expectedNumRecords);

  for (int i = 0; i < expectedNumRecords; ++i) {
    ASSERT_EQ(patch.getRecords()[i].offset, std::get<0>(expectedValues[i]));
    ASSERT_EQ(patch.getRecords()[i].data.size(),
              std::get<1>(expectedValues[i]));
    ASSERT_EQ(patch.getRecords()[i].numTimesToWrite,
              std::get<2>(expectedValues[i]));
  }
}

TEST_F(IPSPatchTest, PatchRomTest) {
  const std::vector<uint8_t> romData(4000000);

  const auto path = "test_resources/wellformatted.ips";
  const auto size = std::filesystem::file_size(path);

  std::ifstream file(path, std::ios::binary);

  std::vector<uint8_t> patchData(size);
  file.read(reinterpret_cast<char *>(patchData.data()), size);

  file.close();

  const firelight::patching::IPSPatch patch(patchData);

  ASSERT_FALSE(patch.patchRom(romData).empty());
}
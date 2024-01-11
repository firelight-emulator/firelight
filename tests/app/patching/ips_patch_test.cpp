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
      {47006, 4, 0},      {48398, 4, 0},      {48412, 4, 0},
      {48518, 4, 0},      {58582, 1, 0},      {65495, 9, 0},
      {98516, 119, 0},    {101395, 157, 0},   {102111, 4, 0},
      {102164, 4, 0},     {102721, 4, 0},     {106027, 4, 0},
      {117013, 40, 0},    {117300, 23, 0},    {117384, 173, 0},
      {122001, 52, 0},    {122325, 13, 0},    {124561, 1, 0},
      {124601, 1, 0},     {124657, 1, 0},     {124803, 1, 0},
      {125330, 4, 0},     {125713, 4, 0},     {125787, 60, 0},
      {125859, 45, 0},    {125934, 3, 0},     {125949, 27, 0},
      {130003, 1, 0},     {130971, 96, 0},    {318140, 4, 0},
      {318633, 1, 0},     {318653, 6, 0},     {325729, 81, 0},
      {325831, 34, 0},    {368970, 13, 0},    {368992, 92, 0},
      {369084, 1, 16},    {376502, 13, 0},    {377437, 13, 0},
      {377467, 98, 0},    {377565, 1, 32},    {378888, 24, 0},
      {379008, 24, 0},    {379072, 32, 0},    {379204, 28, 0},
      {379264, 8, 0},     {379336, 8, 0},     {379464, 8, 0},
      {379480, 8, 0},     {379520, 30, 0},    {379592, 8, 0},
      {379608, 8, 0},     {379648, 16, 0},    {379712, 2, 0},
      {379784, 24, 0},    {379840, 32, 0},    {379976, 8, 0},
      {379992, 16, 0},    {380104, 10, 0},    {380120, 8, 0},
      {380928, 105, 0},   {381056, 224, 0},   {381312, 102, 0},
      {381440, 105, 0},   {381568, 114, 0},   {381696, 87, 0},
      {381824, 63, 0},    {381955, 117, 0},   {382080, 105, 0},
      {382208, 99, 0},    {382336, 114, 0},   {382467, 96, 0},
      {382592, 99, 0},    {382720, 93, 0},    {382848, 227, 0},
      {383104, 102, 0},   {383232, 117, 0},   {383360, 90, 0},
      {524288, 817, 0},   {525105, 1, 131},   {525236, 125, 0},
      {525361, 1, 770},   {526131, 126, 0},   {526257, 1, 900},
      {527157, 124, 0},   {527281, 1, 1408},  {528689, 172, 0},
      {528861, 1, 2900},  {531761, 120, 0},   {531881, 1, 3976},
      {535857, 166, 0},   {536023, 1, 8026},  {544049, 114, 0},
      {544163, 1, 3982},  {548145, 96, 0},    {548241, 1, 24},
      {548265, 1, 0},     {548266, 1, 18},    {548284, 407, 0},
      {548691, 1, 151},   {548842, 1, 16},    {548858, 1, 16},
      {548874, 1, 16},    {548890, 98, 0},    {548988, 1, 16},
      {549004, 46, 0},    {549050, 1, 48},    {549098, 1, 16},
      {549114, 1, 16},    {549130, 1, 16},    {549146, 132, 0},
      {549278, 1, 12},    {549290, 1, 64},    {549354, 1, 16},
      {549370, 1, 16},    {549386, 1, 16},    {549402, 32, 0},
      {549434, 1, 14},    {549448, 2, 0},     {549450, 1, 14},
      {549464, 2, 0},     {549466, 1, 14},    {549480, 25, 0},
      {549505, 1, 13},    {549518, 1, 92},    {549610, 1, 16},
      {549626, 1, 16},    {549642, 1, 16},    {549658, 62, 0},
      {549720, 1, 21},    {549741, 59, 0},    {549800, 1, 66},
      {549866, 1, 16},    {549882, 1, 16},    {549898, 1, 16},
      {549914, 20, 0},    {549934, 1, 19},    {549953, 73, 0},
      {550026, 1, 20},    {550046, 1, 9},     {550055, 3, 0},
      {550058, 1, 64},    {550122, 1, 16},    {550138, 1, 16},
      {550154, 1, 16},    {550170, 42, 0},    {550212, 1, 50},
      {550262, 62, 0},    {550324, 1, 54},    {550378, 1, 16},
      {550394, 1, 16},    {550410, 1, 16},    {550426, 28, 0},
      {550454, 1, 180},   {550634, 1, 16},    {550650, 1, 16},
      {550666, 1, 16},    {550682, 36791, 0}, {587473, 1, 2351},
      {589824, 55941, 0}, {645765, 1, 65535}, {711300, 1, 65535},
      {776835, 1, 9597}};

  const auto path = "test_resources/wellformatted.ips";
  const auto size = std::filesystem::file_size(path);

  std::ifstream file(path, std::ios::binary);

  std::vector<uint8_t> data(size);
  file.read(reinterpret_cast<char *>(data.data()), size);

  file.close();

  Firelight::Patching::IPSPatch patch(data);

  constexpr int expectedNumRecords = 163;
  ASSERT_EQ(patch.getRecords().size(), expectedNumRecords);

  for (int i = 0; i < expectedNumRecords; ++i) {
    ASSERT_EQ(patch.getRecords()[i].offset, std::get<0>(expectedValues[i]));
    ASSERT_EQ(patch.getRecords()[i].data.size(),
              std::get<1>(expectedValues[i]));
    ASSERT_EQ(patch.getRecords()[i].numTimesToWrite,
              std::get<2>(expectedValues[i]));
  }

  // for (const auto& tuple : expectedValues) {
  //   std::cout << "Offset: " << std::get<0>(tuple)
  //             << ", Size: " << std::get<1>(tuple)
  //             << ", Repeat: " << std::get<2>(tuple) << std::endl;
  // }

  //  // Create test data for the constructor
  //  std::vector<uint8_t> testData = {
  //      // Add your test data here
  //  };
  //
  //  // Create an instance of the PMStarRodModPatch class
  //  FL::Patching::PMStarRodModPatch patch(testData);
  //
  //  // Add assertions to check the state of the patch object
  //  // For example, you can check if records were correctly parsed and stored
  //  ASSERT_EQ(patch.getRecords().size(), 5);
  //  // Add more assertions as needed
}
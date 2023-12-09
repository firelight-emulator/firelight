#include "../../../src/app/patching/pm_star_rod_mod_patch.hpp"
#include "gtest/gtest.h"
#include <filesystem>

// Test fixture for PMStarRodModPatch class
class PMStarRodModPatchTest : public testing::Test {
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
TEST_F(PMStarRodModPatchTest, ConstructorTest) {
  std::cout << "Current working directory: " << std::filesystem::current_path()
            << std::endl;
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

// Test case for PMStarRodModPatch patchRom method
TEST_F(PMStarRodModPatchTest, PatchRomTest) {
  std::cout << "Hey there" << std::endl;
  // Create an instance of the PMStarRodModPatch class (you can use a fixture
  // member)
  //  FL::Patching::PMStarRodModPatch patch({});
  //
  //  // Create test data for patching (original ROM data)
  //  std::vector<uint8_t> originalData = {
  //      // Add your original data here
  //  };
  //
  //  // Apply the patch using the patchRom method
  //  std::vector<uint8_t> patchedData = patch.patchRom(originalData);
  //
  //  // Add assertions to check if the patching was successful
  //  // For example, you can check if specific bytes in patchedData match
  //  expected
  //  // values
  //  ASSERT_EQ(patchedData.size(), 5);
  // Add more assertions as needed
}

// You can add more test cases to cover various scenarios and edge cases
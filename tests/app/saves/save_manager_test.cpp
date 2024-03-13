#include <gtest/gtest.h>

namespace firelight::saves {

class SaveManagerTest : public testing::Test {
protected:
  void SetUp() override {}

  void TearDown() override {}
};

// Save file only writes if new/different
// Save file metadata is updated/created correctly
// Save file is written to the correct location
// Save file is read from the correct location
// something about locks???
// Save file slot number is used correctly

} // namespace firelight::saves

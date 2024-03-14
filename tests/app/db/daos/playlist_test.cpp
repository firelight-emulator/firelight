#include "../../../src/app/db/daos/playlist.hpp"
#include <gtest/gtest.h>

namespace firelight::db {
class PlaylistTest : public testing::Test {
protected:
  void SetUp() override {}

  void TearDown() override {}
};

TEST_F(PlaylistTest, DefaultValuesTest) {
  const Playlist playlist;

  ASSERT_EQ(playlist.id, -1);
  ASSERT_EQ(playlist.displayName, "");
}

} // namespace firelight::db

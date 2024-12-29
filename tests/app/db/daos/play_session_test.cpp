#include "firelight/activity/play_session.hpp"
#include <gtest/gtest.h>

namespace firelight::db {
class PlaySessionTest : public testing::Test {
protected:
  void SetUp() override {}

  void TearDown() override {}
};

TEST_F(PlaySessionTest, DefaultValuesTest) {
  PlaySession session;

  ASSERT_EQ(session.id, -1);
  // ASSERT_EQ(session.displayName, "");
}

} // namespace firelight::db

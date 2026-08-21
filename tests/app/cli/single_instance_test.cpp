#include "cli/single_instance.hpp"

#include <gtest/gtest.h>

namespace firelight::cli {

TEST(SingleInstanceTest, ServerNameIsDeterministic) {
  const auto a = singleInstanceServerName("C:/Users/x/AppData/Firelight");
  const auto b = singleInstanceServerName("C:/Users/x/AppData/Firelight");
  EXPECT_EQ(a, b);
}

TEST(SingleInstanceTest, ServerNameIsScopedToDataDir) {
  // Distinct data dirs (e.g. different --config-dir) must not collide
  const auto a = singleInstanceServerName("C:/Users/x/AppData/Firelight");
  const auto b = singleInstanceServerName("C:/tmp/other-instance");
  EXPECT_NE(a, b);
}

TEST(SingleInstanceTest, ServerNameHasStablePrefix) {
  const auto name = singleInstanceServerName("/home/user/.local/share/Firelight");
  EXPECT_FALSE(name.isEmpty());
  EXPECT_TRUE(name.startsWith("firelight-"));
}

} // namespace firelight::cli

#include <gtest/gtest.h>
#include <rcheevos/offline/ra_cache.hpp>

namespace firelight::achievements {

class RetroAchievementsCacheTest : public testing::Test {};

TEST_F(RetroAchievementsCacheTest, TablesExist) {
  const auto cache = RetroAchievementsCache(":memory:");
  ASSERT_TRUE(cache.tableExists("achievement_sets"));
  ASSERT_TRUE(cache.tableExists("achievements"));
  ASSERT_TRUE(cache.tableExists("hashes"));
  ASSERT_TRUE(cache.tableExists("patch_response_cache"));
  ASSERT_TRUE(cache.tableExists("user_unlocks"));
  ASSERT_TRUE(cache.tableExists("users"));
}

TEST_F(RetroAchievementsCacheTest, UserNotExistsReturnsEmpty) {
  const auto cache = RetroAchievementsCache(":memory:");

  const auto user = cache.getUser("testuser");
  ASSERT_FALSE(user.has_value());
}

TEST_F(RetroAchievementsCacheTest, CreateUserSuccessfully) {
  const auto cache = RetroAchievementsCache(":memory:");

  auto user = cache.getUser("testuser");
  ASSERT_FALSE(user.has_value());

  cache.createUser("testuser", "testtoken", 100, 200);

  user = cache.getUser("testuser");
  ASSERT_TRUE(user.has_value());
  ASSERT_EQ(user->username, "testuser");
  ASSERT_EQ(user->token, "testtoken");
  ASSERT_EQ(user->points, 100);
  ASSERT_EQ(user->hardcore_points, 200);
}

TEST_F(RetroAchievementsCacheTest, GetUserScoreWhenNotExistsReturnsZero) {
  const auto cache = RetroAchievementsCache(":memory:");

  const auto score = cache.getUserScore("testuser", false);
  ASSERT_EQ(score, 0);

  const auto hardcoreScore = cache.getUserScore("testuser", true);
  ASSERT_EQ(hardcoreScore, 0);
}

TEST_F(RetroAchievementsCacheTest, CreateAchievementSuccess) {
  const auto cache = RetroAchievementsCache(":memory:");

  const auto achieve = cache.getAchievement(1);
  ASSERT_FALSE(achieve.has_value());

  auto newAchievement = Achievement{
      .id = 1,
      .name = "Test Achievement",
      .description = "This is a test achievement",
      .imageUrl = "",
      .points = 10,
      .type = "test",
      .displayOrder = 1,
      .setId = 4,
      .flags = 4,
  };

  auto id = cache.createAchievement(newAchievement);
  ASSERT_EQ(id, 1);

  auto retrievedAchievement = cache.getAchievement(1);
  ASSERT_TRUE(retrievedAchievement.has_value());
  ASSERT_EQ(retrievedAchievement->id, 1);
  ASSERT_EQ(retrievedAchievement->name, "Test Achievement");
  ASSERT_EQ(retrievedAchievement->description, "This is a test achievement");
  ASSERT_EQ(retrievedAchievement->points, 10);
  ASSERT_EQ(retrievedAchievement->setId, 4);
  ASSERT_EQ(retrievedAchievement->flags, 4);
}

// TEST_F(RetroAchievementsCacheTest, SetUserScoreWhenNoUserTest) {
//   auto cache = RetroAchievementsCache(":memory:");
//
//   auto user = cache.getUser("testuser");
//   ASSERT_FALSE(user.has_value());
//
//   cache.setUserScore("testuser", 100, false);
//   ASSERT_EQ(cache.getUserScore("testuser", false), 100);
//
//   // user = cache.getUser("testuser");
//   //
//   // ASSERT_TRUE(user.has_value());
//   // ASSERT_EQ(user->username, "testuser");
//   // ASSERT_EQ(user->points, 100);
//   // ASSERT_EQ(user->hardcore_points, 0);
//   //
//   // cache.setUserScore("testuser", 200, true);
//   // user = cache.getUser("testuser");
//   // ASSERT_TRUE(user.has_value());
//   // ASSERT_EQ(user->username, "testuser");
//   // ASSERT_EQ(user->points, 100);
//   // ASSERT_EQ(user->hardcore_points, 200);
// }

} // namespace firelight::achievements

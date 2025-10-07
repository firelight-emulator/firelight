#include "../../../src/app/achievements/sqlite_achievement_repository.hpp"
#include <filesystem>
#include <gtest/gtest.h>

namespace firelight::achievements {

/**
 * @brief Test fixture for SqliteAchievementRepository functionality
 *
 * Comprehensive test suite for the SQLite-based achievement repository
 * implementation, covering CRUD operations for achievement sets, achievements,
 * progress tracking, and hash-based lookups.
 */
class SqliteAchievementRepositoryTest : public testing::Test {
protected:
  std::unique_ptr<SqliteAchievementRepository> repository;

  void SetUp() override {
    repository = std::make_unique<SqliteAchievementRepository>(":memory:");
  }

  void TearDown() override { repository.reset(); }

  AchievementSet createTestAchievementSet() {
    AchievementSet achievementSet;
    achievementSet.id = 1;
    achievementSet.name = "Test Game Achievements";
    achievementSet.iconUrl = "https://example.com/icon.png";
    achievementSet.numAchievements = 3;
    achievementSet.totalPoints = 150;
    return achievementSet;
  }

  Achievement createTestAchievement(unsigned id = 100, unsigned setId = 1) {
    Achievement achievement;
    achievement.id = id;
    achievement.name = "Test Achievement";
    achievement.description = "Complete a test task";
    achievement.imageUrl = "https://example.com/achievement.png";
    achievement.points = 50;
    achievement.type = "progression";
    achievement.displayOrder = 1;
    achievement.setId = setId;
    achievement.flags = 3; // Active achievement
    return achievement;
  }

  AchievementProgress
  createTestProgress(const std::string &username = "testuser",
                     unsigned achievementId = 100) {
    AchievementProgress progress;
    progress.username = username;
    progress.achievementId = achievementId;
    progress.numerator = 5;
    progress.denominator = 10;
    return progress;
  }

  UserUnlock createTestUserUnlock(const std::string &username = "testuser",
                                  unsigned achievementId = 100) {
    UserUnlock unlock;
    unlock.username = username;
    unlock.achievementId = achievementId;
    unlock.earned = true;
    unlock.earnedHardcore = false;
    unlock.unlockTimestamp = 1609459200; // 2021-01-01 00:00:00 UTC
    unlock.unlockTimestampHardcore = 0;
    unlock.synced = false;
    return unlock;
  }

  User createTestUser(const std::string &username = "testuser",
                      const std::string &token = "test_token_123",
                      int points = 1500, int hardcorePoints = 750) {
    User user;
    user.username = username;
    user.token = token;
    user.points = points;
    user.hardcore_points = hardcorePoints;
    return user;
  }
};

// User Management Tests

TEST_F(SqliteAchievementRepositoryTest, CreateUser_Success) {
  User user = createTestUser("testuser", "test_token_123", 1500, 750);

  bool result = repository->createOrUpdateUser(user);

  EXPECT_TRUE(result);
}

TEST_F(SqliteAchievementRepositoryTest, CreateUser_UpsertBehavior) {
  User user = createTestUser("testuser", "test_token_123", 1500, 750);

  // First creation
  EXPECT_TRUE(repository->createOrUpdateUser(user));

  // Modify and update (should upsert)
  user.token = "updated_token_456";
  user.points = 2000;
  user.hardcore_points = 1000;

  bool result = repository->createOrUpdateUser(user);
  EXPECT_TRUE(result);

  // Verify the update took effect
  auto retrieved = repository->getUser("testuser");
  ASSERT_TRUE(retrieved.has_value());
  EXPECT_EQ(retrieved->username, "testuser");
  EXPECT_EQ(retrieved->token, "updated_token_456");
  EXPECT_EQ(retrieved->points, 2000);
  EXPECT_EQ(retrieved->hardcore_points, 1000);
}

TEST_F(SqliteAchievementRepositoryTest, GetUser_ExistingUser) {
  User user = createTestUser("testuser", "test_token_123", 1500, 750);
  repository->createOrUpdateUser(user);

  auto result = repository->getUser("testuser");

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->username, "testuser");
  EXPECT_EQ(result->token, "test_token_123");
  EXPECT_EQ(result->points, 1500);
  EXPECT_EQ(result->hardcore_points, 750);
}

TEST_F(SqliteAchievementRepositoryTest, GetUser_NonExistentUser) {
  auto result = repository->getUser("nonexistent");

  EXPECT_FALSE(result.has_value());
}

TEST_F(SqliteAchievementRepositoryTest, CreateUser_MultipleUsers) {
  User user1 = createTestUser("user1", "token1", 1000, 500);
  User user2 = createTestUser("user2", "token2", 2000, 1500);

  EXPECT_TRUE(repository->createOrUpdateUser(user1));
  EXPECT_TRUE(repository->createOrUpdateUser(user2));

  // Verify both users exist independently
  auto retrievedUser1 = repository->getUser("user1");
  auto retrievedUser2 = repository->getUser("user2");

  ASSERT_TRUE(retrievedUser1.has_value());
  ASSERT_TRUE(retrievedUser2.has_value());

  EXPECT_EQ(retrievedUser1->username, "user1");
  EXPECT_EQ(retrievedUser1->token, "token1");
  EXPECT_EQ(retrievedUser1->points, 1000);
  EXPECT_EQ(retrievedUser1->hardcore_points, 500);

  EXPECT_EQ(retrievedUser2->username, "user2");
  EXPECT_EQ(retrievedUser2->token, "token2");
  EXPECT_EQ(retrievedUser2->points, 2000);
  EXPECT_EQ(retrievedUser2->hardcore_points, 1500);
}

TEST_F(SqliteAchievementRepositoryTest, CreateUser_ZeroPoints) {
  User user = createTestUser("beginner", "token", 0, 0);

  bool result = repository->createOrUpdateUser(user);

  EXPECT_TRUE(result);

  auto savedUser = repository->getUser("beginner");
  ASSERT_TRUE(savedUser.has_value());
  EXPECT_EQ(savedUser->points, 0);
  EXPECT_EQ(savedUser->hardcore_points, 0);
}

TEST_F(SqliteAchievementRepositoryTest, CreateUser_EmptyToken) {
  User user = createTestUser("test", "", 100, 50);

  bool result = repository->createOrUpdateUser(user);

  EXPECT_TRUE(result);

  auto savedUser = repository->getUser("test");
  ASSERT_TRUE(savedUser.has_value());
  EXPECT_EQ(savedUser->token, "");
}

// ListUsers Tests

TEST_F(SqliteAchievementRepositoryTest, ListUsers_EmptyDatabase) {
  auto result = repository->listUsers();

  EXPECT_TRUE(result.empty());
}

TEST_F(SqliteAchievementRepositoryTest, ListUsers_SingleUser) {
  User user = createTestUser("testuser", "test_token", 1500, 750);
  repository->createOrUpdateUser(user);

  auto result = repository->listUsers();

  ASSERT_EQ(result.size(), 1);
  EXPECT_EQ(result[0].username, "testuser");
  EXPECT_EQ(result[0].token, "test_token");
  EXPECT_EQ(result[0].points, 1500);
  EXPECT_EQ(result[0].hardcore_points, 750);
}

TEST_F(SqliteAchievementRepositoryTest, ListUsers_MultipleUsers) {
  // Create multiple users with different data
  User user1 = createTestUser("alice", "token_alice", 2000, 1800);
  User user2 = createTestUser("bob", "token_bob", 1200, 900);
  User user3 = createTestUser("charlie", "token_charlie", 3500, 2100);

  repository->createOrUpdateUser(user1);
  repository->createOrUpdateUser(user2);
  repository->createOrUpdateUser(user3);

  auto result = repository->listUsers();

  ASSERT_EQ(result.size(), 3);

  // Results should be ordered by username (alice, bob, charlie)
  EXPECT_EQ(result[0].username, "alice");
  EXPECT_EQ(result[0].token, "token_alice");
  EXPECT_EQ(result[0].points, 2000);
  EXPECT_EQ(result[0].hardcore_points, 1800);

  EXPECT_EQ(result[1].username, "bob");
  EXPECT_EQ(result[1].token, "token_bob");
  EXPECT_EQ(result[1].points, 1200);
  EXPECT_EQ(result[1].hardcore_points, 900);

  EXPECT_EQ(result[2].username, "charlie");
  EXPECT_EQ(result[2].token, "token_charlie");
  EXPECT_EQ(result[2].points, 3500);
  EXPECT_EQ(result[2].hardcore_points, 2100);
}

TEST_F(SqliteAchievementRepositoryTest, ListUsers_OrderedByUsername) {
  // Create users in non-alphabetical order
  User userZ = createTestUser("zebra", "token_z", 100, 50);
  User userA = createTestUser("alpha", "token_a", 200, 100);
  User userM = createTestUser("middle", "token_m", 300, 150);

  repository->createOrUpdateUser(userZ);
  repository->createOrUpdateUser(userA);
  repository->createOrUpdateUser(userM);

  auto result = repository->listUsers();

  ASSERT_EQ(result.size(), 3);

  // Should be ordered alphabetically by username
  EXPECT_EQ(result[0].username, "alpha");
  EXPECT_EQ(result[1].username, "middle");
  EXPECT_EQ(result[2].username, "zebra");
}

TEST_F(SqliteAchievementRepositoryTest, ListUsers_WithZeroPoints) {
  User user1 = createTestUser("newbie", "token1", 0, 0);
  User user2 = createTestUser("veteran", "token2", 5000, 3000);

  repository->createOrUpdateUser(user1);
  repository->createOrUpdateUser(user2);

  auto result = repository->listUsers();

  ASSERT_EQ(result.size(), 2);

  // Find the newbie user
  auto newbieIt = std::find_if(result.begin(), result.end(), [](const User &u) {
    return u.username == "newbie";
  });
  ASSERT_NE(newbieIt, result.end());
  EXPECT_EQ(newbieIt->points, 0);
  EXPECT_EQ(newbieIt->hardcore_points, 0);

  // Find the veteran user
  auto veteranIt =
      std::find_if(result.begin(), result.end(),
                   [](const User &u) { return u.username == "veteran"; });
  ASSERT_NE(veteranIt, result.end());
  EXPECT_EQ(veteranIt->points, 5000);
  EXPECT_EQ(veteranIt->hardcore_points, 3000);
}

TEST_F(SqliteAchievementRepositoryTest, ListUsers_WithEmptyTokens) {
  User user1 = createTestUser("user1", "", 1000, 500);
  User user2 = createTestUser("user2", "real_token", 2000, 1000);

  repository->createOrUpdateUser(user1);
  repository->createOrUpdateUser(user2);

  auto result = repository->listUsers();

  ASSERT_EQ(result.size(), 2);

  // Find user with empty token
  auto emptyTokenIt =
      std::find_if(result.begin(), result.end(),
                   [](const User &u) { return u.username == "user1"; });
  ASSERT_NE(emptyTokenIt, result.end());
  EXPECT_EQ(emptyTokenIt->token, "");

  // Find user with real token
  auto realTokenIt =
      std::find_if(result.begin(), result.end(),
                   [](const User &u) { return u.username == "user2"; });
  ASSERT_NE(realTokenIt, result.end());
  EXPECT_EQ(realTokenIt->token, "real_token");
}

TEST_F(SqliteAchievementRepositoryTest, ListUsers_AfterUserUpdate) {
  User user = createTestUser("updatable", "old_token", 1000, 500);
  repository->createOrUpdateUser(user);

  // Update the user
  user.token = "new_token";
  user.points = 2000;
  user.hardcore_points = 1500;
  repository->createOrUpdateUser(user);

  auto result = repository->listUsers();

  ASSERT_EQ(result.size(), 1);
  EXPECT_EQ(result[0].username, "updatable");
  EXPECT_EQ(result[0].token, "new_token");
  EXPECT_EQ(result[0].points, 2000);
  EXPECT_EQ(result[0].hardcore_points, 1500);
}

TEST_F(SqliteAchievementRepositoryTest, ListUsers_LargeDataset) {
  // Create a larger number of users to test performance and ordering
  const int userCount = 50;

  for (int i = 0; i < userCount; ++i) {
    std::string username = "user" + std::to_string(i);
    std::string token = "token" + std::to_string(i);
    int points = i * 100;
    int hardcorePoints = i * 50;

    User user = createTestUser(username, token, points, hardcorePoints);
    repository->createOrUpdateUser(user);
  }

  auto result = repository->listUsers();

  ASSERT_EQ(result.size(), userCount);

  // Verify ordering - should be alphabetically sorted
  for (int i = 0; i < userCount - 1; ++i) {
    EXPECT_LT(result[i].username, result[i + 1].username);
  }

  // Verify a few specific users
  auto user0It = std::find_if(result.begin(), result.end(), [](const User &u) {
    return u.username == "user0";
  });
  ASSERT_NE(user0It, result.end());
  EXPECT_EQ(user0It->points, 0);
  EXPECT_EQ(user0It->hardcore_points, 0);

  auto user25It = std::find_if(result.begin(), result.end(), [](const User &u) {
    return u.username == "user25";
  });
  ASSERT_NE(user25It, result.end());
  EXPECT_EQ(user25It->points, 2500);
  EXPECT_EQ(user25It->hardcore_points, 1250);
}

// Achievement Set CRUD Tests

TEST_F(SqliteAchievementRepositoryTest, CreateAchievementSet_Success) {
  AchievementSet achievementSet = createTestAchievementSet();

  bool result = repository->create(achievementSet);

  EXPECT_TRUE(result);
}

TEST_F(SqliteAchievementRepositoryTest, CreateAchievementSet_UpsertBehavior) {
  AchievementSet achievementSet = createTestAchievementSet();

  // First creation
  EXPECT_TRUE(repository->create(achievementSet));

  // Modify and createOrUpdate again (should update)
  achievementSet.name = "Updated Game Achievements";
  achievementSet.totalPoints = 200;
  EXPECT_TRUE(repository->create(achievementSet));

  // Verify the update took effect
  auto retrieved = repository->getAchievementSet(achievementSet.id);
  ASSERT_TRUE(retrieved.has_value());
  EXPECT_EQ(retrieved->name, "Updated Game Achievements");
  EXPECT_EQ(retrieved->totalPoints, 200);
}

TEST_F(SqliteAchievementRepositoryTest, GetAchievementSet_ExistingSet) {
  AchievementSet achievementSet = createTestAchievementSet();
  repository->create(achievementSet);

  auto result = repository->getAchievementSet(achievementSet.id);

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->id, achievementSet.id);
  EXPECT_EQ(result->name, achievementSet.name);
  EXPECT_EQ(result->iconUrl, achievementSet.iconUrl);
  EXPECT_EQ(result->numAchievements, achievementSet.numAchievements);
  EXPECT_EQ(result->totalPoints, achievementSet.totalPoints);
  EXPECT_TRUE(result->achievements.empty()); // No achievements added yet
}

TEST_F(SqliteAchievementRepositoryTest, GetAchievementSet_NonExistentSet) {
  auto result = repository->getAchievementSet(999);

  EXPECT_FALSE(result.has_value());
}

TEST_F(SqliteAchievementRepositoryTest, UpdateAchievementSet_Success) {
  AchievementSet achievementSet = createTestAchievementSet();
  repository->create(achievementSet);

  achievementSet.name = "Updated Achievement Set";
  achievementSet.totalPoints = 300;
  achievementSet.iconUrl = "https://example.com/new-icon.png";

  bool result = repository->update(achievementSet);

  EXPECT_TRUE(result);

  auto retrieved = repository->getAchievementSet(achievementSet.id);
  ASSERT_TRUE(retrieved.has_value());
  EXPECT_EQ(retrieved->name, "Updated Achievement Set");
  EXPECT_EQ(retrieved->totalPoints, 300);
  EXPECT_EQ(retrieved->iconUrl, "https://example.com/new-icon.png");
}

TEST_F(SqliteAchievementRepositoryTest, UpdateAchievementSet_NonExistentSet) {
  AchievementSet achievementSet = createTestAchievementSet();
  achievementSet.id = 999;

  bool result = repository->update(achievementSet);

  EXPECT_FALSE(result); // Should return false for non-existent set
}

// Achievement CRUD Tests

TEST_F(SqliteAchievementRepositoryTest, GetAchievement_ExistingAchievement) {
  AchievementSet achievementSet = createTestAchievementSet();
  repository->create(achievementSet);

  Achievement achievement = createTestAchievement(101, 1);
  achievement.name = "Test Achievement";
  achievement.description = "Complete a test task";
  achievement.points = 50;
  achievement.type = "progression";
  achievement.flags = 3;
  achievement.displayOrder = 1;
  repository->create(achievement);

  auto result = repository->getAchievement(101);

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->id, 101);
  EXPECT_EQ(result->name, "Test Achievement");
  EXPECT_EQ(result->description, "Complete a test task");
  EXPECT_EQ(result->points, 50);
  EXPECT_EQ(result->type, "progression");
  EXPECT_EQ(result->flags, 3);
  EXPECT_EQ(result->displayOrder, 1);
  EXPECT_EQ(result->setId, 1);
  EXPECT_EQ(result->imageUrl, "https://example.com/achievement.png");
}

TEST_F(SqliteAchievementRepositoryTest, GetAchievement_NonExistentAchievement) {
  auto result = repository->getAchievement(999);

  EXPECT_FALSE(result.has_value());
}

TEST_F(SqliteAchievementRepositoryTest, GetAchievement_VerifyAllFields) {
  AchievementSet achievementSet = createTestAchievementSet();
  repository->create(achievementSet);

  Achievement achievement = createTestAchievement(202, 1);
  achievement.name = "Detailed Achievement";
  achievement.description = "A comprehensive test achievement";
  achievement.imageUrl = "https://example.com/detailed.png";
  achievement.points = 75;
  achievement.type = "challenge";
  achievement.displayOrder = 5;
  achievement.flags = 3;
  repository->create(achievement);

  auto result = repository->getAchievement(202);

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->id, 202);
  EXPECT_EQ(result->name, "Detailed Achievement");
  EXPECT_EQ(result->description, "A comprehensive test achievement");
  EXPECT_EQ(result->imageUrl, "https://example.com/detailed.png");
  EXPECT_EQ(result->points, 75);
  EXPECT_EQ(result->type, "challenge");
  EXPECT_EQ(result->displayOrder, 5);
  EXPECT_EQ(result->setId, 1);
  EXPECT_EQ(result->flags, 3);
}

TEST_F(SqliteAchievementRepositoryTest, GetAchievement_AfterUpdate) {
  AchievementSet achievementSet = createTestAchievementSet();
  repository->create(achievementSet);

  Achievement achievement = createTestAchievement(303, 1);
  achievement.name = "Original Achievement";
  achievement.points = 25;
  repository->create(achievement);

  // Update the achievement
  achievement.name = "Updated Achievement";
  achievement.points = 50;
  achievement.description = "Updated description";
  EXPECT_TRUE(repository->create(achievement)); // Should upsert

  auto result = repository->getAchievement(303);

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->id, 303);
  EXPECT_EQ(result->name, "Updated Achievement");
  EXPECT_EQ(result->points, 50);
  EXPECT_EQ(result->description, "Updated description");
  EXPECT_EQ(result->setId, 1);
}

TEST_F(SqliteAchievementRepositoryTest, GetAchievement_MultipleAchievements) {
  AchievementSet achievementSet = createTestAchievementSet();
  repository->create(achievementSet);

  // Create multiple achievements
  Achievement achievement1 = createTestAchievement(401, 1);
  achievement1.name = "First Achievement";
  achievement1.points = 10;
  repository->create(achievement1);

  Achievement achievement2 = createTestAchievement(402, 1);
  achievement2.name = "Second Achievement";
  achievement2.points = 20;
  repository->create(achievement2);

  Achievement achievement3 = createTestAchievement(403, 1);
  achievement3.name = "Third Achievement";
  achievement3.points = 30;
  repository->create(achievement3);

  // Verify each achievement can be retrieved independently
  auto result1 = repository->getAchievement(401);
  auto result2 = repository->getAchievement(402);
  auto result3 = repository->getAchievement(403);

  ASSERT_TRUE(result1.has_value());
  ASSERT_TRUE(result2.has_value());
  ASSERT_TRUE(result3.has_value());

  EXPECT_EQ(result1->name, "First Achievement");
  EXPECT_EQ(result1->points, 10);

  EXPECT_EQ(result2->name, "Second Achievement");
  EXPECT_EQ(result2->points, 20);

  EXPECT_EQ(result3->name, "Third Achievement");
  EXPECT_EQ(result3->points, 30);
}

TEST_F(SqliteAchievementRepositoryTest, GetAchievement_DifferentFlags) {
  AchievementSet achievementSet = createTestAchievementSet();
  repository->create(achievementSet);

  // Create achievements with different flag values
  Achievement activeAchievement = createTestAchievement(501, 1);
  activeAchievement.name = "Active Achievement";
  activeAchievement.flags = 3; // Active
  repository->create(activeAchievement);

  Achievement inactiveAchievement = createTestAchievement(502, 1);
  inactiveAchievement.name = "Inactive Achievement";
  inactiveAchievement.flags = 5; // Inactive/prototype
  repository->create(inactiveAchievement);

  // Both should be retrievable individually
  auto activeResult = repository->getAchievement(501);
  auto inactiveResult = repository->getAchievement(502);

  ASSERT_TRUE(activeResult.has_value());
  ASSERT_TRUE(inactiveResult.has_value());

  EXPECT_EQ(activeResult->name, "Active Achievement");
  EXPECT_EQ(activeResult->flags, 3);

  EXPECT_EQ(inactiveResult->name, "Inactive Achievement");
  EXPECT_EQ(inactiveResult->flags, 5);
}

TEST_F(SqliteAchievementRepositoryTest, GetAchievement_EmptyStringFields) {
  AchievementSet achievementSet = createTestAchievementSet();
  repository->create(achievementSet);

  Achievement achievement = createTestAchievement(601, 1);
  achievement.name = "Achievement with Empty Fields";
  achievement.description = "";
  achievement.type = "";
  achievement.imageUrl = "";
  repository->create(achievement);

  auto result = repository->getAchievement(601);

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->name, "Achievement with Empty Fields");
  EXPECT_EQ(result->description, "");
  EXPECT_EQ(result->type, "");
  EXPECT_EQ(result->imageUrl, "");
}

TEST_F(SqliteAchievementRepositoryTest, CreateAchievement_Success) {
  // First createOrUpdate the achievement set
  AchievementSet achievementSet = createTestAchievementSet();
  repository->create(achievementSet);

  Achievement achievement = createTestAchievement();

  bool result = repository->create(achievement);

  EXPECT_TRUE(result);
}

TEST_F(SqliteAchievementRepositoryTest, CreateAchievement_UpsertBehavior) {
  AchievementSet achievementSet = createTestAchievementSet();
  repository->create(achievementSet);

  Achievement achievement = createTestAchievement();

  // First creation
  EXPECT_TRUE(repository->create(achievement));

  // Modify and createOrUpdate again (should update)
  achievement.name = "Updated Achievement";
  achievement.points = 75;
  achievement.type = "challenge";
  EXPECT_TRUE(repository->create(achievement));

  // Verify by retrieving the achievement set
  auto retrievedSet = repository->getAchievementSet(achievementSet.id);
  ASSERT_TRUE(retrievedSet.has_value());
  ASSERT_EQ(retrievedSet->achievements.size(), 1);
  EXPECT_EQ(retrievedSet->achievements[0].name, "Updated Achievement");
  EXPECT_EQ(retrievedSet->achievements[0].points, 75);
  EXPECT_EQ(retrievedSet->achievements[0].type, "challenge");
}

TEST_F(SqliteAchievementRepositoryTest, GetAchievementSet_WithAchievements) {
  AchievementSet achievementSet = createTestAchievementSet();
  repository->create(achievementSet);

  // Add multiple achievements with different display orders
  Achievement achievement1 = createTestAchievement(101, 1);
  achievement1.name = "First Achievement";
  achievement1.displayOrder = 2;
  repository->create(achievement1);

  Achievement achievement2 = createTestAchievement(102, 1);
  achievement2.name = "Second Achievement";
  achievement2.displayOrder = 1;
  repository->create(achievement2);

  Achievement achievement3 = createTestAchievement(103, 1);
  achievement3.name = "Third Achievement";
  achievement3.displayOrder = 3;
  repository->create(achievement3);

  auto result = repository->getAchievementSet(achievementSet.id);

  ASSERT_TRUE(result.has_value());
  ASSERT_EQ(result->achievements.size(), 3);

  // Verify achievements are ordered by display_order
  EXPECT_EQ(result->achievements[0].name, "Second Achievement");
  EXPECT_EQ(result->achievements[0].displayOrder, 1);
  EXPECT_EQ(result->achievements[1].name, "First Achievement");
  EXPECT_EQ(result->achievements[1].displayOrder, 2);
  EXPECT_EQ(result->achievements[2].name, "Third Achievement");
  EXPECT_EQ(result->achievements[2].displayOrder, 3);

  // Verify all achievement fields are populated
  const auto &firstAchievement = result->achievements[0];
  EXPECT_EQ(firstAchievement.id, 102);
  EXPECT_EQ(firstAchievement.description, "Complete a test task");
  EXPECT_EQ(firstAchievement.imageUrl, "https://example.com/achievement.png");
  EXPECT_EQ(firstAchievement.points, 50);
  EXPECT_EQ(firstAchievement.type, "progression");
  EXPECT_EQ(firstAchievement.setId, 1);
  EXPECT_EQ(firstAchievement.flags, 3);
}

// Achievement Progress Tests

TEST_F(SqliteAchievementRepositoryTest, CreateAchievementProgress_Success) {
  AchievementSet achievementSet = createTestAchievementSet();
  repository->create(achievementSet);

  Achievement achievement = createTestAchievement();
  repository->create(achievement);

  AchievementProgress progress = createTestProgress();

  bool result = repository->create(progress);

  EXPECT_TRUE(result);
}

TEST_F(SqliteAchievementRepositoryTest,
       CreateAchievementProgress_UpsertBehavior) {
  AchievementSet achievementSet = createTestAchievementSet();
  repository->create(achievementSet);

  Achievement achievement = createTestAchievement();
  repository->create(achievement);

  AchievementProgress progress = createTestProgress();

  // First creation
  EXPECT_TRUE(repository->create(progress));

  // Update progress
  progress.numerator = 8;
  progress.denominator = 10;
  EXPECT_TRUE(repository->create(progress));

  // The upsert should have updated the existing record
  // We can't directly verify this without a getter, but no exception should
  // occur
}

TEST_F(SqliteAchievementRepositoryTest,
       CreateAchievementProgress_MultipleUsers) {
  AchievementSet achievementSet = createTestAchievementSet();
  repository->create(achievementSet);

  Achievement achievement = createTestAchievement();
  repository->create(achievement);

  // Create progress for multiple users on same achievement
  AchievementProgress progress1 = createTestProgress("user1", 100);
  AchievementProgress progress2 = createTestProgress("user2", 100);

  EXPECT_TRUE(repository->create(progress1));
  EXPECT_TRUE(repository->create(progress2));
}

// User Unlock Tests

TEST_F(SqliteAchievementRepositoryTest, CreateUserUnlock_Success) {
  AchievementSet achievementSet = createTestAchievementSet();
  repository->create(achievementSet);

  Achievement achievement = createTestAchievement();
  repository->create(achievement);

  UserUnlock unlock = createTestUserUnlock();

  bool result = repository->createOrUpdate(unlock);

  EXPECT_TRUE(result);
}

TEST_F(SqliteAchievementRepositoryTest, CreateUserUnlock_UpsertBehavior) {
  AchievementSet achievementSet = createTestAchievementSet();
  repository->create(achievementSet);

  Achievement achievement = createTestAchievement();
  repository->create(achievement);

  UserUnlock unlock = createTestUserUnlock();

  // First creation
  EXPECT_TRUE(repository->createOrUpdate(unlock));

  // Modify and createOrUpdate again (should update)
  unlock.earned = true;
  unlock.earnedHardcore = true;
  unlock.unlockTimestamp = 1609545600;         // 2021-01-02 00:00:00 UTC
  unlock.unlockTimestampHardcore = 1609632000; // 2021-01-03 00:00:00 UTC
  unlock.synced = true;
  EXPECT_TRUE(repository->createOrUpdate(unlock));

  // Verify the update took effect by retrieving the unlock
  auto retrieved =
      repository->getUserUnlock(unlock.username, unlock.achievementId);
  ASSERT_TRUE(retrieved.has_value());
  EXPECT_EQ(retrieved->username, unlock.username);
  EXPECT_EQ(retrieved->achievementId, unlock.achievementId);
  EXPECT_TRUE(retrieved->earned);
  EXPECT_TRUE(retrieved->earnedHardcore);
  EXPECT_EQ(retrieved->unlockTimestamp, 1609545600);
  EXPECT_EQ(retrieved->unlockTimestampHardcore, 1609632000);
  EXPECT_TRUE(retrieved->synced);
}

TEST_F(SqliteAchievementRepositoryTest, CreateUserUnlock_MultipleUsers) {
  AchievementSet achievementSet = createTestAchievementSet();
  repository->create(achievementSet);

  Achievement achievement = createTestAchievement();
  repository->create(achievement);

  // Create unlocks for multiple users on same achievement
  UserUnlock unlock1 = createTestUserUnlock("user1", 100);
  UserUnlock unlock2 = createTestUserUnlock("user2", 100);
  unlock2.earnedHardcore = true;
  unlock2.unlockTimestampHardcore = 1609545600;

  EXPECT_TRUE(repository->createOrUpdate(unlock1));
  EXPECT_TRUE(repository->createOrUpdate(unlock2));

  // Verify both unlocks exist independently
  auto retrieved1 = repository->getUserUnlock("user1", 100);
  auto retrieved2 = repository->getUserUnlock("user2", 100);

  ASSERT_TRUE(retrieved1.has_value());
  ASSERT_TRUE(retrieved2.has_value());

  EXPECT_FALSE(retrieved1->earnedHardcore);
  EXPECT_TRUE(retrieved2->earnedHardcore);
}

TEST_F(SqliteAchievementRepositoryTest, CreateUserUnlock_NullTimestamps) {
  AchievementSet achievementSet = createTestAchievementSet();
  repository->create(achievementSet);

  Achievement achievement = createTestAchievement();
  repository->create(achievement);

  UserUnlock unlock = createTestUserUnlock();
  unlock.unlockTimestamp = 0;
  unlock.unlockTimestampHardcore = 0;

  bool result = repository->createOrUpdate(unlock);

  EXPECT_TRUE(result);

  // Verify null timestamps are handled correctly
  auto retrieved =
      repository->getUserUnlock(unlock.username, unlock.achievementId);
  ASSERT_TRUE(retrieved.has_value());
  EXPECT_EQ(retrieved->unlockTimestamp, 0);
  EXPECT_EQ(retrieved->unlockTimestampHardcore, 0);
}

TEST_F(SqliteAchievementRepositoryTest, GetUserUnlock_ExistingUnlock) {
  AchievementSet achievementSet = createTestAchievementSet();
  repository->create(achievementSet);

  Achievement achievement = createTestAchievement();
  repository->create(achievement);

  UserUnlock unlock = createTestUserUnlock();
  repository->createOrUpdate(unlock);

  auto result =
      repository->getUserUnlock(unlock.username, unlock.achievementId);

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->username, unlock.username);
  EXPECT_EQ(result->achievementId, unlock.achievementId);
  EXPECT_EQ(result->earned, unlock.earned);
  EXPECT_EQ(result->earnedHardcore, unlock.earnedHardcore);
  EXPECT_EQ(result->unlockTimestamp, unlock.unlockTimestamp);
  EXPECT_EQ(result->unlockTimestampHardcore, unlock.unlockTimestampHardcore);
  EXPECT_EQ(result->synced, unlock.synced);
}

TEST_F(SqliteAchievementRepositoryTest, GetUserUnlock_NonExistentUnlock) {
  auto result = repository->getUserUnlock("nonexistent", 999);

  EXPECT_FALSE(result.has_value());
}

TEST_F(SqliteAchievementRepositoryTest, GetAllUserUnlocks_EmptySet) {
  auto result = repository->getAllUserUnlocks("testuser", 1);

  EXPECT_TRUE(result.empty());
}

TEST_F(SqliteAchievementRepositoryTest, GetAllUserUnlocks_WithUnlocks) {
  AchievementSet achievementSet = createTestAchievementSet();
  repository->create(achievementSet);

  // Create multiple achievements in the set
  Achievement achievement1 = createTestAchievement(101, 1);
  achievement1.name = "First Achievement";
  achievement1.displayOrder = 1;
  repository->create(achievement1);

  Achievement achievement2 = createTestAchievement(102, 1);
  achievement2.name = "Second Achievement";
  achievement2.displayOrder = 2;
  repository->create(achievement2);

  Achievement achievement3 = createTestAchievement(103, 1);
  achievement3.name = "Third Achievement";
  achievement3.displayOrder = 3;
  repository->create(achievement3);

  // Create unlocks for some achievements
  UserUnlock unlock1 = createTestUserUnlock("testuser", 101);
  unlock1.earned = true;
  unlock1.earnedHardcore = false;
  repository->createOrUpdate(unlock1);

  UserUnlock unlock3 = createTestUserUnlock("testuser", 103);
  unlock3.earned = true;
  unlock3.earnedHardcore = true;
  unlock3.unlockTimestampHardcore = 1609545600;
  repository->createOrUpdate(unlock3);

  auto result = repository->getAllUserUnlocks("testuser", 1);

  ASSERT_EQ(result.size(), 2);

  // Verify they're ordered by display_order (achievement1 first, then
  // achievement3)
  EXPECT_EQ(result[0].achievementId, 101);
  EXPECT_TRUE(result[0].earned);
  EXPECT_FALSE(result[0].earnedHardcore);

  EXPECT_EQ(result[1].achievementId, 103);
  EXPECT_TRUE(result[1].earned);
  EXPECT_TRUE(result[1].earnedHardcore);
  EXPECT_EQ(result[1].unlockTimestampHardcore, 1609545600);
}

TEST_F(SqliteAchievementRepositoryTest, GetAllUserUnlocks_MultipleUsers) {
  AchievementSet achievementSet = createTestAchievementSet();
  repository->create(achievementSet);

  Achievement achievement = createTestAchievement(101, 1);
  repository->create(achievement);

  // Create unlocks for different users
  UserUnlock unlock1 = createTestUserUnlock("user1", 101);
  UserUnlock unlock2 = createTestUserUnlock("user2", 101);

  repository->createOrUpdate(unlock1);
  repository->createOrUpdate(unlock2);

  // Each user should only see their own unlocks
  auto result1 = repository->getAllUserUnlocks("user1", 1);
  auto result2 = repository->getAllUserUnlocks("user2", 1);

  ASSERT_EQ(result1.size(), 1);
  ASSERT_EQ(result2.size(), 1);

  EXPECT_EQ(result1[0].username, "user1");
  EXPECT_EQ(result2[0].username, "user2");
}

TEST_F(SqliteAchievementRepositoryTest, GetAllUserUnlocks_FiltersBySet) {
  // Create two achievement sets
  AchievementSet achievementSet1 = createTestAchievementSet();
  achievementSet1.id = 1;
  repository->create(achievementSet1);

  AchievementSet achievementSet2 = createTestAchievementSet();
  achievementSet2.id = 2;
  achievementSet2.name = "Second Set";
  repository->create(achievementSet2);

  // Create achievements in different sets
  Achievement achievement1 = createTestAchievement(101, 1);
  repository->create(achievement1);

  Achievement achievement2 = createTestAchievement(102, 2);
  repository->create(achievement2);

  // Create unlocks for achievements in both sets
  UserUnlock unlock1 = createTestUserUnlock("testuser", 101);
  UserUnlock unlock2 = createTestUserUnlock("testuser", 102);

  repository->createOrUpdate(unlock1);
  repository->createOrUpdate(unlock2);

  // Should only return unlocks for the specified set
  auto result1 = repository->getAllUserUnlocks("testuser", 1);
  auto result2 = repository->getAllUserUnlocks("testuser", 2);

  ASSERT_EQ(result1.size(), 1);
  ASSERT_EQ(result2.size(), 1);

  EXPECT_EQ(result1[0].achievementId, 101);
  EXPECT_EQ(result2[0].achievementId, 102);
}

TEST_F(SqliteAchievementRepositoryTest, GetAllUserUnlocks_NonExistentUser) {
  AchievementSet achievementSet = createTestAchievementSet();
  repository->create(achievementSet);

  Achievement achievement = createTestAchievement();
  repository->create(achievement);

  auto result = repository->getAllUserUnlocks("nonexistent", 1);

  EXPECT_TRUE(result.empty());
}

// Hash Operations Tests

TEST_F(SqliteAchievementRepositoryTest, SetGameHash_Success) {
  AchievementSet achievementSet = createTestAchievementSet();
  repository->create(achievementSet);

  const std::string contentHash = "abc123def456";

  bool result = repository->setGameId(contentHash, achievementSet.id);

  EXPECT_TRUE(result);
}

TEST_F(SqliteAchievementRepositoryTest, SetGameHash_UpsertBehavior) {
  AchievementSet achievementSet1 = createTestAchievementSet();
  achievementSet1.id = 1;
  repository->create(achievementSet1);

  AchievementSet achievementSet2 = createTestAchievementSet();
  achievementSet2.id = 2;
  achievementSet2.name = "Second Achievement Set";
  repository->create(achievementSet2);

  const std::string contentHash = "abc123def456";

  // Initially map hash to first set
  EXPECT_TRUE(repository->setGameId(contentHash, achievementSet1.id));

  // Update mapping to second set (upsert behavior)
  EXPECT_TRUE(repository->setGameId(contentHash, achievementSet2.id));

  // Verify the hash now points to the second set
  auto result = repository->getAchievementSetByContentHash(contentHash);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->id, achievementSet2.id);
  EXPECT_EQ(result->name, "Second Achievement Set");
}

TEST_F(SqliteAchievementRepositoryTest,
       GetAchievementSetByContentHash_ExistingHash) {
  AchievementSet achievementSet = createTestAchievementSet();
  repository->create(achievementSet);

  // Add some achievements to verify they're loaded
  Achievement achievement = createTestAchievement();
  repository->create(achievement);

  const std::string contentHash = "abc123def456";
  repository->setGameId(contentHash, achievementSet.id);

  auto result = repository->getAchievementSetByContentHash(contentHash);

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->id, achievementSet.id);
  EXPECT_EQ(result->name, achievementSet.name);
  EXPECT_EQ(result->numAchievements, achievementSet.numAchievements);
  EXPECT_EQ(result->achievements.size(),
            1); // Only achievements with flags == 3 are included
  EXPECT_EQ(result->totalPoints,
            50); // Calculated from achievements with flags == 3
}

TEST_F(SqliteAchievementRepositoryTest,
       GetAchievementSetByContentHash_NonExistentHash) {
  auto result = repository->getAchievementSetByContentHash("nonexistent");

  EXPECT_FALSE(result.has_value());
}

TEST_F(SqliteAchievementRepositoryTest,
       GetAchievementSetByContentHash_FiltersByFlags) {
  AchievementSet achievementSet = createTestAchievementSet();
  repository->create(achievementSet);

  // Add achievements with different flags
  Achievement activeAchievement = createTestAchievement(101, 1);
  activeAchievement.flags = 3; // Active
  activeAchievement.points = 50;
  repository->create(activeAchievement);

  Achievement inactiveAchievement = createTestAchievement(102, 1);
  inactiveAchievement.flags = 5; // Inactive/prototype
  inactiveAchievement.points = 25;
  repository->create(inactiveAchievement);

  const std::string contentHash = "abc123def456";
  repository->setGameId(contentHash, achievementSet.id);

  auto result = repository->getAchievementSetByContentHash(contentHash);

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->achievements.size(), 1); // Only the active achievement
  EXPECT_EQ(result->achievements[0].id, 101);
  EXPECT_EQ(result->totalPoints, 50); // Only points from active achievement
}

TEST_F(SqliteAchievementRepositoryTest, GetGameId_ExistingMapping) {
  AchievementSet achievementSet = createTestAchievementSet();
  repository->create(achievementSet);

  const std::string contentHash = "test_hash_123";

  // First set the mapping
  EXPECT_TRUE(repository->setGameId(contentHash, achievementSet.id));

  // Then retrieve it
  auto result = repository->getGameId(contentHash);

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result.value(), achievementSet.id);
}

TEST_F(SqliteAchievementRepositoryTest, GetGameId_NonExistentMapping) {
  auto result = repository->getGameId("nonexistent_hash");

  EXPECT_FALSE(result.has_value());
}

TEST_F(SqliteAchievementRepositoryTest, GetGameId_UpdateMapping) {
  AchievementSet achievementSet1 = createTestAchievementSet();
  achievementSet1.id = 1;
  repository->create(achievementSet1);

  AchievementSet achievementSet2 = createTestAchievementSet();
  achievementSet2.id = 2;
  repository->create(achievementSet2);

  const std::string contentHash = "test_hash_456";

  // Set initial mapping
  EXPECT_TRUE(repository->setGameId(contentHash, achievementSet1.id));

  auto firstResult = repository->getGameId(contentHash);
  ASSERT_TRUE(firstResult.has_value());
  EXPECT_EQ(firstResult.value(), achievementSet1.id);

  // Update the mapping
  EXPECT_TRUE(repository->setGameId(contentHash, achievementSet2.id));

  auto secondResult = repository->getGameId(contentHash);
  ASSERT_TRUE(secondResult.has_value());
  EXPECT_EQ(secondResult.value(), achievementSet2.id);
}

TEST_F(SqliteAchievementRepositoryTest, GetGameId_MultipleMappings) {
  AchievementSet achievementSet1 = createTestAchievementSet();
  achievementSet1.id = 1;
  repository->create(achievementSet1);

  AchievementSet achievementSet2 = createTestAchievementSet();
  achievementSet2.id = 2;
  repository->create(achievementSet2);

  const std::string hash1 = "hash_1";
  const std::string hash2 = "hash_2";

  // Set multiple mappings
  EXPECT_TRUE(repository->setGameId(hash1, achievementSet1.id));
  EXPECT_TRUE(repository->setGameId(hash2, achievementSet2.id));

  // Verify each mapping independently
  auto result1 = repository->getGameId(hash1);
  auto result2 = repository->getGameId(hash2);

  ASSERT_TRUE(result1.has_value());
  ASSERT_TRUE(result2.has_value());
  EXPECT_EQ(result1.value(), achievementSet1.id);
  EXPECT_EQ(result2.value(), achievementSet2.id);
}

TEST_F(SqliteAchievementRepositoryTest, GetGameId_EmptyContentHash) {
  AchievementSet achievementSet = createTestAchievementSet();
  repository->create(achievementSet);

  const std::string emptyHash = "";

  // Set mapping with empty hash
  EXPECT_TRUE(repository->setGameId(emptyHash, achievementSet.id));

  // Verify we can retrieve it
  auto result = repository->getGameId(emptyHash);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result.value(), achievementSet.id);
}

TEST_F(SqliteAchievementRepositoryTest, GetGameId_IntegrationWithSetAndGet) {
  AchievementSet achievementSet = createTestAchievementSet();
  repository->create(achievementSet);

  Achievement achievement = createTestAchievement();
  repository->create(achievement);

  const std::string contentHash = "integration_test_hash";

  // Set the game ID mapping
  EXPECT_TRUE(repository->setGameId(contentHash, achievementSet.id));

  // Verify we can get the game ID back
  auto retrievedGameId = repository->getGameId(contentHash);
  ASSERT_TRUE(retrievedGameId.has_value());
  EXPECT_EQ(retrievedGameId.value(), achievementSet.id);

  // Verify we can also get the achievement set by content hash
  auto achievementSetByHash =
      repository->getAchievementSetByContentHash(contentHash);
  ASSERT_TRUE(achievementSetByHash.has_value());
  EXPECT_EQ(achievementSetByHash->id, achievementSet.id);
}

TEST_F(SqliteAchievementRepositoryTest, GetGameId_FiltersOutZeroGameId) {
  const std::string contentHash = "hash_with_zero_game_id";

  // Set a mapping with game ID 0
  EXPECT_TRUE(repository->setGameId(contentHash, 0));

  // getGameId should return nullopt because achievement_set_id is 0
  auto result = repository->getGameId(contentHash);

  EXPECT_FALSE(result.has_value());
}

TEST_F(SqliteAchievementRepositoryTest, GetGameId_ReturnsNonZeroGameId) {
  AchievementSet achievementSet = createTestAchievementSet();
  achievementSet.id = 42;
  repository->create(achievementSet);

  const std::string contentHash = "hash_with_nonzero_game_id";

  // Set a mapping with non-zero game ID
  EXPECT_TRUE(repository->setGameId(contentHash, achievementSet.id));

  // getGameId should return the ID since it's not 0
  auto result = repository->getGameId(contentHash);

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result.value(), 42);
}

TEST_F(SqliteAchievementRepositoryTest, GetGameId_UpdateFromZeroToNonZero) {
  AchievementSet achievementSet = createTestAchievementSet();
  achievementSet.id = 100;
  repository->create(achievementSet);

  const std::string contentHash = "hash_to_update";

  // First set mapping to 0
  EXPECT_TRUE(repository->setGameId(contentHash, 0));

  // Should return nullopt for 0
  auto zeroResult = repository->getGameId(contentHash);
  EXPECT_FALSE(zeroResult.has_value());

  // Update mapping to non-zero
  EXPECT_TRUE(repository->setGameId(contentHash, achievementSet.id));

  // Should now return the non-zero value
  auto nonZeroResult = repository->getGameId(contentHash);
  ASSERT_TRUE(nonZeroResult.has_value());
  EXPECT_EQ(nonZeroResult.value(), 100);
}

TEST_F(SqliteAchievementRepositoryTest, GetGameId_UpdateFromNonZeroToZero) {
  AchievementSet achievementSet = createTestAchievementSet();
  achievementSet.id = 50;
  repository->create(achievementSet);

  const std::string contentHash = "hash_to_zero";

  // First set mapping to non-zero
  EXPECT_TRUE(repository->setGameId(contentHash, achievementSet.id));

  // Should return the non-zero value
  auto nonZeroResult = repository->getGameId(contentHash);
  ASSERT_TRUE(nonZeroResult.has_value());
  EXPECT_EQ(nonZeroResult.value(), 50);

  // Update mapping to 0
  EXPECT_TRUE(repository->setGameId(contentHash, 0));

  // Should now return nullopt for 0
  auto zeroResult = repository->getGameId(contentHash);
  EXPECT_FALSE(zeroResult.has_value());
}

// GetGameHash Tests

TEST_F(SqliteAchievementRepositoryTest, GetGameHash_ExistingMapping) {
  AchievementSet achievementSet = createTestAchievementSet();
  repository->create(achievementSet);

  const std::string contentHash = "test_hash_abc123";

  // First set the mapping
  EXPECT_TRUE(repository->setGameId(contentHash, achievementSet.id));

  // Then retrieve the hash by set ID
  auto result = repository->getGameHash(achievementSet.id);

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result.value(), contentHash);
}

TEST_F(SqliteAchievementRepositoryTest, GetGameHash_NonExistentMapping) {
  auto result = repository->getGameHash(999);

  EXPECT_FALSE(result.has_value());
}

TEST_F(SqliteAchievementRepositoryTest, GetGameHash_MultipleMappings) {
  AchievementSet achievementSet1 = createTestAchievementSet();
  achievementSet1.id = 1;
  repository->create(achievementSet1);

  AchievementSet achievementSet2 = createTestAchievementSet();
  achievementSet2.id = 2;
  repository->create(achievementSet2);

  const std::string hash1 = "hash_for_set_1";
  const std::string hash2 = "hash_for_set_2";

  // Set multiple mappings
  EXPECT_TRUE(repository->setGameId(hash1, achievementSet1.id));
  EXPECT_TRUE(repository->setGameId(hash2, achievementSet2.id));

  // Verify each mapping independently
  auto result1 = repository->getGameHash(achievementSet1.id);
  auto result2 = repository->getGameHash(achievementSet2.id);

  ASSERT_TRUE(result1.has_value());
  ASSERT_TRUE(result2.has_value());
  EXPECT_EQ(result1.value(), hash1);
  EXPECT_EQ(result2.value(), hash2);
}

TEST_F(SqliteAchievementRepositoryTest, GetGameHash_EmptyHash) {
  AchievementSet achievementSet = createTestAchievementSet();
  repository->create(achievementSet);

  const std::string emptyHash = "";

  // Set mapping with empty hash
  EXPECT_TRUE(repository->setGameId(emptyHash, achievementSet.id));

  auto result = repository->getGameHash(achievementSet.id);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result.value(), emptyHash);
}

TEST_F(SqliteAchievementRepositoryTest, GetGameHash_LongHash) {
  AchievementSet achievementSet = createTestAchievementSet();
  repository->create(achievementSet);

  // Create a long hash string to test handling of larger strings
  const std::string longHash =
      "abcdef1234567890" + std::string(100, 'x') + "fedcba0987654321";

  EXPECT_TRUE(repository->setGameId(longHash, achievementSet.id));

  auto result = repository->getGameHash(achievementSet.id);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result.value(), longHash);
}

TEST_F(SqliteAchievementRepositoryTest, GetGameHash_SpecialCharacters) {
  AchievementSet achievementSet = createTestAchievementSet();
  repository->create(achievementSet);

  // Test hash with special characters
  const std::string specialHash = "hash-with_special.chars+symbols@2024!";

  EXPECT_TRUE(repository->setGameId(specialHash, achievementSet.id));

  auto result = repository->getGameHash(achievementSet.id);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result.value(), specialHash);
}

TEST_F(SqliteAchievementRepositoryTest, GetGameHash_RoundTripConsistency) {
  AchievementSet achievementSet = createTestAchievementSet();
  repository->create(achievementSet);

  const std::string originalHash = "round_trip_test_hash_789";

  // Set the hash->ID mapping
  EXPECT_TRUE(repository->setGameId(originalHash, achievementSet.id));

  // Get ID from hash
  auto retrievedId = repository->getGameId(originalHash);
  ASSERT_TRUE(retrievedId.has_value());
  EXPECT_EQ(retrievedId.value(), achievementSet.id);

  // Get hash from ID
  auto retrievedHash = repository->getGameHash(achievementSet.id);
  ASSERT_TRUE(retrievedHash.has_value());
  EXPECT_EQ(retrievedHash.value(), originalHash);

  // Verify round-trip consistency
  EXPECT_EQ(retrievedHash.value(), originalHash);
}

TEST_F(SqliteAchievementRepositoryTest, GetGameHash_AfterSetDeletion) {
  AchievementSet achievementSet = createTestAchievementSet();
  repository->create(achievementSet);

  const std::string contentHash = "hash_for_deleted_set";

  // Set mapping
  EXPECT_TRUE(repository->setGameId(contentHash, achievementSet.id));

  // Verify mapping exists
  auto beforeResult = repository->getGameHash(achievementSet.id);
  ASSERT_TRUE(beforeResult.has_value());
  EXPECT_EQ(beforeResult.value(), contentHash);

  // Note: We don't actually delete the achievement set in this test since
  // there's no delete method, but this tests the case where we query
  // for a non-existent set ID
  auto nonExistentResult = repository->getGameHash(999);
  EXPECT_FALSE(nonExistentResult.has_value());
}

TEST_F(SqliteAchievementRepositoryTest, GetGameHash_IntegrationWithGetGameId) {
  AchievementSet achievementSet = createTestAchievementSet();
  repository->create(achievementSet);

  const std::string contentHash = "integration_test_hash_bidirectional";

  // Set the hash->ID mapping using setGameId
  EXPECT_TRUE(repository->setGameId(contentHash, achievementSet.id));

  // Test both directions of the mapping
  auto retrievedId = repository->getGameId(contentHash);
  auto retrievedHash = repository->getGameHash(achievementSet.id);

  ASSERT_TRUE(retrievedId.has_value());
  ASSERT_TRUE(retrievedHash.has_value());

  EXPECT_EQ(retrievedId.value(), achievementSet.id);
  EXPECT_EQ(retrievedHash.value(), contentHash);

  // Verify bidirectional consistency
  auto doubleCheckId = repository->getGameId(retrievedHash.value());
  auto doubleCheckHash = repository->getGameHash(retrievedId.value());

  ASSERT_TRUE(doubleCheckId.has_value());
  ASSERT_TRUE(doubleCheckHash.has_value());

  EXPECT_EQ(doubleCheckId.value(), achievementSet.id);
  EXPECT_EQ(doubleCheckHash.value(), contentHash);
}

// Integration Tests

TEST_F(SqliteAchievementRepositoryTest, CompleteWorkflow_CreateRetrieveUpdate) {
  // Create achievement set
  AchievementSet achievementSet = createTestAchievementSet();
  EXPECT_TRUE(repository->create(achievementSet));

  // Add achievements
  Achievement achievement1 = createTestAchievement(101, 1);
  achievement1.name = "First Achievement";
  achievement1.displayOrder = 1;
  EXPECT_TRUE(repository->create(achievement1));

  Achievement achievement2 = createTestAchievement(102, 1);
  achievement2.name = "Second Achievement";
  achievement2.displayOrder = 2;
  EXPECT_TRUE(repository->create(achievement2));

  // Set hash mapping
  const std::string contentHash = "integration_test_hash";
  EXPECT_TRUE(repository->setGameId(contentHash, achievementSet.id));

  // Add progress
  AchievementProgress progress1 = createTestProgress("user1", 101);
  AchievementProgress progress2 = createTestProgress("user1", 102);
  EXPECT_TRUE(repository->create(progress1));
  EXPECT_TRUE(repository->create(progress2));

  // Retrieve by ID
  auto retrievedById = repository->getAchievementSet(achievementSet.id);
  ASSERT_TRUE(retrievedById.has_value());
  EXPECT_EQ(retrievedById->achievements.size(), 2);

  // Retrieve by hash
  auto retrievedByHash =
      repository->getAchievementSetByContentHash(contentHash);
  ASSERT_TRUE(retrievedByHash.has_value());
  EXPECT_EQ(retrievedByHash->id, achievementSet.id);
  EXPECT_EQ(retrievedByHash->achievements.size(), 2);

  // Update achievement set
  achievementSet.name = "Updated Achievement Set";
  EXPECT_TRUE(repository->update(achievementSet));

  auto updatedSet = repository->getAchievementSet(achievementSet.id);
  ASSERT_TRUE(updatedSet.has_value());
  EXPECT_EQ(updatedSet->name, "Updated Achievement Set");
}

// GetAllUnsyncedUserUnlocks Tests

TEST_F(SqliteAchievementRepositoryTest, GetAllUnsyncedUserUnlocks_NoUnlocks) {
  auto result = repository->getAllUnsyncedUserUnlocks("nonexistent_user");

  EXPECT_TRUE(result.empty());
}

TEST_F(SqliteAchievementRepositoryTest,
       GetAllUnsyncedUserUnlocks_OnlyUnsyncedUnlocks) {
  // Setup test data
  AchievementSet achievementSet = createTestAchievementSet();
  repository->create(achievementSet);

  Achievement achievement1 = createTestAchievement(101, 1);
  Achievement achievement2 = createTestAchievement(102, 1);
  Achievement achievement3 = createTestAchievement(103, 1);
  repository->create(achievement1);
  repository->create(achievement2);
  repository->create(achievement3);

  // Create unsynced unlocks
  UserUnlock unlock1 = createTestUserUnlock("testuser", 101);
  unlock1.synced = false;
  unlock1.earned = true;
  unlock1.earnedHardcore = false;
  unlock1.unlockTimestamp = 1609459200;

  UserUnlock unlock2 = createTestUserUnlock("testuser", 102);
  unlock2.synced = false;
  unlock2.earned = false;
  unlock2.earnedHardcore = true;
  unlock2.unlockTimestampHardcore = 1609545600;

  UserUnlock unlock3 = createTestUserUnlock("testuser", 103);
  unlock3.synced = false;
  unlock3.earned = true;
  unlock3.earnedHardcore = true;
  unlock3.unlockTimestamp = 1609632000;
  unlock3.unlockTimestampHardcore = 1609718400;

  repository->createOrUpdate(unlock1);
  repository->createOrUpdate(unlock2);
  repository->createOrUpdate(unlock3);

  auto result = repository->getAllUnsyncedUserUnlocks("testuser");

  ASSERT_EQ(result.size(), 3);

  // Verify all unlocks are returned and are unsynced
  for (const auto &unlock : result) {
    EXPECT_EQ(unlock.username, "testuser");
    EXPECT_FALSE(unlock.synced);
  }

  // Verify specific unlock data
  auto unlock1_result =
      std::find_if(result.begin(), result.end(),
                   [](const UserUnlock &u) { return u.achievementId == 101; });
  ASSERT_NE(unlock1_result, result.end());
  EXPECT_TRUE(unlock1_result->earned);
  EXPECT_FALSE(unlock1_result->earnedHardcore);
  EXPECT_EQ(unlock1_result->unlockTimestamp, 1609459200);

  auto unlock2_result =
      std::find_if(result.begin(), result.end(),
                   [](const UserUnlock &u) { return u.achievementId == 102; });
  ASSERT_NE(unlock2_result, result.end());
  EXPECT_FALSE(unlock2_result->earned);
  EXPECT_TRUE(unlock2_result->earnedHardcore);
  EXPECT_EQ(unlock2_result->unlockTimestampHardcore, 1609545600);
}

TEST_F(SqliteAchievementRepositoryTest,
       GetAllUnsyncedUserUnlocks_MixedSyncStatus) {
  // Setup test data
  AchievementSet achievementSet = createTestAchievementSet();
  repository->create(achievementSet);

  Achievement achievement1 = createTestAchievement(201, 1);
  Achievement achievement2 = createTestAchievement(202, 1);
  Achievement achievement3 = createTestAchievement(203, 1);
  repository->create(achievement1);
  repository->create(achievement2);
  repository->create(achievement3);

  // Create unlocks with mixed sync status
  UserUnlock unsyncedUnlock = createTestUserUnlock("testuser", 201);
  unsyncedUnlock.synced = false;
  unsyncedUnlock.earned = true;

  UserUnlock syncedUnlock = createTestUserUnlock("testuser", 202);
  syncedUnlock.synced = true;
  syncedUnlock.earned = true;

  UserUnlock anotherUnsyncedUnlock = createTestUserUnlock("testuser", 203);
  anotherUnsyncedUnlock.synced = false;
  anotherUnsyncedUnlock.earnedHardcore = true;

  repository->createOrUpdate(unsyncedUnlock);
  repository->createOrUpdate(syncedUnlock);
  repository->createOrUpdate(anotherUnsyncedUnlock);

  auto result = repository->getAllUnsyncedUserUnlocks("testuser");

  // Should only return the unsynced unlocks (2 out of 3)
  ASSERT_EQ(result.size(), 2);

  // Verify only unsynced unlocks are returned
  for (const auto &unlock : result) {
    EXPECT_EQ(unlock.username, "testuser");
    EXPECT_FALSE(unlock.synced);
    EXPECT_TRUE(unlock.achievementId == 201 || unlock.achievementId == 203);
  }
}

TEST_F(SqliteAchievementRepositoryTest, GetAllUnsyncedUserUnlocks_AllSynced) {
  // Setup test data
  AchievementSet achievementSet = createTestAchievementSet();
  repository->create(achievementSet);

  Achievement achievement1 = createTestAchievement(301, 1);
  Achievement achievement2 = createTestAchievement(302, 1);
  repository->create(achievement1);
  repository->create(achievement2);

  // Create only synced unlocks
  UserUnlock syncedUnlock1 = createTestUserUnlock("testuser", 301);
  syncedUnlock1.synced = true;

  UserUnlock syncedUnlock2 = createTestUserUnlock("testuser", 302);
  syncedUnlock2.synced = true;

  repository->createOrUpdate(syncedUnlock1);
  repository->createOrUpdate(syncedUnlock2);

  auto result = repository->getAllUnsyncedUserUnlocks("testuser");

  // Should return empty vector since all unlocks are synced
  EXPECT_TRUE(result.empty());
}

TEST_F(SqliteAchievementRepositoryTest,
       GetAllUnsyncedUserUnlocks_MultipleUsers) {
  // Setup test data
  AchievementSet achievementSet = createTestAchievementSet();
  repository->create(achievementSet);

  Achievement achievement1 = createTestAchievement(401, 1);
  Achievement achievement2 = createTestAchievement(402, 1);
  repository->create(achievement1);
  repository->create(achievement2);

  // Create unlocks for multiple users
  UserUnlock user1Unlock = createTestUserUnlock("user1", 401);
  user1Unlock.synced = false;

  UserUnlock user2Unlock = createTestUserUnlock("user2", 401);
  user2Unlock.synced = false;

  UserUnlock user1Unlock2 = createTestUserUnlock("user1", 402);
  user1Unlock2.synced = true; // This one is synced

  repository->createOrUpdate(user1Unlock);
  repository->createOrUpdate(user2Unlock);
  repository->createOrUpdate(user1Unlock2);

  // Query for user1 - should only get unsynced unlocks for that user
  auto user1Result = repository->getAllUnsyncedUserUnlocks("user1");
  ASSERT_EQ(user1Result.size(), 1);
  EXPECT_EQ(user1Result[0].username, "user1");
  EXPECT_EQ(user1Result[0].achievementId, 401);
  EXPECT_FALSE(user1Result[0].synced);

  // Query for user2 - should only get unsynced unlocks for that user
  auto user2Result = repository->getAllUnsyncedUserUnlocks("user2");
  ASSERT_EQ(user2Result.size(), 1);
  EXPECT_EQ(user2Result[0].username, "user2");
  EXPECT_EQ(user2Result[0].achievementId, 401);
  EXPECT_FALSE(user2Result[0].synced);
}

TEST_F(SqliteAchievementRepositoryTest,
       GetAllUnsyncedUserUnlocks_ZeroTimestamps) {
  // Setup test data
  AchievementSet achievementSet = createTestAchievementSet();
  repository->create(achievementSet);

  Achievement achievement = createTestAchievement(501, 1);
  repository->create(achievement);

  // Create unlock with zero timestamps
  UserUnlock unlock = createTestUserUnlock("testuser", 501);
  unlock.synced = false;
  unlock.unlockTimestamp = 0;
  unlock.unlockTimestampHardcore = 0;

  repository->createOrUpdate(unlock);

  auto result = repository->getAllUnsyncedUserUnlocks("testuser");

  ASSERT_EQ(result.size(), 1);
  EXPECT_EQ(result[0].username, "testuser");
  EXPECT_EQ(result[0].achievementId, 501);
  EXPECT_FALSE(result[0].synced);
  EXPECT_EQ(result[0].unlockTimestamp, 0);
  EXPECT_EQ(result[0].unlockTimestampHardcore, 0);
}

} // namespace firelight::achievements
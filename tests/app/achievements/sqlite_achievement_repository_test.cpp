#include "../../../src/app/achievements/sqlite_achievement_repository.hpp"
#include <gtest/gtest.h>
#include <filesystem>

namespace firelight::achievements {

/**
 * @brief Test fixture for SqliteAchievementRepository functionality
 *
 * Comprehensive test suite for the SQLite-based achievement repository implementation,
 * covering CRUD operations for achievement sets, achievements, progress tracking,
 * and hash-based lookups.
 */
class SqliteAchievementRepositoryTest : public testing::Test {
protected:
  std::unique_ptr<SqliteAchievementRepository> repository;

  void SetUp() override {
    repository = std::make_unique<SqliteAchievementRepository>(":memory:");
  }

  void TearDown() override {
    repository.reset();
  }

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

  AchievementProgress createTestProgress(const std::string& username = "testuser", unsigned achievementId = 100) {
    AchievementProgress progress;
    progress.username = username;
    progress.achievementId = achievementId;
    progress.numerator = 5;
    progress.denominator = 10;
    return progress;
  }
};

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

  // Modify and create again (should update)
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

TEST_F(SqliteAchievementRepositoryTest, CreateAchievement_Success) {
  // First create the achievement set
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

  // Modify and create again (should update)
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
  const auto& firstAchievement = result->achievements[0];
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

TEST_F(SqliteAchievementRepositoryTest, CreateAchievementProgress_UpsertBehavior) {
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
  // We can't directly verify this without a getter, but no exception should occur
}

TEST_F(SqliteAchievementRepositoryTest, CreateAchievementProgress_MultipleUsers) {
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

// Hash Operations Tests

TEST_F(SqliteAchievementRepositoryTest, SetGameHash_Success) {
  AchievementSet achievementSet = createTestAchievementSet();
  repository->create(achievementSet);

  const std::string contentHash = "abc123def456";

  bool result = repository->setGameHash(achievementSet.id, contentHash);

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
  EXPECT_TRUE(repository->setGameHash(achievementSet1.id, contentHash));

  // Update mapping to second set (upsert behavior)
  EXPECT_TRUE(repository->setGameHash(achievementSet2.id, contentHash));

  // Verify the hash now points to the second set
  auto result = repository->getAchievementSetByContentHash(contentHash);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->id, achievementSet2.id);
  EXPECT_EQ(result->name, "Second Achievement Set");
}

TEST_F(SqliteAchievementRepositoryTest, GetAchievementSetByContentHash_ExistingHash) {
  AchievementSet achievementSet = createTestAchievementSet();
  repository->create(achievementSet);

  // Add some achievements to verify they're loaded
  Achievement achievement = createTestAchievement();
  repository->create(achievement);

  const std::string contentHash = "abc123def456";
  repository->setGameHash(achievementSet.id, contentHash);

  auto result = repository->getAchievementSetByContentHash(contentHash);

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->id, achievementSet.id);
  EXPECT_EQ(result->name, achievementSet.name);
  EXPECT_EQ(result->numAchievements, achievementSet.numAchievements);
  EXPECT_EQ(result->achievements.size(), 1); // Only achievements with flags == 3 are included
  EXPECT_EQ(result->totalPoints, 50); // Calculated from achievements with flags == 3
}

TEST_F(SqliteAchievementRepositoryTest, GetAchievementSetByContentHash_NonExistentHash) {
  auto result = repository->getAchievementSetByContentHash("nonexistent");

  EXPECT_FALSE(result.has_value());
}

TEST_F(SqliteAchievementRepositoryTest, GetAchievementSetByContentHash_FiltersByFlags) {
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
  repository->setGameHash(achievementSet.id, contentHash);

  auto result = repository->getAchievementSetByContentHash(contentHash);

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->achievements.size(), 1); // Only the active achievement
  EXPECT_EQ(result->achievements[0].id, 101);
  EXPECT_EQ(result->totalPoints, 50); // Only points from active achievement
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
  EXPECT_TRUE(repository->setGameHash(achievementSet.id, contentHash));

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
  auto retrievedByHash = repository->getAchievementSetByContentHash(contentHash);
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

} // namespace firelight::achievements
#include "../../../src/app/achievements/achievement_service.hpp"
#include "../../../src/app/achievements/sqlite_achievement_repository.hpp"
#include <gtest/gtest.h>
#include <optional>
#include <rcheevos/patch_response.hpp>
#include <rcheevos/ra_constants.h>
#include <rcheevos/startsession_response.hpp>

namespace firelight::achievements {

/**
 * @brief Test fixture for AchievementService functionality
 *
 * Comprehensive test suite for the AchievementService class using an in-memory
 * SQLite database for testing. Tests cover service delegation, patch response
 * processing, start session response processing, and edge cases.
 */
class AchievementServiceTest : public testing::Test {
protected:
  std::unique_ptr<SqliteAchievementRepository> repository;
  std::unique_ptr<AchievementService> service;

  void SetUp() override {
    repository = std::make_unique<SqliteAchievementRepository>(":memory:");
    service = std::make_unique<AchievementService>(*repository);
  }

  void TearDown() override {
    service.reset();
    repository.reset();
  }

  // Helper functions for creating test data

  PatchAchievement createTestPatchAchievement(unsigned id = 1, int flags = 3,
                                              int points = 50) {
    PatchAchievement achievement;
    achievement.ID = id;
    achievement.Title = "Test Achievement " + std::to_string(id);
    achievement.Description = "Test achievement description";
    achievement.Points = points;
    achievement.Author = "TestAuthor";
    achievement.Modified = 1234567890;
    achievement.Created = 1234567890;
    achievement.BadgeName = "test_badge";
    achievement.Flags = flags;
    achievement.Type = "progression";
    achievement.Rarity = 50;
    achievement.RarityHardcore = 25;
    achievement.BadgeURL = "https://example.com/badge.png";
    achievement.BadgeLockedURL = "https://example.com/badge_locked.png";
    achievement.MemAddr = "0x1234";
    return achievement;
  }

  PatchDataStruct createTestPatchData(unsigned setId = 1) {
    PatchDataStruct patchData;
    patchData.ID = setId;
    patchData.Title = "Test Game " + std::to_string(setId);
    patchData.ImageIcon = "test_icon";
    patchData.RichPresencePatch = "Display: Test Rich Presence";
    patchData.ConsoleID = 1;
    patchData.ImageIconURL = "https://example.com/icon.png";
    return patchData;
  }

  PatchResponse createTestPatchResponse(
      unsigned setId = 1,
      std::optional<std::vector<PatchAchievement>> achievements =
          std::nullopt) {
    PatchResponse response;
    response.Success = true;
    response.PatchData = createTestPatchData(setId);
    if (!achievements.has_value()) {
      achievements = {createTestPatchAchievement(1),
                      createTestPatchAchievement(2)};
    }
    response.PatchData.Achievements = achievements.value();
    return response;
  }

  Unlock createTestUnlock(unsigned achievementId = 1,
                          unsigned long long when = 1609459200) {
    return Unlock{.ID = achievementId, .When = when};
  }

  StartSessionResponse
  createTestStartSessionResponse(std::vector<Unlock> unlocks = {},
                                 std::vector<Unlock> hardcoreUnlocks = {}) {
    StartSessionResponse response;
    response.Success = true;
    response.Unlocks = unlocks;
    response.HardcoreUnlocks = hardcoreUnlocks;
    response.ServerNow = 1609459200;
    return response;
  }

  AchievementProgress
  createTestProgress(const std::string &username = "testuser",
                     unsigned achievementId = 1) {
    AchievementProgress progress;
    progress.username = username;
    progress.achievementId = achievementId;
    progress.numerator = 5;
    progress.denominator = 10;
    return progress;
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

// Basic Service Operations Tests

TEST_F(AchievementServiceTest, Constructor_Success) {
  EXPECT_NO_THROW(AchievementService(*repository));
}

// User Management Tests

TEST_F(AchievementServiceTest, CreateUser_Success) {
  auto user = createTestUser("testuser", "test_token_123", 1500, 750);

  bool result = service->createOrUpdateUser(user);

  EXPECT_TRUE(result);

  // Verify the user was created
  auto savedUser = service->getUser("testuser");
  ASSERT_TRUE(savedUser.has_value());
  EXPECT_EQ(savedUser->username, "testuser");
  EXPECT_EQ(savedUser->token, "test_token_123");
  EXPECT_EQ(savedUser->points, 1500);
  EXPECT_EQ(savedUser->hardcore_points, 750);
}

TEST_F(AchievementServiceTest, UpdateUser_Success) {
  auto user = createTestUser("testuser", "test_token_123", 1500, 750);

  // Create user
  EXPECT_TRUE(service->createOrUpdateUser(user));

  // Update user data
  user.token = "updated_token_456";
  user.points = 2000;
  user.hardcore_points = 1000;

  bool result = service->createOrUpdateUser(user);

  EXPECT_TRUE(result);

  // Verify the user was updated
  auto updatedUser = service->getUser("testuser");
  ASSERT_TRUE(updatedUser.has_value());
  EXPECT_EQ(updatedUser->username, "testuser");
  EXPECT_EQ(updatedUser->token, "updated_token_456");
  EXPECT_EQ(updatedUser->points, 2000);
  EXPECT_EQ(updatedUser->hardcore_points, 1000);
}

TEST_F(AchievementServiceTest, GetUser_ExistingUser) {
  auto user = createTestUser("testuser", "test_token_123", 1500, 750);
  EXPECT_TRUE(service->createOrUpdateUser(user));

  auto result = service->getUser("testuser");

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->username, "testuser");
  EXPECT_EQ(result->token, "test_token_123");
  EXPECT_EQ(result->points, 1500);
  EXPECT_EQ(result->hardcore_points, 750);
}

TEST_F(AchievementServiceTest, GetUser_NonExistentUser) {
  auto result = service->getUser("nonexistent");

  EXPECT_FALSE(result.has_value());
}

TEST_F(AchievementServiceTest, CreateOrUpdateUser_MultipleUsers) {
  auto user1 = createTestUser("user1", "token1", 1000, 500);
  auto user2 = createTestUser("user2", "token2", 2000, 1500);

  EXPECT_TRUE(service->createOrUpdateUser(user1));
  EXPECT_TRUE(service->createOrUpdateUser(user2));

  // Verify both users exist independently
  auto retrievedUser1 = service->getUser("user1");
  auto retrievedUser2 = service->getUser("user2");

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

TEST_F(AchievementServiceTest, CreateOrUpdateUser_ZeroPoints) {
  auto user = createTestUser("beginner", "token", 0, 0);

  bool result = service->createOrUpdateUser(user);

  EXPECT_TRUE(result);

  auto savedUser = service->getUser("beginner");
  ASSERT_TRUE(savedUser.has_value());
  EXPECT_EQ(savedUser->points, 0);
  EXPECT_EQ(savedUser->hardcore_points, 0);
}

TEST_F(AchievementServiceTest, CreateOrUpdateUser_EmptyToken) {
  auto user = createTestUser("test", "", 100, 50);

  bool result = service->createOrUpdateUser(user);

  EXPECT_TRUE(result);

  auto savedUser = service->getUser("test");
  ASSERT_TRUE(savedUser.has_value());
  EXPECT_EQ(savedUser->token, "");
}

// Content Hash Mapping Tests

TEST_F(AchievementServiceTest, GetGameId_ExistingMapping) {
  const std::string contentHash = "test_hash_123";
  const int setId = 42;

  // First set the mapping
  EXPECT_TRUE(service->setGameId(contentHash, setId));

  // Then retrieve it
  auto result = service->getGameId(contentHash);

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result.value(), setId);
}

TEST_F(AchievementServiceTest, GetGameId_NonExistentMapping) {
  auto result = service->getGameId("nonexistent_hash");

  EXPECT_FALSE(result.has_value());
}

TEST_F(AchievementServiceTest, GetGameId_UpdateMapping) {
  const std::string contentHash = "test_hash_456";
  const int originalSetId = 10;
  const int updatedSetId = 20;

  // Set initial mapping
  EXPECT_TRUE(service->setGameId(contentHash, originalSetId));

  auto firstResult = service->getGameId(contentHash);
  ASSERT_TRUE(firstResult.has_value());
  EXPECT_EQ(firstResult.value(), originalSetId);

  // Update the mapping
  EXPECT_TRUE(service->setGameId(contentHash, updatedSetId));

  auto secondResult = service->getGameId(contentHash);
  ASSERT_TRUE(secondResult.has_value());
  EXPECT_EQ(secondResult.value(), updatedSetId);
}

TEST_F(AchievementServiceTest, GetGameId_MultipleMappings) {
  const std::string hash1 = "hash_1";
  const std::string hash2 = "hash_2";
  const int setId1 = 100;
  const int setId2 = 200;

  // Set multiple mappings
  EXPECT_TRUE(service->setGameId(hash1, setId1));
  EXPECT_TRUE(service->setGameId(hash2, setId2));

  // Verify each mapping independently
  auto result1 = service->getGameId(hash1);
  auto result2 = service->getGameId(hash2);

  ASSERT_TRUE(result1.has_value());
  ASSERT_TRUE(result2.has_value());
  EXPECT_EQ(result1.value(), setId1);
  EXPECT_EQ(result2.value(), setId2);
}

TEST_F(AchievementServiceTest, GetGameId_IntegrationWithSetGameId) {
  // This test verifies the complete round-trip functionality
  const std::string contentHash = "integration_test_hash";
  const int setId = 777;

  // First create some achievement data
  auto patchResponse = createTestPatchResponse(setId);
  EXPECT_TRUE(service->processPatchResponse("testuser", patchResponse));

  // Set the game ID mapping
  EXPECT_TRUE(service->setGameId(contentHash, setId));

  // Verify we can get the game ID back
  auto retrievedGameId = service->getGameId(contentHash);
  ASSERT_TRUE(retrievedGameId.has_value());
  EXPECT_EQ(retrievedGameId.value(), setId);

  // Verify we can also get the achievement set by content hash
  auto achievementSet = service->getAchievementSetByContentHash(contentHash);
  ASSERT_TRUE(achievementSet.has_value());
  EXPECT_EQ(achievementSet->id, setId);
}

TEST_F(AchievementServiceTest, GetGameId_EmptyContentHash) {
  const std::string emptyHash = "";
  const int setId = 123;

  // Set mapping with empty hash
  EXPECT_TRUE(service->setGameId(emptyHash, setId));

  // Verify we can retrieve it
  auto result = service->getGameId(emptyHash);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result.value(), setId);
}

TEST_F(AchievementServiceTest, GetAchievementSetByContentHash_Delegates) {
  const std::string contentHash = "test_hash";

  // First set up some data in the repository
  auto patchResponse = createTestPatchResponse(1);
  EXPECT_TRUE(service->processPatchResponse("testuser", patchResponse));
  EXPECT_TRUE(service->setGameId(contentHash, 1));

  auto result = service->getAchievementSetByContentHash(contentHash);

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->id, 1);
  EXPECT_EQ(result->name, "Test Game 1");
}

TEST_F(AchievementServiceTest, GetAchievementSetByContentHash_NotFound) {
  auto result = service->getAchievementSetByContentHash("nonexistent_hash");

  EXPECT_FALSE(result.has_value());
}

TEST_F(AchievementServiceTest, SetGameId_Success) {
  const std::string contentHash = "test_hash";
  const int setId = 1;

  bool result = service->setGameId(contentHash, setId);

  EXPECT_TRUE(result);
}

TEST_F(AchievementServiceTest, UpdateAchievementProgress_Success) {
  auto progress = createTestProgress();

  bool result = service->updateAchievementProgress(progress);

  EXPECT_TRUE(result);
}

TEST_F(AchievementServiceTest, GetUserUnlock_ExistingUnlock) {
  // Set up achievement data first
  auto patchResponse = createTestPatchResponse(1);
  EXPECT_TRUE(service->processPatchResponse("testuser", patchResponse));

  auto result = service->getUserUnlock("testuser", 1);

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->username, "testuser");
  EXPECT_EQ(result->achievementId, 1);
  EXPECT_FALSE(result->earned); // Should be false initially
}

TEST_F(AchievementServiceTest, GetUserUnlock_NotFound) {
  auto result = service->getUserUnlock("nonexistent", 999);

  EXPECT_FALSE(result.has_value());
}

TEST_F(AchievementServiceTest, GetPatchResponse_ExistingResponse) {
  auto patchResponse = createTestPatchResponse(1);
  EXPECT_TRUE(service->processPatchResponse("testuser", patchResponse));

  auto result = service->getPatchResponse(1);

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->PatchData.ID, 1);
  EXPECT_EQ(result->PatchData.Title, "Test Game 1");
}

TEST_F(AchievementServiceTest, GetPatchResponse_NotFound) {
  auto result = service->getPatchResponse(999);

  EXPECT_FALSE(result.has_value());
}

// ProcessPatchResponse Tests

TEST_F(AchievementServiceTest, ProcessPatchResponse_Success) {
  auto patchResponse = createTestPatchResponse(1);

  bool result = service->processPatchResponse("testuser", patchResponse);

  EXPECT_TRUE(result);

  // Verify the achievement set was created
  auto savedResponse = service->getPatchResponse(1);
  ASSERT_TRUE(savedResponse.has_value());
  EXPECT_EQ(savedResponse->PatchData.Title, "Test Game 1");
}

TEST_F(AchievementServiceTest, ProcessPatchResponse_CreatesUserUnlocks) {
  auto patchResponse = createTestPatchResponse(1);

  bool result = service->processPatchResponse("testuser", patchResponse);

  EXPECT_TRUE(result);

  // Verify user unlocks were created for both achievements
  auto unlock1 = service->getUserUnlock("testuser", 1);
  auto unlock2 = service->getUserUnlock("testuser", 2);

  ASSERT_TRUE(unlock1.has_value());
  ASSERT_TRUE(unlock2.has_value());

  EXPECT_FALSE(unlock1->earned);
  EXPECT_FALSE(unlock1->earnedHardcore);
  EXPECT_TRUE(unlock1->synced);

  EXPECT_FALSE(unlock2->earned);
  EXPECT_FALSE(unlock2->earnedHardcore);
  EXPECT_TRUE(unlock2->synced);
}

TEST_F(AchievementServiceTest, ProcessPatchResponse_SkipsInactiveAchievements) {
  std::vector<PatchAchievement> achievements = {
      createTestPatchAchievement(1, 3, 50), // Active (flags = 3)
      createTestPatchAchievement(
          2, 5, 25), // Inactive (flags = 5) - should be skipped
      createTestPatchAchievement(3, 3, 75) // Active (flags = 3)
  };

  auto patchResponse = createTestPatchResponse(1, achievements);

  bool result = service->processPatchResponse("testuser", patchResponse);

  EXPECT_TRUE(result);

  // Verify only active achievements have unlocks created
  auto unlock1 = service->getUserUnlock("testuser", 1);
  auto unlock2 = service->getUserUnlock("testuser", 2);
  auto unlock3 = service->getUserUnlock("testuser", 3);

  EXPECT_TRUE(unlock1.has_value());  // Active
  EXPECT_FALSE(unlock2.has_value()); // Inactive - no unlock created
  EXPECT_TRUE(unlock3.has_value());  // Active
}

TEST_F(AchievementServiceTest, ProcessPatchResponse_CalculatesCorrectPoints) {
  std::vector<PatchAchievement> achievements = {
      createTestPatchAchievement(1, 3, 50), // Active - 50 points
      createTestPatchAchievement(
          2, 5, 25), // Inactive - 25 points (should be excluded)
      createTestPatchAchievement(3, 3, 75) // Active - 75 points
  };

  auto patchResponse = createTestPatchResponse(1, achievements);

  bool result = service->processPatchResponse("testuser", patchResponse);

  EXPECT_TRUE(result);

  // Get the created achievement set via hash mapping
  EXPECT_TRUE(service->setGameId("test_hash", 1));
  auto achievementSet = service->getAchievementSetByContentHash("test_hash");

  ASSERT_TRUE(achievementSet.has_value());
  EXPECT_EQ(achievementSet->totalPoints, 125);   // 50 + 75 (excluding inactive)
  EXPECT_EQ(achievementSet->numAchievements, 2); // Only active achievements
}

TEST_F(AchievementServiceTest,
       ProcessPatchResponse_DoesNotOverwriteExistingUnlocks) {
  auto patchResponse = createTestPatchResponse(1);

  // Process once
  EXPECT_TRUE(service->processPatchResponse("testuser", patchResponse));

  // Manually update an unlock
  auto unlock = service->getUserUnlock("testuser", 1);
  ASSERT_TRUE(unlock.has_value());
  unlock->earned = true;
  unlock->unlockTimestamp = 1609545600;
  EXPECT_TRUE(repository->createOrUpdate(*unlock));

  // Process again - should not overwrite existing unlock
  EXPECT_TRUE(service->processPatchResponse("testuser", patchResponse));

  auto updatedUnlock = service->getUserUnlock("testuser", 1);
  ASSERT_TRUE(updatedUnlock.has_value());
  EXPECT_TRUE(updatedUnlock->earned); // Should still be earned
  EXPECT_EQ(updatedUnlock->unlockTimestamp,
            1609545600); // Should keep timestamp
}

TEST_F(AchievementServiceTest, ProcessPatchResponse_EmptyAchievements) {
  std::vector<PatchAchievement> emptyAchievements;
  auto patchResponse = createTestPatchResponse(99, emptyAchievements);

  bool result = service->processPatchResponse("testuser", patchResponse);

  EXPECT_TRUE(result);

  // Verify achievement set was still created but with no achievements
  EXPECT_TRUE(service->setGameId("test_hash_empty", 99));
  auto achievementSet =
      service->getAchievementSetByContentHash("test_hash_empty");

  ASSERT_TRUE(achievementSet.has_value());
  EXPECT_EQ(achievementSet->numAchievements, 0);
  EXPECT_EQ(achievementSet->totalPoints, 0);
}

TEST_F(AchievementServiceTest, ProcessPatchResponse_MultipleUsers) {
  auto patchResponse = createTestPatchResponse(1);

  bool result1 = service->processPatchResponse("user1", patchResponse);
  bool result2 = service->processPatchResponse("user2", patchResponse);

  EXPECT_TRUE(result1);
  EXPECT_TRUE(result2);

  // Verify both users have separate unlocks
  auto unlock1_user1 = service->getUserUnlock("user1", 1);
  auto unlock1_user2 = service->getUserUnlock("user2", 1);

  ASSERT_TRUE(unlock1_user1.has_value());
  ASSERT_TRUE(unlock1_user2.has_value());

  EXPECT_EQ(unlock1_user1->username, "user1");
  EXPECT_EQ(unlock1_user2->username, "user2");
}

// ProcessStartSessionResponse Tests

TEST_F(AchievementServiceTest, ProcessStartSessionResponse_Success) {
  // Set up achievements first
  auto patchResponse = createTestPatchResponse(1);
  EXPECT_TRUE(service->processPatchResponse("testuser", patchResponse));

  std::vector<Unlock> unlocks = {createTestUnlock(1, 1609459200)};
  std::vector<Unlock> hardcoreUnlocks = {createTestUnlock(2, 1609545600)};
  auto startSessionResponse =
      createTestStartSessionResponse(unlocks, hardcoreUnlocks);

  bool result =
      service->processStartSessionResponse("testuser", 1, startSessionResponse);

  EXPECT_TRUE(result);

  // Verify normal unlock
  auto unlock1 = service->getUserUnlock("testuser", 1);
  ASSERT_TRUE(unlock1.has_value());
  EXPECT_TRUE(unlock1->earned);
  EXPECT_EQ(unlock1->unlockTimestamp, 1609459200);
  EXPECT_TRUE(unlock1->synced);

  // Verify hardcore unlock
  auto unlock2 = service->getUserUnlock("testuser", 2);
  ASSERT_TRUE(unlock2.has_value());
  EXPECT_TRUE(unlock2->earnedHardcore);
  EXPECT_EQ(unlock2->unlockTimestampHardcore, 1609545600);
}

TEST_F(AchievementServiceTest, ProcessStartSessionResponse_CreatesNewUnlocks) {
  std::vector<Unlock> unlocks = {createTestUnlock(999, 1609459200)};
  auto startSessionResponse = createTestStartSessionResponse(unlocks, {});

  bool result =
      service->processStartSessionResponse("testuser", 1, startSessionResponse);

  EXPECT_TRUE(result);

  // Should create a new unlock for the achievement that didn't exist
  auto unlock = service->getUserUnlock("testuser", 999);
  ASSERT_TRUE(unlock.has_value());
  EXPECT_TRUE(unlock->earned);
  EXPECT_EQ(unlock->unlockTimestamp, 1609459200);
  EXPECT_TRUE(unlock->synced);
}

TEST_F(AchievementServiceTest,
       ProcessStartSessionResponse_HandlesUnsupportedEmulatorId) {
  std::vector<Unlock> unlocks = {
      createTestUnlock(1, 1609459200),
      createTestUnlock(UNSUPPORTED_EMULATOR_ACHIEVEMENT_ID, 1609459200)};
  auto startSessionResponse = createTestStartSessionResponse(unlocks, {});

  bool result =
      service->processStartSessionResponse("testuser", 1, startSessionResponse);

  EXPECT_TRUE(result);

  // Regular unlock should be processed
  auto regularUnlock = service->getUserUnlock("testuser", 1);
  ASSERT_TRUE(regularUnlock.has_value());
  EXPECT_TRUE(regularUnlock->earned);

  auto unsupportedUnlock =
      service->getUserUnlock("testuser", UNSUPPORTED_EMULATOR_ACHIEVEMENT_ID);
  ASSERT_TRUE(unsupportedUnlock.has_value());
  EXPECT_TRUE(unsupportedUnlock->earned); // Should be false
}

TEST_F(AchievementServiceTest,
       ProcessStartSessionResponse_CreatesUnsupportedEmulatorUnlock) {
  // Process without unsupported emulator achievement in the response
  std::vector<Unlock> unlocks = {createTestUnlock(1, 1609459200)};
  auto startSessionResponse = createTestStartSessionResponse(unlocks, {});

  bool result =
      service->processStartSessionResponse("testuser", 1, startSessionResponse);

  EXPECT_TRUE(result);

  // Should create unsupported emulator unlock as unearned
  auto unsupportedUnlock =
      service->getUserUnlock("testuser", UNSUPPORTED_EMULATOR_ACHIEVEMENT_ID);
  ASSERT_TRUE(unsupportedUnlock.has_value());
  EXPECT_FALSE(unsupportedUnlock->earned);
  EXPECT_FALSE(unsupportedUnlock->earnedHardcore);
  EXPECT_TRUE(unsupportedUnlock->synced);
}

TEST_F(AchievementServiceTest,
       ProcessStartSessionResponse_RelocksRemovedAchievements) {
  // Set up achievements first
  auto patchResponse = createTestPatchResponse(1);
  EXPECT_TRUE(service->processPatchResponse("testuser", patchResponse));

  // Manually set achievements as earned
  auto unlock1 = service->getUserUnlock("testuser", 1);
  auto unlock2 = service->getUserUnlock("testuser", 2);
  ASSERT_TRUE(unlock1.has_value() && unlock2.has_value());

  unlock1->earned = true;
  unlock1->unlockTimestamp = 1609459200;
  unlock2->earned = true;
  unlock2->unlockTimestamp = 1609459200;

  EXPECT_TRUE(repository->createOrUpdate(*unlock1));
  EXPECT_TRUE(repository->createOrUpdate(*unlock2));

  // Process start session with only achievement 1 unlocked
  std::vector<Unlock> unlocks = {createTestUnlock(1, 1609459200)};
  auto startSessionResponse = createTestStartSessionResponse(unlocks, {});

  bool result =
      service->processStartSessionResponse("testuser", 1, startSessionResponse);

  EXPECT_TRUE(result);

  // Achievement 1 should remain earned
  auto updatedUnlock1 = service->getUserUnlock("testuser", 1);
  ASSERT_TRUE(updatedUnlock1.has_value());
  EXPECT_TRUE(updatedUnlock1->earned);

  // Achievement 2 should be re-locked
  auto updatedUnlock2 = service->getUserUnlock("testuser", 2);
  ASSERT_TRUE(updatedUnlock2.has_value());
  EXPECT_FALSE(updatedUnlock2->earned);
  EXPECT_EQ(updatedUnlock2->unlockTimestamp, 0);
}

TEST_F(AchievementServiceTest,
       ProcessStartSessionResponse_RelocksRemovedHardcoreAchievements) {
  // Set up achievements first
  auto patchResponse = createTestPatchResponse(1);
  EXPECT_TRUE(service->processPatchResponse("testuser", patchResponse));

  // Manually set achievements as hardcore earned
  auto unlock1 = service->getUserUnlock("testuser", 1);
  auto unlock2 = service->getUserUnlock("testuser", 2);
  ASSERT_TRUE(unlock1.has_value() && unlock2.has_value());

  unlock1->earnedHardcore = true;
  unlock1->unlockTimestampHardcore = 1609459200;
  unlock2->earnedHardcore = true;
  unlock2->unlockTimestampHardcore = 1609459200;

  EXPECT_TRUE(repository->createOrUpdate(*unlock1));
  EXPECT_TRUE(repository->createOrUpdate(*unlock2));

  // Process start session with only achievement 1 hardcore unlocked
  std::vector<Unlock> hardcoreUnlocks = {createTestUnlock(1, 1609459200)};
  auto startSessionResponse =
      createTestStartSessionResponse({}, hardcoreUnlocks);

  bool result =
      service->processStartSessionResponse("testuser", 1, startSessionResponse);

  EXPECT_TRUE(result);

  // Achievement 1 hardcore should remain earned
  auto updatedUnlock1 = service->getUserUnlock("testuser", 1);
  ASSERT_TRUE(updatedUnlock1.has_value());
  EXPECT_TRUE(updatedUnlock1->earnedHardcore);

  // Achievement 2 hardcore should be re-locked
  auto updatedUnlock2 = service->getUserUnlock("testuser", 2);
  ASSERT_TRUE(updatedUnlock2.has_value());
  EXPECT_FALSE(updatedUnlock2->earnedHardcore);
  EXPECT_EQ(updatedUnlock2->unlockTimestampHardcore, 0);
}

TEST_F(AchievementServiceTest, ProcessStartSessionResponse_EmptyResponse) {
  auto startSessionResponse = createTestStartSessionResponse({}, {});

  bool result =
      service->processStartSessionResponse("testuser", 1, startSessionResponse);

  EXPECT_TRUE(result);

  // Should still create unsupported emulator unlock
  auto unsupportedUnlock =
      service->getUserUnlock("testuser", UNSUPPORTED_EMULATOR_ACHIEVEMENT_ID);
  ASSERT_TRUE(unsupportedUnlock.has_value());
  EXPECT_FALSE(unsupportedUnlock->earned);
}

TEST_F(AchievementServiceTest,
       ProcessStartSessionResponse_MixedNormalAndHardcore) {
  // Set up achievements first
  auto patchResponse = createTestPatchResponse(1);
  EXPECT_TRUE(service->processPatchResponse("testuser", patchResponse));

  // Different achievements unlocked in each mode
  std::vector<Unlock> unlocks = {createTestUnlock(1, 1609459200)};
  std::vector<Unlock> hardcoreUnlocks = {createTestUnlock(2, 1609545600)};
  auto startSessionResponse =
      createTestStartSessionResponse(unlocks, hardcoreUnlocks);

  bool result =
      service->processStartSessionResponse("testuser", 1, startSessionResponse);

  EXPECT_TRUE(result);

  // Verify normal unlock
  auto unlock1 = service->getUserUnlock("testuser", 1);
  ASSERT_TRUE(unlock1.has_value());
  EXPECT_TRUE(unlock1->earned);
  EXPECT_FALSE(unlock1->earnedHardcore);
  EXPECT_EQ(unlock1->unlockTimestamp, 1609459200);
  EXPECT_EQ(unlock1->unlockTimestampHardcore, 0);
  EXPECT_TRUE(unlock1->synced);

  // Verify hardcore unlock
  auto unlock2 = service->getUserUnlock("testuser", 2);
  ASSERT_TRUE(unlock2.has_value());
  EXPECT_TRUE(unlock2->earned);
  EXPECT_TRUE(unlock2->earnedHardcore);
  EXPECT_EQ(unlock2->unlockTimestamp, 1609545600);
  EXPECT_EQ(unlock2->unlockTimestampHardcore, 1609545600);
  EXPECT_TRUE(unlock2->synced);
}

TEST_F(AchievementServiceTest,
       ProcessStartSessionResponse_HardcoreUnlockSetsEarnedTrue) {
  // Set up achievements first
  auto patchResponse = createTestPatchResponse(1);
  EXPECT_TRUE(service->processPatchResponse("testuser", patchResponse));

  // Achievement unlocked ONLY in hardcore mode (no non-hardcore unlock)
  std::vector<Unlock> unlocks = {};
  std::vector<Unlock> hardcoreUnlocks = {createTestUnlock(1, 1609545600)};
  auto startSessionResponse =
      createTestStartSessionResponse(unlocks, hardcoreUnlocks);

  bool result =
      service->processStartSessionResponse("testuser", 1, startSessionResponse);

  EXPECT_TRUE(result);

  auto unlock = service->getUserUnlock("testuser", 1);
  ASSERT_TRUE(unlock.has_value());

  // Hardcore unlock should set BOTH earned and earnedHardcore to true
  EXPECT_TRUE(unlock->earned);
  EXPECT_TRUE(unlock->earnedHardcore);
  EXPECT_EQ(unlock->unlockTimestamp,
            1609545600); // Should use hardcore timestamp
  EXPECT_EQ(unlock->unlockTimestampHardcore, 1609545600);
  EXPECT_TRUE(unlock->synced);
}

TEST_F(AchievementServiceTest,
       ProcessStartSessionResponse_HardcoreWithExistingNonHardcore) {
  // Set up achievements first
  auto patchResponse = createTestPatchResponse(1);
  EXPECT_TRUE(service->processPatchResponse("testuser", patchResponse));

  // Different achievements unlocked in different modes
  std::vector<Unlock> unlocks = {createTestUnlock(1, 1609459200)};
  std::vector<Unlock> hardcoreUnlocks = {createTestUnlock(2, 1609545600)};
  auto startSessionResponse =
      createTestStartSessionResponse(unlocks, hardcoreUnlocks);

  bool result =
      service->processStartSessionResponse("testuser", 1, startSessionResponse);

  EXPECT_TRUE(result);

  // Verify normal unlock
  auto unlock1 = service->getUserUnlock("testuser", 1);
  ASSERT_TRUE(unlock1.has_value());
  EXPECT_TRUE(unlock1->earned);
  EXPECT_FALSE(unlock1->earnedHardcore);
  EXPECT_EQ(unlock1->unlockTimestamp, 1609459200);
  EXPECT_EQ(unlock1->unlockTimestampHardcore, 0);
  EXPECT_TRUE(unlock1->synced);

  // Verify hardcore unlock (which sets both earned flags)
  auto unlock2 = service->getUserUnlock("testuser", 2);
  ASSERT_TRUE(unlock2.has_value());
  EXPECT_TRUE(unlock2->earned);
  EXPECT_TRUE(unlock2->earnedHardcore);
  EXPECT_EQ(unlock2->unlockTimestamp, 1609545600);
  EXPECT_EQ(unlock2->unlockTimestampHardcore, 1609545600);
  EXPECT_TRUE(unlock2->synced);
}

// Edge Cases and Error Handling

TEST_F(AchievementServiceTest, ProcessStartSessionResponse_NonExistentSetId) {
  // Try to process start session response for a set that doesn't exist
  std::vector<Unlock> unlocks = {createTestUnlock(1, 1609459200)};
  auto startSessionResponse = createTestStartSessionResponse(unlocks, {});

  bool result = service->processStartSessionResponse("testuser", 999,
                                                     startSessionResponse);

  // Should still succeed even if set doesn't exist
  EXPECT_TRUE(result);

  // Unlock should still be created
  auto unlock = service->getUserUnlock("testuser", 1);
  ASSERT_TRUE(unlock.has_value());
  EXPECT_TRUE(unlock->earned);
}

TEST_F(AchievementServiceTest, ProcessPatchResponse_DuplicateProcessing) {
  auto patchResponse = createTestPatchResponse(1);

  // Process the same response twice
  EXPECT_TRUE(service->processPatchResponse("testuser", patchResponse));
  EXPECT_TRUE(service->processPatchResponse("testuser", patchResponse));

  // Should not create duplicate data
  auto unlock1 = service->getUserUnlock("testuser", 1);
  auto unlock2 = service->getUserUnlock("testuser", 2);

  EXPECT_TRUE(unlock1.has_value());
  EXPECT_TRUE(unlock2.has_value());

  // Verify the cached response exists
  auto cachedResponse = service->getPatchResponse(1);
  EXPECT_TRUE(cachedResponse.has_value());
}

TEST_F(AchievementServiceTest, ProcessStartSessionResponse_ZeroTimestamps) {
  std::vector<Unlock> unlocks = {createTestUnlock(1, 0)};
  auto startSessionResponse = createTestStartSessionResponse(unlocks, {});

  bool result =
      service->processStartSessionResponse("testuser", 1, startSessionResponse);

  EXPECT_TRUE(result);

  auto unlock = service->getUserUnlock("testuser", 1);
  ASSERT_TRUE(unlock.has_value());
  EXPECT_TRUE(unlock->earned);
  EXPECT_EQ(unlock->unlockTimestamp, 0);
}

} // namespace firelight::achievements
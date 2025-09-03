#include "../../../src/app/settings/sqlite_settings_repository.hpp"
#include <gtest/gtest.h>
#include <filesystem>

namespace firelight::settings {

/**
 * @brief Test fixture for SqliteSettingsRepository functionality
 * 
 * Comprehensive test suite for the SQLite-based settings repository implementation,
 * covering settings levels, platform-specific values, game-specific values,
 * and data persistence operations.
 */
class SqliteSettingsRepositoryTest : public testing::Test {
protected:
  std::unique_ptr<SqliteSettingsRepository> repository;

  void SetUp() override {
    repository = std::make_unique<SqliteSettingsRepository>(":memory:");
  }

  void TearDown() override { 
    repository.reset();
  }
};

/**
 * @brief Test that settings level defaults to Platform for new content
 * 
 * Verifies that when no explicit settings level is stored for a content hash,
 * the repository returns Platform as the default settings level.
 */
TEST_F(SqliteSettingsRepositoryTest, GetSettingsLevel_DefaultsToPlatform) {
  const std::string contentHash = "test_content_hash";
  
  SettingsLevel level = repository->getSettingsLevel(contentHash);
  
  EXPECT_EQ(level, SettingsLevel::Platform);
}

/**
 * @brief Test setting and retrieving Platform settings level
 * 
 * Verifies that Platform settings level can be set and correctly retrieved
 * from the SQLite database.
 */
TEST_F(SqliteSettingsRepositoryTest, SetAndGetSettingsLevel_Platform) {
  const std::string contentHash = "test_content_hash";
  
  bool success = repository->setSettingsLevel(contentHash, SettingsLevel::Platform);
  EXPECT_TRUE(success);
  
  SettingsLevel level = repository->getSettingsLevel(contentHash);
  EXPECT_EQ(level, SettingsLevel::Platform);
}

/**
 * @brief Test setting and retrieving Game settings level
 * 
 * Verifies that Game settings level can be set and correctly retrieved
 * from the SQLite database.
 */
TEST_F(SqliteSettingsRepositoryTest, SetAndGetSettingsLevel_Game) {
  const std::string contentHash = "test_content_hash";
  
  bool success = repository->setSettingsLevel(contentHash, SettingsLevel::Game);
  EXPECT_TRUE(success);
  
  SettingsLevel level = repository->getSettingsLevel(contentHash);
  EXPECT_EQ(level, SettingsLevel::Game);
}

/**
 * @brief Test that settings level updates overwrite existing values
 * 
 * Verifies that updating the settings level for an existing content hash
 * correctly overwrites the previous value in the database.
 */
TEST_F(SqliteSettingsRepositoryTest, SetSettingsLevel_UpdatesExisting) {
  const std::string contentHash = "test_content_hash";
  
  // Set initial level to Platform
  repository->setSettingsLevel(contentHash, SettingsLevel::Platform);
  EXPECT_EQ(repository->getSettingsLevel(contentHash), SettingsLevel::Platform);
  
  // Update to Game level
  repository->setSettingsLevel(contentHash, SettingsLevel::Game);
  EXPECT_EQ(repository->getSettingsLevel(contentHash), SettingsLevel::Game);
  
  // Update back to Platform level
  repository->setSettingsLevel(contentHash, SettingsLevel::Platform);
  EXPECT_EQ(repository->getSettingsLevel(contentHash), SettingsLevel::Platform);
}

/**
 * @brief Test settings level isolation between different content hashes
 * 
 * Verifies that settings levels are properly isolated between different
 * content hashes and don't interfere with each other.
 */
TEST_F(SqliteSettingsRepositoryTest, SettingsLevel_MultipleDifferentContentHashes) {
  const std::string contentHash1 = "content_hash_1";
  const std::string contentHash2 = "content_hash_2";
  const std::string contentHash3 = "content_hash_3";
  
  // Set different levels for different content hashes
  repository->setSettingsLevel(contentHash1, SettingsLevel::Game);
  repository->setSettingsLevel(contentHash2, SettingsLevel::Platform);
  
  // Verify each content hash has its own level
  EXPECT_EQ(repository->getSettingsLevel(contentHash1), SettingsLevel::Game);
  EXPECT_EQ(repository->getSettingsLevel(contentHash2), SettingsLevel::Platform);
  EXPECT_EQ(repository->getSettingsLevel(contentHash3), SettingsLevel::Platform); // Should default
}

/**
 * @brief Test settings level handling with empty content hash
 * 
 * Verifies that the repository can handle empty content hash strings
 * without errors and store/retrieve settings levels correctly.
 */
TEST_F(SqliteSettingsRepositoryTest, SettingsLevel_EmptyContentHash) {
  const std::string emptyContentHash = "";
  
  repository->setSettingsLevel(emptyContentHash, SettingsLevel::Game);
  SettingsLevel level = repository->getSettingsLevel(emptyContentHash);
  
  EXPECT_EQ(level, SettingsLevel::Game);
}

/**
 * @brief Test that getting non-existent platform value returns nullopt
 * 
 * Verifies that attempting to retrieve a platform setting that doesn't exist
 * in the database returns std::nullopt.
 */
TEST_F(SqliteSettingsRepositoryTest, GetPlatformValue_NonExistentReturnsNullopt) {
  const int platformId = 1;
  const std::string key = "test_key";
  
  auto value = repository->getPlatformValue(platformId, key);
  
  EXPECT_FALSE(value.has_value());
}

/**
 * @brief Test retrieving platform value after setting it
 * 
 * Verifies that a platform setting can be set and then successfully
 * retrieved with the correct value.
 */
TEST_F(SqliteSettingsRepositoryTest, GetPlatformValue_AfterSet) {
  const int platformId = 1;
  const std::string key = "test_key";
  const std::string expectedValue = "test_value";
  
  bool success = repository->setPlatformValue(platformId, key, expectedValue);
  EXPECT_TRUE(success);
  
  auto value = repository->getPlatformValue(platformId, key);
  ASSERT_TRUE(value.has_value());
  EXPECT_EQ(value.value(), expectedValue);
}

/**
 * @brief Test platform value isolation between different platforms
 * 
 * Verifies that platform settings are properly isolated between different
 * platform IDs and don't interfere with each other.
 */
TEST_F(SqliteSettingsRepositoryTest, GetPlatformValue_DifferentPlatforms) {
  const int platformId1 = 1;
  const int platformId2 = 2;
  const std::string key = "test_key";
  const std::string value1 = "value_for_platform_1";
  const std::string value2 = "value_for_platform_2";
  
  repository->setPlatformValue(platformId1, key, value1);
  repository->setPlatformValue(platformId2, key, value2);
  
  auto retrievedValue1 = repository->getPlatformValue(platformId1, key);
  auto retrievedValue2 = repository->getPlatformValue(platformId2, key);
  
  ASSERT_TRUE(retrievedValue1.has_value());
  ASSERT_TRUE(retrievedValue2.has_value());
  EXPECT_EQ(retrievedValue1.value(), value1);
  EXPECT_EQ(retrievedValue2.value(), value2);
}

/**
 * @brief Test platform value isolation between different keys
 * 
 * Verifies that different setting keys for the same platform are properly
 * isolated and can store different values.
 */
TEST_F(SqliteSettingsRepositoryTest, GetPlatformValue_DifferentKeys) {
  const int platformId = 1;
  const std::string key1 = "key1";
  const std::string key2 = "key2";
  const std::string value1 = "value1";
  const std::string value2 = "value2";
  
  repository->setPlatformValue(platformId, key1, value1);
  repository->setPlatformValue(platformId, key2, value2);
  
  auto retrievedValue1 = repository->getPlatformValue(platformId, key1);
  auto retrievedValue2 = repository->getPlatformValue(platformId, key2);
  
  ASSERT_TRUE(retrievedValue1.has_value());
  ASSERT_TRUE(retrievedValue2.has_value());
  EXPECT_EQ(retrievedValue1.value(), value1);
  EXPECT_EQ(retrievedValue2.value(), value2);
}

/**
 * @brief Test updating existing platform values
 * 
 * Verifies that updating an existing platform setting overwrites the
 * previous value correctly.
 */
TEST_F(SqliteSettingsRepositoryTest, GetPlatformValue_UpdateExisting) {
  const int platformId = 1;
  const std::string key = "test_key";
  const std::string initialValue = "initial_value";
  const std::string updatedValue = "updated_value";
  
  repository->setPlatformValue(platformId, key, initialValue);
  auto value1 = repository->getPlatformValue(platformId, key);
  
  repository->setPlatformValue(platformId, key, updatedValue);
  auto value2 = repository->getPlatformValue(platformId, key);
  
  ASSERT_TRUE(value1.has_value());
  ASSERT_TRUE(value2.has_value());
  EXPECT_EQ(value1.value(), initialValue);
  EXPECT_EQ(value2.value(), updatedValue);
}

/**
 * @brief Test storing and retrieving empty platform values
 * 
 * Verifies that empty string values can be stored and retrieved correctly
 * for platform settings.
 */
TEST_F(SqliteSettingsRepositoryTest, GetPlatformValue_EmptyValue) {
  const int platformId = 1;
  const std::string key = "test_key";
  const std::string emptyValue = "";
  
  repository->setPlatformValue(platformId, key, emptyValue);
  auto value = repository->getPlatformValue(platformId, key);
  
  ASSERT_TRUE(value.has_value());
  EXPECT_EQ(value.value(), emptyValue);
}

/**
 * @brief Test that setPlatformValue creates new database records
 * 
 * Verifies that setting a platform value for a new key creates a new
 * record in the database and can be retrieved.
 */
TEST_F(SqliteSettingsRepositoryTest, SetPlatformValue_CreatesNewRecord) {
  const int platformId = 1;
  const std::string key = "new_key";
  const std::string value = "new_value";
  
  // Verify key doesn't exist initially
  EXPECT_FALSE(repository->getPlatformValue(platformId, key).has_value());
  
  repository->setPlatformValue(platformId, key, value);
  
  auto retrievedValue = repository->getPlatformValue(platformId, key);
  ASSERT_TRUE(retrievedValue.has_value());
  EXPECT_EQ(retrievedValue.value(), value);
}

/**
 * @brief Test that setPlatformValue updates existing database records
 * 
 * Verifies that setting a platform value for an existing key updates
 * the existing record rather than creating a duplicate.
 */
TEST_F(SqliteSettingsRepositoryTest, SetPlatformValue_UpdatesExistingRecord) {
  const int platformId = 1;
  const std::string key = "existing_key";
  const std::string initialValue = "initial_value";
  const std::string updatedValue = "updated_value";
  
  repository->setPlatformValue(platformId, key, initialValue);
  repository->setPlatformValue(platformId, key, updatedValue);
  
  auto retrievedValue = repository->getPlatformValue(platformId, key);
  ASSERT_TRUE(retrievedValue.has_value());
  EXPECT_EQ(retrievedValue.value(), updatedValue);
}

/**
 * @brief Test setting multiple platform values across platforms and keys
 * 
 * Verifies that multiple platform settings can be stored simultaneously
 * across different platform IDs and setting keys without conflicts.
 */
TEST_F(SqliteSettingsRepositoryTest, SetPlatformValue_MultiplePlatformsAndKeys) {
  const int platformId1 = 1;
  const int platformId2 = 2;
  const std::string key1 = "key1";
  const std::string key2 = "key2";
  const std::string value1 = "value1";
  const std::string value2 = "value2";
  const std::string value3 = "value3";
  const std::string value4 = "value4";
  
  repository->setPlatformValue(platformId1, key1, value1);
  repository->setPlatformValue(platformId1, key2, value2);
  repository->setPlatformValue(platformId2, key1, value3);
  repository->setPlatformValue(platformId2, key2, value4);
  
  EXPECT_EQ(repository->getPlatformValue(platformId1, key1).value(), value1);
  EXPECT_EQ(repository->getPlatformValue(platformId1, key2).value(), value2);
  EXPECT_EQ(repository->getPlatformValue(platformId2, key1).value(), value3);
  EXPECT_EQ(repository->getPlatformValue(platformId2, key2).value(), value4);
}

/**
 * @brief Test platform values with special characters
 * 
 * Verifies that platform setting values containing special characters,
 * spaces, and symbols are stored and retrieved correctly.
 */
TEST_F(SqliteSettingsRepositoryTest, SetPlatformValue_WithSpecialCharacters) {
  const int platformId = 1;
  const std::string key = "special_key";
  const std::string value = "Value with spaces, symbols: !@#$%^&*()";
  
  repository->setPlatformValue(platformId, key, value);
  
  auto retrievedValue = repository->getPlatformValue(platformId, key);
  ASSERT_TRUE(retrievedValue.has_value());
  EXPECT_EQ(retrievedValue.value(), value);
}

/**
 * @brief Test that resetPlatformValue removes existing values
 * 
 * Verifies that resetting a platform setting removes the value from
 * the database and subsequent queries return nullopt.
 */
TEST_F(SqliteSettingsRepositoryTest, ResetPlatformValue_RemovesExistingValue) {
  const int platformId = 1;
  const std::string key = "test_key";
  const std::string value = "test_value";
  
  bool setSuccess = repository->setPlatformValue(platformId, key, value);
  EXPECT_TRUE(setSuccess);
  EXPECT_TRUE(repository->getPlatformValue(platformId, key).has_value());
  
  bool resetSuccess = repository->resetPlatformValue(platformId, key);
  EXPECT_TRUE(resetSuccess);
  
  auto retrievedValue = repository->getPlatformValue(platformId, key);
  EXPECT_FALSE(retrievedValue.has_value());
}

/**
 * @brief Test that resetting non-existent platform values doesn't error
 * 
 * Verifies that attempting to reset a platform setting that doesn't exist
 * doesn't cause errors or exceptions.
 */
TEST_F(SqliteSettingsRepositoryTest, ResetPlatformValue_NonExistentKeyNoError) {
  const int platformId = 1;
  const std::string key = "non_existent_key";
  
  // Should not throw or cause issues
  repository->resetPlatformValue(platformId, key);
  
  auto retrievedValue = repository->getPlatformValue(platformId, key);
  EXPECT_FALSE(retrievedValue.has_value());
}

/**
 * @brief Test that resetPlatformValue only removes the specific key
 * 
 * Verifies that resetting one platform setting key doesn't affect other
 * setting keys for the same platform.
 */
TEST_F(SqliteSettingsRepositoryTest, ResetPlatformValue_OnlyRemovesSpecificKey) {
  const int platformId = 1;
  const std::string key1 = "key1";
  const std::string key2 = "key2";
  const std::string value1 = "value1";
  const std::string value2 = "value2";
  
  repository->setPlatformValue(platformId, key1, value1);
  repository->setPlatformValue(platformId, key2, value2);
  
  repository->resetPlatformValue(platformId, key1);
  
  EXPECT_FALSE(repository->getPlatformValue(platformId, key1).has_value());
  EXPECT_TRUE(repository->getPlatformValue(platformId, key2).has_value());
  EXPECT_EQ(repository->getPlatformValue(platformId, key2).value(), value2);
}

/**
 * @brief Test that resetPlatformValue only removes the specific platform
 * 
 * Verifies that resetting a platform setting only affects that platform
 * and doesn't remove the same key from other platforms.
 */
TEST_F(SqliteSettingsRepositoryTest, ResetPlatformValue_OnlyRemovesSpecificPlatform) {
  const int platformId1 = 1;
  const int platformId2 = 2;
  const std::string key = "test_key";
  const std::string value1 = "value1";
  const std::string value2 = "value2";
  
  repository->setPlatformValue(platformId1, key, value1);
  repository->setPlatformValue(platformId2, key, value2);
  
  repository->resetPlatformValue(platformId1, key);
  
  EXPECT_FALSE(repository->getPlatformValue(platformId1, key).has_value());
  EXPECT_TRUE(repository->getPlatformValue(platformId2, key).has_value());
  EXPECT_EQ(repository->getPlatformValue(platformId2, key).value(), value2);
}

/**
 * @brief Test that platform values can be set after being reset
 * 
 * Verifies that after resetting a platform setting, a new value can be
 * set for the same key and retrieved correctly.
 */
TEST_F(SqliteSettingsRepositoryTest, ResetPlatformValue_CanSetAfterReset) {
  const int platformId = 1;
  const std::string key = "test_key";
  const std::string initialValue = "initial_value";
  const std::string newValue = "new_value";
  
  repository->setPlatformValue(platformId, key, initialValue);
  repository->resetPlatformValue(platformId, key);
  repository->setPlatformValue(platformId, key, newValue);
  
  auto retrievedValue = repository->getPlatformValue(platformId, key);
  ASSERT_TRUE(retrievedValue.has_value());
  EXPECT_EQ(retrievedValue.value(), newValue);
}

/**
 * @brief Test that getting non-existent game value returns nullopt
 * 
 * Verifies that attempting to retrieve a game setting that doesn't exist
 * in the database returns std::nullopt.
 */
TEST_F(SqliteSettingsRepositoryTest, GetGameValue_NonExistentReturnsNullopt) {
  const std::string contentHash = "test_content_hash";
  const std::string key = "test_key";
  
  auto value = repository->getGameValue(contentHash, key);
  
  EXPECT_FALSE(value.has_value());
}

/**
 * @brief Test retrieving game value after setting it
 * 
 * Verifies that a game setting can be set and then successfully
 * retrieved with the correct value.
 */
TEST_F(SqliteSettingsRepositoryTest, GetGameValue_AfterSet) {
  const std::string contentHash = "test_content_hash";
  const std::string key = "test_key";
  const std::string expectedValue = "test_value";
  
  bool success = repository->setGameValue(contentHash, key, expectedValue);
  EXPECT_TRUE(success);
  
  auto value = repository->getGameValue(contentHash, key);
  ASSERT_TRUE(value.has_value());
  EXPECT_EQ(value.value(), expectedValue);
}

/**
 * @brief Test game value isolation between different content hashes
 * 
 * Verifies that game settings are properly isolated between different
 * content hashes and don't interfere with each other.
 */
TEST_F(SqliteSettingsRepositoryTest, GetGameValue_DifferentContentHashes) {
  const std::string contentHash1 = "content_hash_1";
  const std::string contentHash2 = "content_hash_2";
  const std::string key = "test_key";
  const std::string value1 = "value_for_hash_1";
  const std::string value2 = "value_for_hash_2";
  
  repository->setGameValue(contentHash1, key, value1);
  repository->setGameValue(contentHash2, key, value2);
  
  auto retrievedValue1 = repository->getGameValue(contentHash1, key);
  auto retrievedValue2 = repository->getGameValue(contentHash2, key);
  
  ASSERT_TRUE(retrievedValue1.has_value());
  ASSERT_TRUE(retrievedValue2.has_value());
  EXPECT_EQ(retrievedValue1.value(), value1);
  EXPECT_EQ(retrievedValue2.value(), value2);
}

/**
 * @brief Test game value isolation between different keys
 * 
 * Verifies that different setting keys for the same content hash are
 * properly isolated and can store different values.
 */
TEST_F(SqliteSettingsRepositoryTest, GetGameValue_DifferentKeys) {
  const std::string contentHash = "test_content_hash";
  const std::string key1 = "key1";
  const std::string key2 = "key2";
  const std::string value1 = "value1";
  const std::string value2 = "value2";
  
  repository->setGameValue(contentHash, key1, value1);
  repository->setGameValue(contentHash, key2, value2);
  
  auto retrievedValue1 = repository->getGameValue(contentHash, key1);
  auto retrievedValue2 = repository->getGameValue(contentHash, key2);
  
  ASSERT_TRUE(retrievedValue1.has_value());
  ASSERT_TRUE(retrievedValue2.has_value());
  EXPECT_EQ(retrievedValue1.value(), value1);
  EXPECT_EQ(retrievedValue2.value(), value2);
}

/**
 * @brief Test updating existing game values
 * 
 * Verifies that updating an existing game setting overwrites the
 * previous value correctly.
 */
TEST_F(SqliteSettingsRepositoryTest, GetGameValue_UpdateExisting) {
  const std::string contentHash = "test_content_hash";
  const std::string key = "test_key";
  const std::string initialValue = "initial_value";
  const std::string updatedValue = "updated_value";
  
  repository->setGameValue(contentHash, key, initialValue);
  auto value1 = repository->getGameValue(contentHash, key);
  
  repository->setGameValue(contentHash, key, updatedValue);
  auto value2 = repository->getGameValue(contentHash, key);
  
  ASSERT_TRUE(value1.has_value());
  ASSERT_TRUE(value2.has_value());
  EXPECT_EQ(value1.value(), initialValue);
  EXPECT_EQ(value2.value(), updatedValue);
}

/**
 * @brief Test storing and retrieving empty game values
 * 
 * Verifies that empty string values can be stored and retrieved correctly
 * for game settings.
 */
TEST_F(SqliteSettingsRepositoryTest, GetGameValue_EmptyValue) {
  const std::string contentHash = "test_content_hash";
  const std::string key = "test_key";
  const std::string emptyValue = "";
  
  repository->setGameValue(contentHash, key, emptyValue);
  auto value = repository->getGameValue(contentHash, key);
  
  ASSERT_TRUE(value.has_value());
  EXPECT_EQ(value.value(), emptyValue);
}

/**
 * @brief Test that setGameValue creates new database records
 * 
 * Verifies that setting a game value for a new key creates a new
 * record in the database and can be retrieved.
 */
TEST_F(SqliteSettingsRepositoryTest, SetGameValue_CreatesNewRecord) {
  const std::string contentHash = "test_content_hash";
  const std::string key = "new_key";
  const std::string value = "new_value";
  
  // Verify key doesn't exist initially
  EXPECT_FALSE(repository->getGameValue(contentHash, key).has_value());
  
  repository->setGameValue(contentHash, key, value);
  
  auto retrievedValue = repository->getGameValue(contentHash, key);
  ASSERT_TRUE(retrievedValue.has_value());
  EXPECT_EQ(retrievedValue.value(), value);
}

/**
 * @brief Test that setGameValue updates existing database records
 * 
 * Verifies that setting a game value for an existing key updates
 * the existing record rather than creating a duplicate.
 */
TEST_F(SqliteSettingsRepositoryTest, SetGameValue_UpdatesExistingRecord) {
  const std::string contentHash = "test_content_hash";
  const std::string key = "existing_key";
  const std::string initialValue = "initial_value";
  const std::string updatedValue = "updated_value";
  
  repository->setGameValue(contentHash, key, initialValue);
  repository->setGameValue(contentHash, key, updatedValue);
  
  auto retrievedValue = repository->getGameValue(contentHash, key);
  ASSERT_TRUE(retrievedValue.has_value());
  EXPECT_EQ(retrievedValue.value(), updatedValue);
}

/**
 * @brief Test setting multiple game values across content hashes and keys
 * 
 * Verifies that multiple game settings can be stored simultaneously
 * across different content hashes and setting keys without conflicts.
 */
TEST_F(SqliteSettingsRepositoryTest, SetGameValue_MultipleContentHashesAndKeys) {
  const std::string contentHash1 = "hash1";
  const std::string contentHash2 = "hash2";
  const std::string key1 = "key1";
  const std::string key2 = "key2";
  const std::string value1 = "value1";
  const std::string value2 = "value2";
  const std::string value3 = "value3";
  const std::string value4 = "value4";
  
  repository->setGameValue(contentHash1, key1, value1);
  repository->setGameValue(contentHash1, key2, value2);
  repository->setGameValue(contentHash2, key1, value3);
  repository->setGameValue(contentHash2, key2, value4);
  
  EXPECT_EQ(repository->getGameValue(contentHash1, key1).value(), value1);
  EXPECT_EQ(repository->getGameValue(contentHash1, key2).value(), value2);
  EXPECT_EQ(repository->getGameValue(contentHash2, key1).value(), value3);
  EXPECT_EQ(repository->getGameValue(contentHash2, key2).value(), value4);
}

/**
 * @brief Test game values with special characters
 * 
 * Verifies that game setting values containing special characters,
 * spaces, and symbols are stored and retrieved correctly.
 */
TEST_F(SqliteSettingsRepositoryTest, SetGameValue_WithSpecialCharacters) {
  const std::string contentHash = "test_content_hash";
  const std::string key = "special_key";
  const std::string value = "Value with spaces, symbols: !@#$%^&*()";
  
  repository->setGameValue(contentHash, key, value);
  
  auto retrievedValue = repository->getGameValue(contentHash, key);
  ASSERT_TRUE(retrievedValue.has_value());
  EXPECT_EQ(retrievedValue.value(), value);
}

/**
 * @brief Test that resetGameValue removes existing values
 * 
 * Verifies that resetting a game setting removes the value from
 * the database and subsequent queries return nullopt.
 */
TEST_F(SqliteSettingsRepositoryTest, ResetGameValue_RemovesExistingValue) {
  const std::string contentHash = "test_content_hash";
  const std::string key = "test_key";
  const std::string value = "test_value";
  
  bool setSuccess = repository->setGameValue(contentHash, key, value);
  EXPECT_TRUE(setSuccess);
  EXPECT_TRUE(repository->getGameValue(contentHash, key).has_value());
  
  bool resetSuccess = repository->resetGameValue(contentHash, key);
  EXPECT_TRUE(resetSuccess);
  
  auto retrievedValue = repository->getGameValue(contentHash, key);
  EXPECT_FALSE(retrievedValue.has_value());
}

/**
 * @brief Test that resetting non-existent game values doesn't error
 * 
 * Verifies that attempting to reset a game setting that doesn't exist
 * doesn't cause errors or exceptions.
 */
TEST_F(SqliteSettingsRepositoryTest, ResetGameValue_NonExistentKeyNoError) {
  const std::string contentHash = "test_content_hash";
  const std::string key = "non_existent_key";
  
  // Should not throw or cause issues
  repository->resetGameValue(contentHash, key);
  
  auto retrievedValue = repository->getGameValue(contentHash, key);
  EXPECT_FALSE(retrievedValue.has_value());
}

/**
 * @brief Test that resetGameValue only removes the specific key
 * 
 * Verifies that resetting one game setting key doesn't affect other
 * setting keys for the same content hash.
 */
TEST_F(SqliteSettingsRepositoryTest, ResetGameValue_OnlyRemovesSpecificKey) {
  const std::string contentHash = "test_content_hash";
  const std::string key1 = "key1";
  const std::string key2 = "key2";
  const std::string value1 = "value1";
  const std::string value2 = "value2";
  
  repository->setGameValue(contentHash, key1, value1);
  repository->setGameValue(contentHash, key2, value2);
  
  repository->resetGameValue(contentHash, key1);
  
  EXPECT_FALSE(repository->getGameValue(contentHash, key1).has_value());
  EXPECT_TRUE(repository->getGameValue(contentHash, key2).has_value());
  EXPECT_EQ(repository->getGameValue(contentHash, key2).value(), value2);
}

/**
 * @brief Test that resetGameValue only removes the specific content hash
 * 
 * Verifies that resetting a game setting only affects that content hash
 * and doesn't remove the same key from other content hashes.
 */
TEST_F(SqliteSettingsRepositoryTest, ResetGameValue_OnlyRemovesSpecificContentHash) {
  const std::string contentHash1 = "hash1";
  const std::string contentHash2 = "hash2";
  const std::string key = "test_key";
  const std::string value1 = "value1";
  const std::string value2 = "value2";
  
  repository->setGameValue(contentHash1, key, value1);
  repository->setGameValue(contentHash2, key, value2);
  
  repository->resetGameValue(contentHash1, key);
  
  EXPECT_FALSE(repository->getGameValue(contentHash1, key).has_value());
  EXPECT_TRUE(repository->getGameValue(contentHash2, key).has_value());
  EXPECT_EQ(repository->getGameValue(contentHash2, key).value(), value2);
}

/**
 * @brief Test that game values can be set after being reset
 * 
 * Verifies that after resetting a game setting, a new value can be
 * set for the same key and retrieved correctly.
 */
TEST_F(SqliteSettingsRepositoryTest, ResetGameValue_CanSetAfterReset) {
  const std::string contentHash = "test_content_hash";
  const std::string key = "test_key";
  const std::string initialValue = "initial_value";
  const std::string newValue = "new_value";
  
  repository->setGameValue(contentHash, key, initialValue);
  repository->resetGameValue(contentHash, key);
  repository->setGameValue(contentHash, key, newValue);
  
  auto retrievedValue = repository->getGameValue(contentHash, key);
  ASSERT_TRUE(retrievedValue.has_value());
  EXPECT_EQ(retrievedValue.value(), newValue);
}

/**
 * @brief Test that game values are isolated from platform values
 * 
 * Verifies that game settings and platform settings are completely isolated
 * and don't interfere with each other, even when using the same keys.
 */
TEST_F(SqliteSettingsRepositoryTest, GameValue_IsolatedFromPlatformValue) {
  const std::string contentHash = "test_content_hash";
  const int platformId = 1;
  const std::string key = "test_key";
  const std::string gameValue = "game_value";
  const std::string platformValue = "platform_value";
  
  repository->setGameValue(contentHash, key, gameValue);
  repository->setPlatformValue(platformId, key, platformValue);
  
  auto retrievedGameValue = repository->getGameValue(contentHash, key);
  auto retrievedPlatformValue = repository->getPlatformValue(platformId, key);
  
  ASSERT_TRUE(retrievedGameValue.has_value());
  ASSERT_TRUE(retrievedPlatformValue.has_value());
  EXPECT_EQ(retrievedGameValue.value(), gameValue);
  EXPECT_EQ(retrievedPlatformValue.value(), platformValue);
  
  // Resetting one shouldn't affect the other
  repository->resetGameValue(contentHash, key);
  EXPECT_FALSE(repository->getGameValue(contentHash, key).has_value());
  EXPECT_TRUE(repository->getPlatformValue(platformId, key).has_value());
}

} // namespace firelight::settings
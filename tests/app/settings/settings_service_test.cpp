#include "../../../src/app/settings/settings_service.hpp"
#include "../../../src/app/settings/sqlite_settings_repository.hpp"
#include "event_dispatcher.hpp"
#include <filesystem>
#include <gtest/gtest.h>

namespace firelight::settings {

class SettingsServiceTest : public testing::Test {
protected:
  std::unique_ptr<SqliteSettingsRepository> repository;
  std::unique_ptr<SettingsService> service;

  // Event handlers
  ScopedConnection settingsLevelChangedHandler;
  ScopedConnection platformSettingChangedHandler;
  ScopedConnection platformSettingResetHandler;
  ScopedConnection gameSettingChangedHandler;
  ScopedConnection gameSettingResetHandler;

  // Event storage
  std::vector<SettingsLevelChangedEvent> settingsLevelChangedEvents;
  std::vector<PlatformSettingChangedEvent> platformSettingChangedEvents;
  std::vector<PlatformSettingResetEvent> platformSettingResetEvents;
  std::vector<GameSettingChangedEvent> gameSettingChangedEvents;
  std::vector<GameSettingResetEvent> gameSettingResetEvents;

  void SetUp() override {
    repository = std::make_unique<SqliteSettingsRepository>(":memory:");
    service = std::make_unique<SettingsService>(*repository);

    // Subscribe to all events
    settingsLevelChangedHandler =
        EventDispatcher::instance().subscribe<SettingsLevelChangedEvent>(
            [this](const SettingsLevelChangedEvent &event) {
              settingsLevelChangedEvents.push_back(event);
            });

    platformSettingChangedHandler =
        EventDispatcher::instance().subscribe<PlatformSettingChangedEvent>(
            [this](const PlatformSettingChangedEvent &event) {
              platformSettingChangedEvents.push_back(event);
            });

    platformSettingResetHandler =
        EventDispatcher::instance().subscribe<PlatformSettingResetEvent>(
            [this](const PlatformSettingResetEvent &event) {
              platformSettingResetEvents.push_back(event);
            });

    gameSettingChangedHandler =
        EventDispatcher::instance().subscribe<GameSettingChangedEvent>(
            [this](const GameSettingChangedEvent &event) {
              gameSettingChangedEvents.push_back(event);
            });

    gameSettingResetHandler =
        EventDispatcher::instance().subscribe<GameSettingResetEvent>(
            [this](const GameSettingResetEvent &event) {
              gameSettingResetEvents.push_back(event);
            });
  }

  void TearDown() override {
    service.reset();
    repository.reset();

    // Clear event vectors
    settingsLevelChangedEvents.clear();
    platformSettingChangedEvents.clear();
    platformSettingResetEvents.clear();
    gameSettingChangedEvents.clear();
    gameSettingResetEvents.clear();
  }
};

// Settings Level Tests
TEST_F(SettingsServiceTest, GetSettingsLevel_DefaultsToPlatform) {
  const std::string contentHash = "test_content_hash";

  SettingsLevel level = service->getSettingsLevel(contentHash);

  EXPECT_EQ(level, SettingsLevel::Platform);
}

TEST_F(SettingsServiceTest, SetSettingsLevel_Success_PublishesEvent) {
  const std::string contentHash = "test_content_hash";
  const SettingsLevel level = Game;

  bool success = service->setSettingsLevel(contentHash, level);

  EXPECT_TRUE(success);
  EXPECT_EQ(service->getSettingsLevel(contentHash), level);

  // Verify event was published
  ASSERT_EQ(settingsLevelChangedEvents.size(), 1);
  EXPECT_EQ(settingsLevelChangedEvents[0].contentHash, contentHash);
  EXPECT_EQ(settingsLevelChangedEvents[0].level, level);
}

TEST_F(SettingsServiceTest,
       SetSettingsLevel_MultipleChanges_PublishesMultipleEvents) {
  const std::string contentHash = "test_content_hash";

  service->setSettingsLevel(contentHash, Game);
  service->setSettingsLevel(contentHash, Platform);
  service->setSettingsLevel(contentHash, Game);

  ASSERT_EQ(settingsLevelChangedEvents.size(), 3);
  EXPECT_EQ(settingsLevelChangedEvents[0].level, SettingsLevel::Game);
  EXPECT_EQ(settingsLevelChangedEvents[1].level, SettingsLevel::Platform);
  EXPECT_EQ(settingsLevelChangedEvents[2].level, SettingsLevel::Game);
}

// Platform Value Tests
TEST_F(SettingsServiceTest, GetPlatformValue_NonExistentReturnsNullopt) {
  const int platformId = 1;
  const std::string key = "test_key";

  auto value = service->getPlatformValue(platformId, key);

  EXPECT_FALSE(value.has_value());
}

TEST_F(SettingsServiceTest, SetPlatformValue_Success_PublishesEvent) {
  const int platformId = 1;
  const std::string key = "test_key";
  const std::string value = "test_value";

  bool success = service->setPlatformValue(platformId, key, value);

  EXPECT_TRUE(success);

  auto retrievedValue = service->getPlatformValue(platformId, key);
  ASSERT_TRUE(retrievedValue.has_value());
  EXPECT_EQ(retrievedValue.value(), value);

  // Verify event was published
  ASSERT_EQ(platformSettingChangedEvents.size(), 1);
  EXPECT_EQ(platformSettingChangedEvents[0].platformId, platformId);
  EXPECT_EQ(platformSettingChangedEvents[0].key, key);
  EXPECT_EQ(platformSettingChangedEvents[0].value, value);
}

TEST_F(SettingsServiceTest, SetPlatformValue_Update_PublishesNewEvent) {
  const int platformId = 1;
  const std::string key = "test_key";
  const std::string initialValue = "initial_value";
  const std::string updatedValue = "updated_value";

  service->setPlatformValue(platformId, key, initialValue);
  service->setPlatformValue(platformId, key, updatedValue);

  ASSERT_EQ(platformSettingChangedEvents.size(), 2);
  EXPECT_EQ(platformSettingChangedEvents[0].value, initialValue);
  EXPECT_EQ(platformSettingChangedEvents[1].value, updatedValue);
}

TEST_F(SettingsServiceTest, ResetPlatformValue_Success_PublishesEvent) {
  const int platformId = 1;
  const std::string key = "test_key";
  const std::string value = "test_value";

  service->setPlatformValue(platformId, key, value);
  bool resetSuccess = service->resetPlatformValue(platformId, key);

  EXPECT_TRUE(resetSuccess);
  EXPECT_FALSE(service->getPlatformValue(platformId, key).has_value());

  // Verify events were published
  ASSERT_EQ(platformSettingChangedEvents.size(), 1);
  ASSERT_EQ(platformSettingResetEvents.size(), 1);
  EXPECT_EQ(platformSettingResetEvents[0].platformId, platformId);
  EXPECT_EQ(platformSettingResetEvents[0].key, key);
}

TEST_F(SettingsServiceTest, ResetPlatformValue_NonExistent_PublishesEvent) {
  const int platformId = 1;
  const std::string key = "non_existent_key";

  bool resetSuccess = service->resetPlatformValue(platformId, key);

  EXPECT_TRUE(resetSuccess);

  // Should still publish event even if key didn't exist
  ASSERT_EQ(platformSettingResetEvents.size(), 1);
  EXPECT_EQ(platformSettingResetEvents[0].platformId, platformId);
  EXPECT_EQ(platformSettingResetEvents[0].key, key);
}

// Game Value Tests
TEST_F(SettingsServiceTest, GetGameValue_NonExistentReturnsNullopt) {
  const std::string contentHash = "test_content_hash";
  const std::string key = "test_key";

  auto value = service->getGameValue(contentHash, key);

  EXPECT_FALSE(value.has_value());
}

TEST_F(SettingsServiceTest, SetGameValue_Success_PublishesEvent) {
  const std::string contentHash = "test_content_hash";
  const std::string key = "test_key";
  const std::string value = "test_value";

  bool success = service->setGameValue(contentHash, key, value);

  EXPECT_TRUE(success);

  auto retrievedValue = service->getGameValue(contentHash, key);
  ASSERT_TRUE(retrievedValue.has_value());
  EXPECT_EQ(retrievedValue.value(), value);

  // Verify event was published
  ASSERT_EQ(gameSettingChangedEvents.size(), 1);
  EXPECT_EQ(gameSettingChangedEvents[0].contentHash, contentHash);
  EXPECT_EQ(gameSettingChangedEvents[0].key, key);
  EXPECT_EQ(gameSettingChangedEvents[0].value, value);
}

TEST_F(SettingsServiceTest, SetGameValue_Update_PublishesNewEvent) {
  const std::string contentHash = "test_content_hash";
  const std::string key = "test_key";
  const std::string initialValue = "initial_value";
  const std::string updatedValue = "updated_value";

  service->setGameValue(contentHash, key, initialValue);
  service->setGameValue(contentHash, key, updatedValue);

  ASSERT_EQ(gameSettingChangedEvents.size(), 2);
  EXPECT_EQ(gameSettingChangedEvents[0].value, initialValue);
  EXPECT_EQ(gameSettingChangedEvents[1].value, updatedValue);
}

TEST_F(SettingsServiceTest, ResetGameValue_Success_PublishesEvent) {
  const std::string contentHash = "test_content_hash";
  const std::string key = "test_key";
  const std::string value = "test_value";

  service->setGameValue(contentHash, key, value);
  bool resetSuccess = service->resetGameValue(contentHash, key);

  EXPECT_TRUE(resetSuccess);
  EXPECT_FALSE(service->getGameValue(contentHash, key).has_value());

  // Verify events were published
  ASSERT_EQ(gameSettingChangedEvents.size(), 1);
  ASSERT_EQ(gameSettingResetEvents.size(), 1);
  EXPECT_EQ(gameSettingResetEvents[0].contentHash, contentHash);
  EXPECT_EQ(gameSettingResetEvents[0].key, key);
}

TEST_F(SettingsServiceTest, ResetGameValue_NonExistent_PublishesEvent) {
  const std::string contentHash = "test_content_hash";
  const std::string key = "non_existent_key";

  bool resetSuccess = service->resetGameValue(contentHash, key);

  EXPECT_TRUE(resetSuccess);

  // Should still publish event even if key didn't exist
  ASSERT_EQ(gameSettingResetEvents.size(), 1);
  EXPECT_EQ(gameSettingResetEvents[0].contentHash, contentHash);
  EXPECT_EQ(gameSettingResetEvents[0].key, key);
}

// Integration Tests
TEST_F(SettingsServiceTest, MixedOperations_PublishesCorrectEvents) {
  const std::string contentHash = "game_hash";
  const int platformId = 1;
  const std::string key = "setting_key";
  const std::string value = "setting_value";

  // Perform mixed operations
  service->setSettingsLevel(contentHash, SettingsLevel::Game);
  service->setPlatformValue(platformId, key, value);
  service->setGameValue(contentHash, key, value);
  service->resetPlatformValue(platformId, key);
  service->resetGameValue(contentHash, key);

  // Verify all events were published
  EXPECT_EQ(settingsLevelChangedEvents.size(), 1);
  EXPECT_EQ(platformSettingChangedEvents.size(), 1);
  EXPECT_EQ(gameSettingChangedEvents.size(), 1);
  EXPECT_EQ(platformSettingResetEvents.size(), 1);
  EXPECT_EQ(gameSettingResetEvents.size(), 1);

  // Verify event contents
  EXPECT_EQ(settingsLevelChangedEvents[0].contentHash, contentHash);
  EXPECT_EQ(settingsLevelChangedEvents[0].level, SettingsLevel::Game);

  EXPECT_EQ(platformSettingChangedEvents[0].platformId, platformId);
  EXPECT_EQ(platformSettingChangedEvents[0].key, key);
  EXPECT_EQ(platformSettingChangedEvents[0].value, value);

  EXPECT_EQ(gameSettingChangedEvents[0].contentHash, contentHash);
  EXPECT_EQ(gameSettingChangedEvents[0].key, key);
  EXPECT_EQ(gameSettingChangedEvents[0].value, value);

  EXPECT_EQ(platformSettingResetEvents[0].platformId, platformId);
  EXPECT_EQ(platformSettingResetEvents[0].key, key);

  EXPECT_EQ(gameSettingResetEvents[0].contentHash, contentHash);
  EXPECT_EQ(gameSettingResetEvents[0].key, key);
}

TEST_F(SettingsServiceTest, EventsIsolatedBetweenDifferentKeysAndIds) {
  const std::string contentHash1 = "game_hash_1";
  const std::string contentHash2 = "game_hash_2";
  const int platformId1 = 1;
  const int platformId2 = 2;
  const std::string key1 = "key1";
  const std::string key2 = "key2";
  const std::string value = "test_value";

  // Set values for different combinations
  service->setGameValue(contentHash1, key1, value);
  service->setGameValue(contentHash2, key2, value);
  service->setPlatformValue(platformId1, key1, value);
  service->setPlatformValue(platformId2, key2, value);

  // Each operation should generate a separate event
  ASSERT_EQ(gameSettingChangedEvents.size(), 2);
  ASSERT_EQ(platformSettingChangedEvents.size(), 2);

  // Verify event isolation
  EXPECT_NE(gameSettingChangedEvents[0].contentHash,
            gameSettingChangedEvents[1].contentHash);
  EXPECT_NE(gameSettingChangedEvents[0].key, gameSettingChangedEvents[1].key);

  EXPECT_NE(platformSettingChangedEvents[0].platformId,
            platformSettingChangedEvents[1].platformId);
  EXPECT_NE(platformSettingChangedEvents[0].key,
            platformSettingChangedEvents[1].key);
}

} // namespace firelight::settings
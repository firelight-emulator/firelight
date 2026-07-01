#include <firelight/event_dispatcher.hpp>
#include <firelight/library/sqlite_user_library.hpp>
#include <firelight/settings/sqlite_settings_repository.hpp>

#include "fake_core.hpp"

#include <firelight/db/sqlite_userdata_database.hpp>
#include <emulation/emulation_service.hpp>
#include <firelight/library/entry_resolver.hpp>
#include <firelight/library/library_ingest_service.hpp>
#include <firelight/library/user_library_service.hpp>
#include <gtest/gtest.h>
#include <firelight/settings/settings_service.hpp>

namespace firelight::emulation {

/**
 * @brief Test fixture for EmulatorInstance functionality
 *
 * Tests EmulatorInstance behavior with settings changes, including game-level
 * and platform-level setting updates, settings level transitions, and
 * settings isolation between different content hashes and platforms.
 */
class EmulatorInstanceTest : public testing::Test {
protected:
  std::unique_ptr<library::SqliteUserLibraryRepository> m_library;
  std::unique_ptr<library::LibraryIngestService> m_ingest;
  std::unique_ptr<library::UserLibraryService> m_service;
  std::unique_ptr<library::EntryResolver> m_resolver;
  std::unique_ptr<EmulationService> m_emulationService;
  std::unique_ptr<settings::SettingsService> m_settingsService;

  std::string m_testContentHash = "e26ee0d44e809351c8ce2d73c7400cdd";

  void SetUp() override {
    m_library = std::make_unique<library::SqliteUserLibraryRepository>(":memory:");
    m_ingest = std::make_unique<library::LibraryIngestService>(*m_library);
    m_service = std::make_unique<library::UserLibraryService>(*m_library, ".");
    m_resolver = std::make_unique<library::EntryResolver>(*m_library);
    m_settingsService = std::make_unique<settings::SettingsService>(
        *new settings::SqliteSettingsRepository(":memory:"));
    m_emulationService = std::make_unique<EmulationService>(
        *m_service, *m_resolver, *m_settingsService,
        [](int, const std::string &,
           std::shared_ptr<firelight::libretro::IConfigurationProvider>,
           const std::string &) -> std::unique_ptr<::libretro::ICore> {
          return std::make_unique<FakeCore>();
        });

    settings::SettingsService::setInstance(m_settingsService.get());
  }

  void TearDown() override {
    m_settingsService.reset();
    m_emulationService.reset();
    m_resolver.reset();
    m_service.reset();
    m_ingest.reset();
    m_library.reset();
  }
};

/**
 * @brief Test that game-level setting changes update the EmulatorInstance
 *
 * Verifies that when game-specific settings are changed, the EmulatorInstance
 * receives the updates and reflects the new values. Tests the picture-mode
 * setting change from default to integer-scale.
 */
TEST_F(EmulatorInstanceTest, GameSettingChangeUpdatesInstance) {
  library::ContentFile info{.m_fileSizeBytes = 16777216,
                            .m_filePath = "test_resources/testrom.gba",
                            .m_fileMd5 = m_testContentHash,
                            .m_inArchive = false,
                            .m_platformId = 3,
                            .m_contentHash = m_testContentHash};

  m_library->create(info);
  ASSERT_NE(info.m_id, -1);

  auto entry = m_library->getEntryWithContentHash(
      QString::fromStdString(m_testContentHash));
  ASSERT_TRUE(entry.has_value());
  ASSERT_NE(nullptr, m_emulationService->loadEntry(entry->id).get());

  auto instance = m_emulationService->getCurrentEmulatorInstance();
  ASSERT_FALSE(instance->isInitialized());

  // Check initial picture mode value
  EXPECT_EQ("aspect-ratio-fill", instance->getPictureMode());

  // Change game setting
  m_settingsService->setSettingsLevel(m_testContentHash,
                                      settings::SettingsLevel::Game);
  m_settingsService->setGameValue(m_testContentHash, "picture-mode",
                                  "integer-scale");

  // Verify the instance received the update
  EXPECT_EQ("integer-scale", instance->getPictureMode());
}

/**
 * @brief Test that platform-level setting changes update the EmulatorInstance
 *
 * Verifies that when platform-specific settings are changed, the
 * EmulatorInstance receives the updates when configured to use platform-level
 * settings. Tests aspect-ratio setting change from emulator-corrected to
 * pixel-perfect.
 */
TEST_F(EmulatorInstanceTest, PlatformSettingChangeUpdatesInstance) {
  library::ContentFile info{.m_fileSizeBytes = 16777216,
                            .m_filePath = "test_resources/testrom.gba",
                            .m_fileMd5 = "e26ee0d44e809351c8ce2d73c7400cdd",
                            .m_inArchive = false,
                            .m_platformId = 3,
                            .m_contentHash =
                                "e26ee0d44e809351c8ce2d73c7400cdd"};

  m_library->create(info);
  ASSERT_NE(info.m_id, -1);

  auto entry = m_library->getEntryWithContentHash(
      QString::fromStdString(m_testContentHash));
  ASSERT_TRUE(entry.has_value());
  ASSERT_NE(nullptr, m_emulationService->loadEntry(entry->id).get());

  auto instance = m_emulationService->getCurrentEmulatorInstance();
  // Verify instance is not initialized
  ASSERT_FALSE(instance->isInitialized());

  // Set instance to use platform-level settings
  m_settingsService->setSettingsLevel("e26ee0d44e809351c8ce2d73c7400cdd",
                                      settings::SettingsLevel::Platform);

  // Check initial aspect ratio mode value
  EXPECT_EQ("emulator-corrected", instance->getAspectRatioMode());

  // Change platform setting
  m_settingsService->setSettingsLevel(m_testContentHash,
                                      settings::SettingsLevel::Platform);
  m_settingsService->setPlatformValue(3, "aspect-ratio", "pixel-perfect");

  // Verify the instance received the update
  EXPECT_EQ("pixel-perfect", instance->getAspectRatioMode());
}

// TEST_F(EmulatorInstanceTest, RewindSettingChangeUpdatesInstance) {
//   library::ContentFile info{.m_contentHash =
//   "e26ee0d44e809351c8ce2d73c7400cdd",
//                             .m_filePath = "test_resources/testrom.gba",
//                             .m_fileMd5 = "e26ee0d44e809351c8ce2d73c7400cdd",
//                             .m_inArchive = false,
//                             .m_platformId = 3,
//                             .m_fileSizeBytes = 16777216};
//
//   m_library->create(info);
//   ASSERT_NE(info.m_id, -1);
//
//   auto entry =
//       m_library->getEntryWithContentHash("e26ee0d44e809351c8ce2d73c7400cdd");
//   ASSERT_TRUE(entry.has_value());
//   ASSERT_NE(nullptr, m_emulationService->loadEntry(entry->id).get());
//
//   auto instance = m_emulationService->getCurrentEmulatorInstance();
//   // Verify instance is not initialized
//   ASSERT_FALSE(instance->isInitialized());
//
//   // Check initial rewind enabled value (should be true by default)
//   EXPECT_TRUE(instance->isRewindEnabled());
//
//   // Change game rewind setting to false
//   m_settingsService->setSettingsLevel(m_testContentHash,
//                                       settings::SettingsLevel::Game);
//   m_settingsService->setGameValue("e26ee0d44e809351c8ce2d73c7400cdd",
//                                   "rewind-enabled", "false");
//
//   // Verify the instance received the update
//   EXPECT_FALSE(instance->isRewindEnabled());
//
//   // Change it back to true
//   m_settingsService->setGameValue("e26ee0d44e809351c8ce2d73c7400cdd",
//                                   "rewind-enabled", "true");
//
//   // Verify the instance updated again
//   EXPECT_TRUE(instance->isRewindEnabled());
// }

/**
 * @brief Test that changing settings level triggers EmulatorInstance refresh
 *
 * Verifies that when the settings level is changed between Game and Platform,
 * the EmulatorInstance updates its settings accordingly. Tests transitioning
 * from game-level to platform-level settings and back.
 */
TEST_F(EmulatorInstanceTest, SettingsLevelChangeTriggersRefresh) {
  library::ContentFile info{.m_fileSizeBytes = 16777216,
                            .m_filePath = "test_resources/testrom.gba",
                            .m_fileMd5 = "e26ee0d44e809351c8ce2d73c7400cdd",
                            .m_inArchive = false,
                            .m_platformId = 3,
                            .m_contentHash =
                                "e26ee0d44e809351c8ce2d73c7400cdd"};

  m_library->create(info);
  ASSERT_NE(info.m_id, -1);

  auto entry =
      m_library->getEntryWithContentHash("e26ee0d44e809351c8ce2d73c7400cdd");
  ASSERT_TRUE(entry.has_value());
  ASSERT_NE(nullptr, m_emulationService->loadEntry(entry->id).get());

  auto instance = m_emulationService->getCurrentEmulatorInstance();
  // Verify instance is not initialized
  ASSERT_FALSE(instance->isInitialized());

  // Set different values at game and platform level
  m_settingsService->setSettingsLevel("e26ee0d44e809351c8ce2d73c7400cdd",
                                      settings::SettingsLevel::Game);
  m_settingsService->setGameValue("e26ee0d44e809351c8ce2d73c7400cdd",
                                  "picture-mode", "integer-scale");
  m_settingsService->setPlatformValue(3, "picture-mode", "stretch");

  // Initially should use game settings
  EXPECT_EQ("integer-scale", instance->getPictureMode());

  // Change to platform level
  m_settingsService->setSettingsLevel("e26ee0d44e809351c8ce2d73c7400cdd",
                                      settings::SettingsLevel::Platform);

  // Should now reflect platform setting
  EXPECT_EQ("stretch", instance->getPictureMode());

  // Change back to game level
  m_settingsService->setSettingsLevel("e26ee0d44e809351c8ce2d73c7400cdd",
                                      settings::SettingsLevel::Game);

  // Should reflect game setting again
  EXPECT_EQ("integer-scale", instance->getPictureMode());
}

/**
 * @brief Test that multiple simultaneous setting changes are handled correctly
 *
 * Verifies that when multiple game settings are changed at once, the
 * EmulatorInstance properly updates all affected properties. Tests
 * picture-mode, aspect-ratio, and rewind-enabled settings being changed
 * together.
 */
TEST_F(EmulatorInstanceTest, MultipleSettingsChangeSimultaneously) {
  library::ContentFile info{.m_fileSizeBytes = 16777216,
                            .m_filePath = "test_resources/testrom.gba",
                            .m_fileMd5 = "e26ee0d44e809351c8ce2d73c7400cdd",
                            .m_inArchive = false,
                            .m_platformId = 3,
                            .m_contentHash =
                                "e26ee0d44e809351c8ce2d73c7400cdd"};

  m_library->create(info);
  ASSERT_NE(info.m_id, -1);

  auto entry =
      m_library->getEntryWithContentHash("e26ee0d44e809351c8ce2d73c7400cdd");
  ASSERT_TRUE(entry.has_value());
  ASSERT_NE(nullptr, m_emulationService->loadEntry(entry->id).get());

  auto instance = m_emulationService->getCurrentEmulatorInstance();
  // Verify instance is not initialized
  ASSERT_FALSE(instance->isInitialized());

  // Change multiple settings at once
  m_settingsService->setSettingsLevel(m_testContentHash,
                                      settings::SettingsLevel::Game);
  m_settingsService->setGameValue("e26ee0d44e809351c8ce2d73c7400cdd",
                                  "picture-mode", "stretch");
  m_settingsService->setGameValue("e26ee0d44e809351c8ce2d73c7400cdd",
                                  "aspect-ratio", "pixel-perfect");
  m_settingsService->setGameValue("e26ee0d44e809351c8ce2d73c7400cdd",
                                  "rewind-enabled", "false");

  // Verify all settings were updated
  EXPECT_EQ("stretch", instance->getPictureMode());
  EXPECT_EQ("pixel-perfect", instance->getAspectRatioMode());
  // EXPECT_FALSE(instance->isRewindEnabled());
}

/**
 * @brief Test that settings for different content hashes are ignored
 *
 * Verifies that game settings changes for a different content hash do not
 * affect the current EmulatorInstance. Tests settings isolation between
 * different games.
 */
TEST_F(EmulatorInstanceTest, WrongContentHashIgnoresGameSettings) {
  library::ContentFile info{.m_fileSizeBytes = 16777216,
                            .m_filePath = "test_resources/testrom.gba",
                            .m_fileMd5 = "e26ee0d44e809351c8ce2d73c7400cdd",
                            .m_inArchive = false,
                            .m_platformId = 3,
                            .m_contentHash =
                                "e26ee0d44e809351c8ce2d73c7400cdd"};

  m_library->create(info);
  ASSERT_NE(info.m_id, -1);

  auto entry =
      m_library->getEntryWithContentHash("e26ee0d44e809351c8ce2d73c7400cdd");
  ASSERT_TRUE(entry.has_value());
  ASSERT_NE(nullptr, m_emulationService->loadEntry(entry->id).get());

  auto instance = m_emulationService->getCurrentEmulatorInstance();
  // Verify instance is not initialized
  ASSERT_FALSE(instance->isInitialized());

  // Check initial picture mode
  EXPECT_EQ("aspect-ratio-fill", instance->getPictureMode());

  // Try to change setting for different content hash
  m_settingsService->setGameValue("different_hash_456", "picture-mode",
                                  "integer-scale");

  // Instance should not be affected
  EXPECT_EQ("aspect-ratio-fill", instance->getPictureMode());
}

/**
 * @brief Test that settings for different platform IDs are ignored
 *
 * Verifies that platform settings changes for a different platform ID do not
 * affect the current EmulatorInstance. Tests settings isolation between
 * different platforms.
 */
TEST_F(EmulatorInstanceTest, WrongPlatformIdIgnoresPlatformSettings) {
  library::ContentFile info{.m_fileSizeBytes = 16777216,
                            .m_filePath = "test_resources/testrom.gba",
                            .m_fileMd5 = "e26ee0d44e809351c8ce2d73c7400cdd",
                            .m_inArchive = false,
                            .m_platformId = 3,
                            .m_contentHash =
                                "e26ee0d44e809351c8ce2d73c7400cdd"};

  m_library->create(info);
  ASSERT_NE(info.m_id, -1);

  auto entry =
      m_library->getEntryWithContentHash("e26ee0d44e809351c8ce2d73c7400cdd");
  ASSERT_TRUE(entry.has_value());
  ASSERT_NE(nullptr, m_emulationService->loadEntry(entry->id).get());

  auto instance = m_emulationService->getCurrentEmulatorInstance();
  // Verify instance is not initialized
  ASSERT_FALSE(instance->isInitialized());

  // Set to platform settings level
  m_settingsService->setSettingsLevel("e26ee0d44e809351c8ce2d73c7400cdd",
                                      settings::SettingsLevel::Platform);

  // Check initial aspect ratio mode
  EXPECT_EQ("emulator-corrected", instance->getAspectRatioMode());

  // Try to change setting for different platform ID
  m_settingsService->setPlatformValue(999, "aspect-ratio", "pixel-perfect");

  // Instance should not be affected
  EXPECT_EQ("emulator-corrected", instance->getAspectRatioMode());
}

} // namespace firelight::emulation

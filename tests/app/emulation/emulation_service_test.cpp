#include "../../mocks/mock_library.hpp"
#include "event_dispatcher.hpp"
#include "library/sqlite_user_library.hpp"

#include <db/sqlite_userdata_database.hpp>
#include <emulation/emulation_service.hpp>
#include <gmock/gmock-function-mocker.h>
#include <gtest/gtest.h>
#include <settings/settings_service.hpp>
#include <settings/sqlite_settings_repository.hpp>

namespace firelight::emulation {

/**
 * @brief Test fixture for EmulationService functionality
 * 
 * Tests the core emulation service operations including ROM loading,
 * archive extraction, and emulator instance management.
 */
class EmulationServiceTest : public testing::Test {
protected:
  std::unique_ptr<library::SqliteUserLibrary> m_library;
  std::unique_ptr<EmulationService> m_emulationService;
  std::unique_ptr<settings::SettingsService> m_settingsService;

  std::string m_testContentHash = "e26ee0d44e809351c8ce2d73c7400cdd";

  void SetUp() override {
    m_library = std::make_unique<library::SqliteUserLibrary>(":memory:", ".");
    m_emulationService = std::make_unique<EmulationService>(*m_library);

    m_settingsService = std::make_unique<settings::SettingsService>(
        *new settings::SqliteSettingsRepository(":memory:"));
    settings::SettingsService::setInstance(m_settingsService.get());
  }

  void TearDown() override {
    m_settingsService.reset();
    m_emulationService.reset();
    m_library.reset();
  }
};

/**
 * @brief Test that loading a non-existent entry fails gracefully
 * 
 * Verifies that attempting to load an entry that doesn't exist in the library
 * returns nullptr and triggers a GameLoadFailedEvent.
 */
TEST_F(EmulationServiceTest, LoadWithNoEntryFails) {
  bool gameLoadFailedEventReceived = false;
  ScopedConnection loadFailedConnection =
      EventDispatcher::instance().subscribe<GameLoadFailedEvent>(
          [&gameLoadFailedEventReceived](const GameLoadFailedEvent &event) {
            gameLoadFailedEventReceived = true;
          });

  library::SqliteUserLibrary library(":memory:", ".");
  EmulationService service(library);

  ASSERT_EQ(nullptr, service.loadEntry(1).get());
  ASSERT_TRUE(gameLoadFailedEventReceived);
}

/**
 * @brief Test successful loading of a valid ROM file
 * 
 * Verifies that a valid ROM file can be loaded, creates an EmulatorInstance,
 * and triggers a GameLoadedEvent. Tests that the instance has correct metadata
 * including content hash and platform ID.
 */
TEST_F(EmulationServiceTest, LoadValidRomSucceeds) {
  bool gameLoadedEventReceived = false;
  ScopedConnection loadedConnection =
      EventDispatcher::instance().subscribe<GameLoadedEvent>(
          [&gameLoadedEventReceived](const GameLoadedEvent &event) {
            gameLoadedEventReceived = true;
          });

  library::RomFileInfo info{.m_fileSizeBytes = 16777216,
                            .m_filePath = "test_resources/testrom.gba",
                            .m_fileMd5 = "e26ee0d44e809351c8ce2d73c7400cdd",
                            .m_inArchive = false,
                            .m_platformId = 3,
                            .m_contentHash = "e26ee0d44e809351c8ce2d73c7400cdd"};

  m_library->create(info);
  ASSERT_NE(info.m_id, -1);

  auto entry =
      m_library->getEntryWithContentHash("e26ee0d44e809351c8ce2d73c7400cdd");
  ASSERT_TRUE(entry.has_value());

  ASSERT_NE(nullptr, m_emulationService->loadEntry(entry->id).get());
  ASSERT_TRUE(gameLoadedEventReceived);

  auto currentEmulatorInstance =
      m_emulationService->getCurrentEmulatorInstance();
  ASSERT_FALSE(currentEmulatorInstance->isInitialized());
  ASSERT_EQ("e26ee0d44e809351c8ce2d73c7400cdd",
            currentEmulatorInstance->getContentHash());
  ASSERT_EQ(3, currentEmulatorInstance->getPlatformId());
}

/**
 * @brief Test successful loading of a ROM file from a ZIP archive
 * 
 * Verifies that ROM files stored in ZIP archives can be properly extracted
 * and loaded. Tests archive extraction functionality and ensures the
 * EmulatorInstance is created with correct metadata.
 */
TEST_F(EmulationServiceTest, LoadValidRomInZipSucceeds) {
  bool gameLoadedEventReceived = false;
  ScopedConnection loadedConnection =
      EventDispatcher::instance().subscribe<GameLoadedEvent>(
          [&gameLoadedEventReceived](const GameLoadedEvent &event) {
            gameLoadedEventReceived = true;
          });

  library::RomFileInfo info{.m_fileSizeBytes = 0,
                            .m_filePath = "testrom.gba",
                            .m_fileMd5 = "e26ee0d44e809351c8ce2d73c7400cdd",
                            .m_inArchive = true,
                            .m_archivePathName =
                                "test_resources/testrom.gba.zip",
                            .m_platformId = 3,
                            .m_contentHash = "e26ee0d44e809351c8ce2d73c7400cdd"};

  m_library->create(info);
  ASSERT_NE(info.m_id, -1);

  auto entry =
      m_library->getEntryWithContentHash("e26ee0d44e809351c8ce2d73c7400cdd");
  ASSERT_TRUE(entry.has_value());

  ASSERT_NE(nullptr, m_emulationService->loadEntry(entry->id).get());
  ASSERT_TRUE(gameLoadedEventReceived);

  auto currentEmulatorInstance =
      m_emulationService->getCurrentEmulatorInstance();
  ASSERT_FALSE(currentEmulatorInstance->isInitialized());
  ASSERT_EQ("e26ee0d44e809351c8ce2d73c7400cdd",
            currentEmulatorInstance->getContentHash());
  ASSERT_EQ(3, currentEmulatorInstance->getPlatformId());
}

/**
 * @brief Test successful loading of a ROM file from a 7Z archive
 * 
 * Verifies that ROM files stored in 7Z archives can be properly extracted
 * and loaded. Tests 7-Zip archive format support and ensures proper
 * EmulatorInstance creation.
 */
TEST_F(EmulationServiceTest, LoadValidRomIn7ZSucceeds) {
  bool gameLoadedEventReceived = false;
  ScopedConnection loadedConnection =
      EventDispatcher::instance().subscribe<GameLoadedEvent>(
          [&gameLoadedEventReceived](const GameLoadedEvent &event) {
            gameLoadedEventReceived = true;
          });

  library::RomFileInfo info{.m_fileSizeBytes = 0,
                            .m_filePath = "testrom.gba",
                            .m_fileMd5 = "e26ee0d44e809351c8ce2d73c7400cdd",
                            .m_inArchive = true,
                            .m_archivePathName =
                                "test_resources/testrom.gba.7z",
                            .m_platformId = 3,
                            .m_contentHash = "e26ee0d44e809351c8ce2d73c7400cdd"};

  m_library->create(info);
  ASSERT_NE(info.m_id, -1);

  auto entry =
      m_library->getEntryWithContentHash("e26ee0d44e809351c8ce2d73c7400cdd");
  ASSERT_TRUE(entry.has_value());

  ASSERT_NE(nullptr, m_emulationService->loadEntry(entry->id).get());
  ASSERT_TRUE(gameLoadedEventReceived);

  auto currentEmulatorInstance =
      m_emulationService->getCurrentEmulatorInstance();
  ASSERT_FALSE(currentEmulatorInstance->isInitialized());
  ASSERT_EQ("e26ee0d44e809351c8ce2d73c7400cdd",
            currentEmulatorInstance->getContentHash());
  ASSERT_EQ(3, currentEmulatorInstance->getPlatformId());
}

/**
 * @brief Test successful loading of a ROM file from a TAR archive
 * 
 * Verifies that ROM files stored in TAR archives can be properly extracted
 * and loaded. Tests TAR archive format support and validates that the
 * resulting EmulatorInstance has correct properties.
 */
TEST_F(EmulationServiceTest, LoadValidRomInTarSucceeds) {
  bool gameLoadedEventReceived = false;
  ScopedConnection loadedConnection =
      EventDispatcher::instance().subscribe<GameLoadedEvent>(
          [&gameLoadedEventReceived](const GameLoadedEvent &event) {
            gameLoadedEventReceived = true;
          });

  library::RomFileInfo info{.m_fileSizeBytes = 0,
                            .m_filePath = "testrom.gba",
                            .m_fileMd5 = "e26ee0d44e809351c8ce2d73c7400cdd",
                            .m_inArchive = true,
                            .m_archivePathName =
                                "test_resources/testrom.gba.tar",
                            .m_platformId = 3,
                            .m_contentHash = "e26ee0d44e809351c8ce2d73c7400cdd"};

  m_library->create(info);
  ASSERT_NE(info.m_id, -1);

  auto entry =
      m_library->getEntryWithContentHash("e26ee0d44e809351c8ce2d73c7400cdd");
  ASSERT_TRUE(entry.has_value());

  ASSERT_NE(nullptr, m_emulationService->loadEntry(entry->id).get());
  ASSERT_TRUE(gameLoadedEventReceived);

  auto currentEmulatorInstance =
      m_emulationService->getCurrentEmulatorInstance();
  ASSERT_FALSE(currentEmulatorInstance->isInitialized());
  ASSERT_EQ("e26ee0d44e809351c8ce2d73c7400cdd",
            currentEmulatorInstance->getContentHash());
  ASSERT_EQ(3, currentEmulatorInstance->getPlatformId());
}

} // namespace firelight::emulation

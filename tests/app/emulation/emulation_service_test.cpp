#include "../../mocks/mock_library.hpp"
#include "event_dispatcher.hpp"
#include "library/sqlite_user_library.hpp"

#include <db/sqlite_userdata_database.hpp>
#include <emulation/emulation_service.hpp>
#include <gmock/gmock-function-mocker.h>
#include <gtest/gtest.h>

namespace firelight::emulation {

class EmulationServiceTest : public testing::Test {
protected:
  void SetUp() override {}

  void TearDown() override {}
};

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

TEST_F(EmulationServiceTest, LoadValidRomSucceeds) {
  bool gameLoadedEventReceived = false;
  ScopedConnection loadedConnection =
      EventDispatcher::instance().subscribe<GameLoadedEvent>(
          [&gameLoadedEventReceived](const GameLoadedEvent &event) {
            gameLoadedEventReceived = true;
          });

  library::SqliteUserLibrary library(":memory:", ".");
  library::RomFileInfo info{.m_contentHash = "e26ee0d44e809351c8ce2d73c7400cdd",
                            .m_filePath = "test_resources/testrom.gba",
                            .m_fileMd5 = "e26ee0d44e809351c8ce2d73c7400cdd",
                            .m_inArchive = false,
                            .m_platformId = 3,
                            .m_fileSizeBytes = 16777216};

  library.create(info);
  ASSERT_NE(info.m_id, -1);

  auto entry =
      library.getEntryWithContentHash("e26ee0d44e809351c8ce2d73c7400cdd");
  ASSERT_TRUE(entry.has_value());

  EmulationService service(library);

  ASSERT_NE(nullptr, service.loadEntry(entry->id).get());
  ASSERT_TRUE(gameLoadedEventReceived);

  auto currentEmulatorInstance = service.getCurrentEmulatorInstance();
  ASSERT_FALSE(currentEmulatorInstance->isInitialized());
  ASSERT_EQ("e26ee0d44e809351c8ce2d73c7400cdd",
            currentEmulatorInstance->getContentHash());
  ASSERT_EQ(3, currentEmulatorInstance->getPlatformId());
}

TEST_F(EmulationServiceTest, LoadValidRomInZipSucceeds) {
  bool gameLoadedEventReceived = false;
  ScopedConnection loadedConnection =
      EventDispatcher::instance().subscribe<GameLoadedEvent>(
          [&gameLoadedEventReceived](const GameLoadedEvent &event) {
            gameLoadedEventReceived = true;
          });

  library::SqliteUserLibrary library(":memory:", ".");
  library::RomFileInfo info{.m_contentHash = "e26ee0d44e809351c8ce2d73c7400cdd",
                            .m_filePath = "testrom.gba",
                            .m_fileMd5 = "e26ee0d44e809351c8ce2d73c7400cdd",
                            .m_inArchive = true,
                            .m_archivePathName =
                                "test_resources/testrom.gba.zip",
                            .m_platformId = 3,
                            .m_fileSizeBytes = 0};

  library.create(info);
  ASSERT_NE(info.m_id, -1);

  auto entry =
      library.getEntryWithContentHash("e26ee0d44e809351c8ce2d73c7400cdd");
  ASSERT_TRUE(entry.has_value());

  EmulationService service(library);

  ASSERT_NE(nullptr, service.loadEntry(entry->id).get());
  ASSERT_TRUE(gameLoadedEventReceived);

  auto currentEmulatorInstance = service.getCurrentEmulatorInstance();
  ASSERT_FALSE(currentEmulatorInstance->isInitialized());
  ASSERT_EQ("e26ee0d44e809351c8ce2d73c7400cdd",
            currentEmulatorInstance->getContentHash());
  ASSERT_EQ(3, currentEmulatorInstance->getPlatformId());
}

TEST_F(EmulationServiceTest, LoadValidRomIn7ZSucceeds) {
  bool gameLoadedEventReceived = false;
  ScopedConnection loadedConnection =
      EventDispatcher::instance().subscribe<GameLoadedEvent>(
          [&gameLoadedEventReceived](const GameLoadedEvent &event) {
            gameLoadedEventReceived = true;
          });

  library::SqliteUserLibrary library(":memory:", ".");
  library::RomFileInfo info{.m_contentHash = "e26ee0d44e809351c8ce2d73c7400cdd",
                            .m_filePath = "testrom.gba",
                            .m_fileMd5 = "e26ee0d44e809351c8ce2d73c7400cdd",
                            .m_inArchive = true,
                            .m_archivePathName =
                                "test_resources/testrom.gba.7z",
                            .m_platformId = 3,
                            .m_fileSizeBytes = 0};

  library.create(info);
  ASSERT_NE(info.m_id, -1);

  auto entry =
      library.getEntryWithContentHash("e26ee0d44e809351c8ce2d73c7400cdd");
  ASSERT_TRUE(entry.has_value());

  EmulationService service(library);

  ASSERT_NE(nullptr, service.loadEntry(entry->id).get());
  ASSERT_TRUE(gameLoadedEventReceived);

  auto currentEmulatorInstance = service.getCurrentEmulatorInstance();
  ASSERT_FALSE(currentEmulatorInstance->isInitialized());
  ASSERT_EQ("e26ee0d44e809351c8ce2d73c7400cdd",
            currentEmulatorInstance->getContentHash());
  ASSERT_EQ(3, currentEmulatorInstance->getPlatformId());
}

TEST_F(EmulationServiceTest, LoadValidRomInTarSucceeds) {
  bool gameLoadedEventReceived = false;
  ScopedConnection loadedConnection =
      EventDispatcher::instance().subscribe<GameLoadedEvent>(
          [&gameLoadedEventReceived](const GameLoadedEvent &event) {
            gameLoadedEventReceived = true;
          });

  library::SqliteUserLibrary library(":memory:", ".");
  library::RomFileInfo info{.m_contentHash = "e26ee0d44e809351c8ce2d73c7400cdd",
                            .m_filePath = "testrom.gba",
                            .m_fileMd5 = "e26ee0d44e809351c8ce2d73c7400cdd",
                            .m_inArchive = true,
                            .m_archivePathName =
                                "test_resources/testrom.gba.tar",
                            .m_platformId = 3,
                            .m_fileSizeBytes = 0};

  library.create(info);
  ASSERT_NE(info.m_id, -1);

  auto entry =
      library.getEntryWithContentHash("e26ee0d44e809351c8ce2d73c7400cdd");
  ASSERT_TRUE(entry.has_value());

  EmulationService service(library);

  ASSERT_NE(nullptr, service.loadEntry(entry->id).get());
  ASSERT_TRUE(gameLoadedEventReceived);

  auto currentEmulatorInstance = service.getCurrentEmulatorInstance();
  ASSERT_FALSE(currentEmulatorInstance->isInitialized());
  ASSERT_EQ("e26ee0d44e809351c8ce2d73c7400cdd",
            currentEmulatorInstance->getContentHash());
  ASSERT_EQ(3, currentEmulatorInstance->getPlatformId());
}

} // namespace firelight::emulation
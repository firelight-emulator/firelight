#include "fake_core.hpp"

#include <firelight/event_dispatcher.hpp>
#include <firelight/library/disc_set_service.hpp>
#include <firelight/library/entry_resolver.hpp>
#include <firelight/library/library_ingest_service.hpp>
#include <firelight/library/sqlite_user_library.hpp>
#include <firelight/library/user_library_service.hpp>
#include <firelight/settings/settings_service.hpp>
#include <firelight/settings/sqlite_settings_repository.hpp>

#include <emulation/emulation_service.hpp>
#include <gtest/gtest.h>

namespace firelight::emulation {

/**
 * @brief Test fixture for EmulationService functionality
 *
 * Tests the core emulation service operations including ROM loading, archive extraction, and emulator instance management
 */
class EmulationServiceTest : public testing::Test {
protected:
  std::unique_ptr<library::SqliteUserLibraryRepository> m_library;
  std::unique_ptr<library::DiscSetService> m_discSets;
  std::unique_ptr<library::LibraryIngestService> m_ingest;
  std::unique_ptr<library::UserLibraryService> m_service;
  std::unique_ptr<library::EntryResolver> m_resolver;
  std::unique_ptr<EmulationService> m_emulationService;
  std::unique_ptr<settings::SettingsService> m_settingsService;

  std::string m_testContentHash = "e26ee0d44e809351c8ce2d73c7400cdd";

  void SetUp() override {
    m_library = std::make_unique<library::SqliteUserLibraryRepository>(":memory:");
    m_discSets = std::make_unique<library::DiscSetService>(*m_library, "");
    m_ingest = std::make_unique<library::LibraryIngestService>(*m_library, *m_discSets);
    m_service = std::make_unique<library::UserLibraryService>(*m_library, ".");
    m_resolver = std::make_unique<library::EntryResolver>(*m_library, "");
    m_settingsService =
        std::make_unique<settings::SettingsService>(*new settings::SqliteSettingsRepository(":memory:"));
    settings::SettingsService::setInstance(m_settingsService.get());
    m_emulationService = std::make_unique<EmulationService>(
        *m_service, *m_resolver, *m_settingsService, EmulationContext{},
        [](const firelight::libretro::CoreRunConfig &) -> std::unique_ptr<::libretro::ICore> {
          return std::make_unique<FakeCore>();
        });
  }

  void TearDown() override {
    m_settingsService.reset();
    m_emulationService.reset();
    m_resolver.reset();
    m_service.reset();
    m_ingest.reset();
    m_library.reset();
  }

  /**
   * Adds the GBA test ROM to the library
   *
   * @return Its entry id, or -1
   */
  int createTestEntry() {
    library::ContentFile info{.m_fileSizeBytes = 16777216,
                              .m_filePath = "test_resources/testrom.gba",
                              .m_fileMd5 = m_testContentHash,
                              .m_inArchive = false,
                              .m_platformId = 3,
                              .m_contentHash = m_testContentHash};
    m_library->create(info);

    const auto entry = m_library->getEntryWithContentHash(m_testContentHash);
    return entry.has_value() ? entry->id : -1;
  }

  /**
   * Loads the GBA test ROM
   *
   * @return Its entry id, or -1 when the load produced no instance
   */
  int loadTestRom() {
    const auto entryId = createTestEntry();

    if (entryId < 0 || m_emulationService->loadEntry(entryId).get() == nullptr) {
      return -1;
    }

    return entryId;
  }
};

/**
 * @brief Test that loading a non-existent entry fails gracefully
 *
 * Verifies that attempting to load an entry that doesn't exist in the library returns nullptr and triggers a
 * GameLoadFailedEvent
 */
TEST_F(EmulationServiceTest, LoadWithNoEntryFails) {
  bool gameLoadFailedEventReceived = false;
  ScopedConnection loadFailedConnection = EventDispatcher::instance().subscribe<GameLoadFailedEvent>(
      [&gameLoadFailedEventReceived](const GameLoadFailedEvent &event) { gameLoadFailedEventReceived = true; });

  library::SqliteUserLibraryRepository library(":memory:");
  library::UserLibraryService libraryService(library, ".");
  library::EntryResolver resolver(library, "");
  EmulationService service(libraryService, resolver, *m_settingsService, EmulationContext{});

  ASSERT_EQ(nullptr, service.loadEntry(1).get());
  ASSERT_TRUE(gameLoadFailedEventReceived);
}

/**
 * @brief Test that an entry whose content file is missing fails gracefully
 *
 * The entry resolves from the library, but its on-disk content path does not exist. loadEntry must return a ready
 * future holding nullptr (never an invalid future) and publish a GameLoadFailedEvent, without attempting to load a core
 */
TEST_F(EmulationServiceTest, LoadWithMissingContentPathFails) {
  bool gameLoadFailedEventReceived = false;
  ScopedConnection loadFailedConnection = EventDispatcher::instance().subscribe<GameLoadFailedEvent>(
      [&gameLoadFailedEventReceived](const GameLoadFailedEvent &) { gameLoadFailedEventReceived = true; });

  library::ContentFile info{.m_fileSizeBytes = 16777216,
                            .m_filePath = "test_resources/does_not_exist.gba",
                            .m_fileMd5 = "deadbeefdeadbeefdeadbeefdeadbeef",
                            .m_inArchive = false,
                            .m_platformId = 3,
                            .m_contentHash = "deadbeefdeadbeefdeadbeefdeadbeef"};
  m_library->create(info);
  ASSERT_NE(info.m_id, -1);

  auto entry = m_library->getEntryWithContentHash("deadbeefdeadbeefdeadbeefdeadbeef");
  ASSERT_TRUE(entry.has_value());

  ASSERT_EQ(nullptr, m_emulationService->loadEntry(entry->id).get());
  ASSERT_TRUE(gameLoadFailedEventReceived);
}

/**
 * @brief Test successful loading of a valid ROM file
 *
 * Verifies that a valid ROM file can be loaded, creates an EmulatorInstance, and triggers a GameLoadedEvent
 */
TEST_F(EmulationServiceTest, LoadValidRomSucceeds) {
  bool gameLoadedEventReceived = false;
  ScopedConnection loadedConnection = EventDispatcher::instance().subscribe<GameLoadedEvent>(
      [&gameLoadedEventReceived](const GameLoadedEvent &event) { gameLoadedEventReceived = true; });

  library::ContentFile info{.m_fileSizeBytes = 16777216,
                            .m_filePath = "test_resources/testrom.gba",
                            .m_fileMd5 = "e26ee0d44e809351c8ce2d73c7400cdd",
                            .m_inArchive = false,
                            .m_platformId = 3,
                            .m_contentHash = "e26ee0d44e809351c8ce2d73c7400cdd"};

  m_library->create(info);
  ASSERT_NE(info.m_id, -1);

  auto entry = m_library->getEntryWithContentHash("e26ee0d44e809351c8ce2d73c7400cdd");
  ASSERT_TRUE(entry.has_value());

  ASSERT_NE(nullptr, m_emulationService->loadEntry(entry->id).get());
  ASSERT_TRUE(gameLoadedEventReceived);

  auto currentEmulatorInstance = m_emulationService->getCurrentEmulatorInstance();
  ASSERT_FALSE(currentEmulatorInstance->isInitialized());
  ASSERT_EQ("e26ee0d44e809351c8ce2d73c7400cdd", currentEmulatorInstance->getContentHash());
  ASSERT_EQ(3, currentEmulatorInstance->getPlatformId());
}

/**
 * @brief Test successful loading of a ROM file from a ZIP archive
 *
 * Verifies that ROM files stored in ZIP archives can be properly extracted and loaded
 */
TEST_F(EmulationServiceTest, LoadValidRomInZipSucceeds) {
  bool gameLoadedEventReceived = false;
  ScopedConnection loadedConnection = EventDispatcher::instance().subscribe<GameLoadedEvent>(
      [&gameLoadedEventReceived](const GameLoadedEvent &event) { gameLoadedEventReceived = true; });

  library::ContentFile info{.m_fileSizeBytes = 0,
                            .m_filePath = "testrom.gba",
                            .m_fileMd5 = "e26ee0d44e809351c8ce2d73c7400cdd",
                            .m_inArchive = true,
                            .m_archivePathName = "test_resources/testrom.gba.zip",
                            .m_platformId = 3,
                            .m_contentHash = "e26ee0d44e809351c8ce2d73c7400cdd"};

  m_library->create(info);
  ASSERT_NE(info.m_id, -1);

  auto entry = m_library->getEntryWithContentHash("e26ee0d44e809351c8ce2d73c7400cdd");
  ASSERT_TRUE(entry.has_value());

  ASSERT_NE(nullptr, m_emulationService->loadEntry(entry->id).get());
  ASSERT_TRUE(gameLoadedEventReceived);

  auto currentEmulatorInstance = m_emulationService->getCurrentEmulatorInstance();
  ASSERT_FALSE(currentEmulatorInstance->isInitialized());
  ASSERT_EQ("e26ee0d44e809351c8ce2d73c7400cdd", currentEmulatorInstance->getContentHash());
  ASSERT_EQ(3, currentEmulatorInstance->getPlatformId());
}

/**
 * @brief Test successful loading of a ROM file from a 7z archive
 *
* Verifies that ROM files stored in 7z archives can be properly extracted and loaded
 */
TEST_F(EmulationServiceTest, LoadValidRomIn7ZSucceeds) {
  bool gameLoadedEventReceived = false;
  ScopedConnection loadedConnection = EventDispatcher::instance().subscribe<GameLoadedEvent>(
      [&gameLoadedEventReceived](const GameLoadedEvent &event) { gameLoadedEventReceived = true; });

  library::ContentFile info{.m_fileSizeBytes = 0,
                            .m_filePath = "testrom.gba",
                            .m_fileMd5 = "e26ee0d44e809351c8ce2d73c7400cdd",
                            .m_inArchive = true,
                            .m_archivePathName = "test_resources/testrom.gba.7z",
                            .m_platformId = 3,
                            .m_contentHash = "e26ee0d44e809351c8ce2d73c7400cdd"};

  m_library->create(info);
  ASSERT_NE(info.m_id, -1);

  auto entry = m_library->getEntryWithContentHash("e26ee0d44e809351c8ce2d73c7400cdd");
  ASSERT_TRUE(entry.has_value());

  ASSERT_NE(nullptr, m_emulationService->loadEntry(entry->id).get());
  ASSERT_TRUE(gameLoadedEventReceived);

  auto currentEmulatorInstance = m_emulationService->getCurrentEmulatorInstance();
  ASSERT_FALSE(currentEmulatorInstance->isInitialized());
  ASSERT_EQ("e26ee0d44e809351c8ce2d73c7400cdd", currentEmulatorInstance->getContentHash());
  ASSERT_EQ(3, currentEmulatorInstance->getPlatformId());
}

/**
 * @brief Test successful loading of a ROM file from a TAR archive
 *
* Verifies that ROM files stored in TAR archives can be properly extracted and loaded
 */
TEST_F(EmulationServiceTest, LoadValidRomInTarSucceeds) {
  bool gameLoadedEventReceived = false;
  ScopedConnection loadedConnection = EventDispatcher::instance().subscribe<GameLoadedEvent>(
      [&gameLoadedEventReceived](const GameLoadedEvent &event) { gameLoadedEventReceived = true; });

  library::ContentFile info{.m_fileSizeBytes = 0,
                            .m_filePath = "testrom.gba",
                            .m_fileMd5 = "e26ee0d44e809351c8ce2d73c7400cdd",
                            .m_inArchive = true,
                            .m_archivePathName = "test_resources/testrom.gba.tar",
                            .m_platformId = 3,
                            .m_contentHash = "e26ee0d44e809351c8ce2d73c7400cdd"};

  m_library->create(info);
  ASSERT_NE(info.m_id, -1);

  auto entry = m_library->getEntryWithContentHash("e26ee0d44e809351c8ce2d73c7400cdd");
  ASSERT_TRUE(entry.has_value());

  ASSERT_NE(nullptr, m_emulationService->loadEntry(entry->id).get());
  ASSERT_TRUE(gameLoadedEventReceived);

  auto currentEmulatorInstance = m_emulationService->getCurrentEmulatorInstance();
  ASSERT_FALSE(currentEmulatorInstance->isInitialized());
  ASSERT_EQ("e26ee0d44e809351c8ce2d73c7400cdd", currentEmulatorInstance->getContentHash());
  ASSERT_EQ(3, currentEmulatorInstance->getPlatformId());
}

/**
 * @brief Suspending with no game loaded changes nothing and publishes nothing
 */
TEST_F(EmulationServiceTest, SuspendWithNoGameIsIgnored) {
  int suspendedEvents = 0;
  ScopedConnection connection = EventDispatcher::instance().subscribe<GameSuspendedChangedEvent>(
      [&suspendedEvents](const GameSuspendedChangedEvent &) { ++suspendedEvents; });

  m_emulationService->suspend();

  ASSERT_FALSE(m_emulationService->isSuspended());
  ASSERT_EQ(0, suspendedEvents);
}

/**
 * @brief A loaded game suspends once, and a repeated suspend publishes nothing more
 */
TEST_F(EmulationServiceTest, SuspendPublishesOnce) {
  std::vector<bool> published;
  ScopedConnection connection = EventDispatcher::instance().subscribe<GameSuspendedChangedEvent>(
      [&published](const GameSuspendedChangedEvent &event) { published.push_back(event.suspended); });

  ASSERT_GE(loadTestRom(), 0);
  ASSERT_FALSE(m_emulationService->isSuspended());

  m_emulationService->suspend();
  m_emulationService->suspend();

  ASSERT_TRUE(m_emulationService->isSuspended());
  ASSERT_EQ(std::vector<bool>{true}, published);
}

/**
 * @brief Resuming a suspended game publishes false once
 */
TEST_F(EmulationServiceTest, ResumePublishesFalse) {
  std::vector<bool> published;
  ScopedConnection connection = EventDispatcher::instance().subscribe<GameSuspendedChangedEvent>(
      [&published](const GameSuspendedChangedEvent &event) { published.push_back(event.suspended); });

  ASSERT_GE(loadTestRom(), 0);
  m_emulationService->suspend();
  m_emulationService->resume();
  m_emulationService->resume();

  ASSERT_FALSE(m_emulationService->isSuspended());
  ASSERT_EQ((std::vector{true, false}), published);
}

/**
 * @brief Stopping a suspended game clears the suspension, and says so before it says the game stopped
 */
TEST_F(EmulationServiceTest, StopClearsSuspension) {
  std::vector<std::string> order;
  ScopedConnection suspendedConnection = EventDispatcher::instance().subscribe<GameSuspendedChangedEvent>(
      [&order](const GameSuspendedChangedEvent &event) { order.push_back(event.suspended ? "suspended" : "resumed"); });
  ScopedConnection stoppedConnection = EventDispatcher::instance().subscribe<EmulationStoppedEvent>(
      [&order](const EmulationStoppedEvent &) { order.push_back("stopped"); });

  ASSERT_GE(loadTestRom(), 0);
  m_emulationService->suspend();
  m_emulationService->stopEmulation();

  ASSERT_FALSE(m_emulationService->isSuspended());
  ASSERT_EQ((std::vector<std::string>{"suspended", "resumed", "stopped"}), order);
}

/**
 * @brief Loading another game while one is suspended starts the new one unsuspended
 */
TEST_F(EmulationServiceTest, RelaunchStartsUnsuspended) {
  std::vector<bool> published;
  ScopedConnection connection = EventDispatcher::instance().subscribe<GameSuspendedChangedEvent>(
      [&published](const GameSuspendedChangedEvent &event) { published.push_back(event.suspended); });

  const auto entryId = loadTestRom();
  ASSERT_GE(entryId, 0);
  m_emulationService->suspend();

  ASSERT_NE(nullptr, m_emulationService->loadEntry(entryId).get());

  ASSERT_FALSE(m_emulationService->isSuspended());
  ASSERT_EQ((std::vector{true, false}), published);
}

} // namespace firelight::emulation

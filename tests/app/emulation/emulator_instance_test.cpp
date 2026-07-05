#include <firelight/event_dispatcher.hpp>
#include <firelight/input/gamepad_input.hpp>
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
        *m_service, *m_resolver, *m_settingsService, EmulationContext{},
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
      m_testContentHash);
  ASSERT_TRUE(entry.has_value());
  ASSERT_NE(nullptr, m_emulationService->loadEntry(entry->id).get());

  auto instance = m_emulationService->getCurrentEmulatorInstance();
  ASSERT_FALSE(instance->isInitialized());

  // Check initial picture mode value
  EXPECT_EQ("aspect-ratio-fill", instance->getPictureMode());

  // Change game setting — a game override applies immediately.
  m_settingsService->setGameValue(m_testContentHash, "picture-mode",
                                  "integer-scale");

  // Verify the instance received the update
  EXPECT_EQ("integer-scale", instance->getPictureMode());
}

/**
 * @brief Sync-method and target-framerate resolve and react to setting changes.
 *
 * These drive the frame-pacing strategy. With no override they fall back to the
 * defaults (audio / 60), and a game override updates the live instance via the
 * GameSettingChangedEvent -> refreshAllSettings path.
 */
TEST_F(EmulatorInstanceTest, SyncSettingsUpdateInstance) {
  library::ContentFile info{.m_fileSizeBytes = 16777216,
                            .m_filePath = "test_resources/testrom.gba",
                            .m_fileMd5 = m_testContentHash,
                            .m_inArchive = false,
                            .m_platformId = 3,
                            .m_contentHash = m_testContentHash};

  m_library->create(info);
  ASSERT_NE(info.m_id, -1);

  auto entry = m_library->getEntryWithContentHash(
      m_testContentHash);
  ASSERT_TRUE(entry.has_value());
  ASSERT_NE(nullptr, m_emulationService->loadEntry(entry->id).get());

  auto instance = m_emulationService->getCurrentEmulatorInstance();
  ASSERT_NE(instance, nullptr);

  // No overrides yet -> defaults.
  EXPECT_EQ("audio", instance->getSyncMethod());
  EXPECT_EQ(60, instance->getTargetFramerate());

  // Game overrides update the live instance.
  m_settingsService->setGameValue(m_testContentHash, "sync-method", "monitor");
  EXPECT_EQ("monitor", instance->getSyncMethod());

  m_settingsService->setGameValue(m_testContentHash, "target-framerate", "120");
  EXPECT_EQ(120, instance->getTargetFramerate());
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
      m_testContentHash);
  ASSERT_TRUE(entry.has_value());
  ASSERT_NE(nullptr, m_emulationService->loadEntry(entry->id).get());

  auto instance = m_emulationService->getCurrentEmulatorInstance();
  // Verify instance is not initialized
  ASSERT_FALSE(instance->isInitialized());

  // Check initial aspect ratio mode value
  EXPECT_EQ("emulator-corrected", instance->getAspectRatioMode());

  // Change platform setting — with no game override, it applies to this game.
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
 * @brief Test that settings resolve by inheritance (game overrides platform).
 *
 * There is no stored "settings level": the running game resolves each setting
 * as game override -> platform override -> global -> default. A platform value
 * applies when there's no game override; a game override then shadows it.
 */
TEST_F(EmulatorInstanceTest, GameValueOverridesPlatformValue) {
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
  ASSERT_FALSE(instance->isInitialized());

  // With no game override, the platform value applies.
  m_settingsService->setPlatformValue(3, "picture-mode", "stretch");
  EXPECT_EQ("stretch", instance->getPictureMode());

  // A game override shadows the platform value.
  m_settingsService->setGameValue(m_testContentHash, "picture-mode",
                                  "integer-scale");
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

  // Check initial aspect ratio mode
  EXPECT_EQ("emulator-corrected", instance->getAspectRatioMode());

  // Try to change setting for different platform ID
  m_settingsService->setPlatformValue(999, "aspect-ratio", "pixel-perfect");

  // Instance should not be affected
  EXPECT_EQ("emulator-corrected", instance->getAspectRatioMode());
}

/**
 * @brief Disc-control passes through to the core and swapDisc publishes an event.
 *
 * Uses a FakeCore configured with a 3-disc set: the instance reports the count,
 * a valid swap changes the current index and emits a DiscChangedEvent, and an
 * out-of-range swap is rejected without an event.
 */
TEST_F(EmulatorInstanceTest, DiscControlPassthroughAndEvent) {
  // Rebuild the service with a factory that hands out a multi-disc FakeCore.
  m_emulationService = std::make_unique<EmulationService>(
      *m_service, *m_resolver, *m_settingsService, EmulationContext{},
      [](int, const std::string &,
         std::shared_ptr<firelight::libretro::IConfigurationProvider>,
         const std::string &) -> std::unique_ptr<::libretro::ICore> {
        auto core = std::make_unique<FakeCore>();
        core->setDiscCount(3);
        return core;
      });
  EmulationService::setInstance(m_emulationService.get());

  library::ContentFile info{.m_fileSizeBytes = 16777216,
                            .m_filePath = "test_resources/testrom.gba",
                            .m_fileMd5 = m_testContentHash,
                            .m_inArchive = false,
                            .m_platformId = 3,
                            .m_contentHash = m_testContentHash};
  m_library->create(info);
  ASSERT_NE(info.m_id, -1);

  auto entry = m_library->getEntryWithContentHash(
      m_testContentHash);
  ASSERT_TRUE(entry.has_value());
  ASSERT_NE(nullptr, m_emulationService->loadEntry(entry->id).get());

  auto *instance = m_emulationService->getCurrentEmulatorInstance();
  ASSERT_NE(instance, nullptr);

  EXPECT_EQ(3u, instance->getDiscCount());
  EXPECT_EQ(0u, instance->getCurrentDiscIndex());

  std::vector<DiscChangedEvent> events;
  auto conn = EventDispatcher::instance().subscribe<DiscChangedEvent>(
      [&](const DiscChangedEvent &e) { events.push_back(e); });

  EXPECT_TRUE(instance->swapDisc(2));
  EXPECT_EQ(2u, instance->getCurrentDiscIndex());
  ASSERT_EQ(1u, events.size());
  EXPECT_EQ(2u, events[0].index);
  EXPECT_EQ(3u, events[0].count);
  EXPECT_EQ(m_testContentHash, events[0].contentHash);

  // Out of range -> rejected, no new event.
  EXPECT_FALSE(instance->swapDisc(5));
  EXPECT_EQ(1u, events.size());
}

/**
 * @brief A CLI --save-slot override applies to the next launch, then is consumed.
 */
TEST_F(EmulatorInstanceTest, SaveSlotOverrideAppliesOnceThenConsumed) {
  library::ContentFile info{.m_fileSizeBytes = 16777216,
                            .m_filePath = "test_resources/testrom.gba",
                            .m_fileMd5 = m_testContentHash,
                            .m_inArchive = false,
                            .m_platformId = 3,
                            .m_contentHash = m_testContentHash};
  m_library->create(info);
  ASSERT_NE(info.m_id, -1);

  auto entry = m_library->getEntryWithContentHash(
      m_testContentHash);
  ASSERT_TRUE(entry.has_value());

  // Override the save slot for the next launch.
  m_emulationService->setPendingLaunchOverrides({.saveSlot = 7});
  ASSERT_NE(nullptr, m_emulationService->loadEntry(entry->id).get());
  auto *first = m_emulationService->getCurrentEmulatorInstance();
  ASSERT_NE(first, nullptr);
  EXPECT_EQ(7, first->getSaveSlotNumber());

  // The override is one-shot: a second launch uses the entry's own active slot.
  ASSERT_NE(nullptr, m_emulationService->loadEntry(entry->id).get());
  auto *second = m_emulationService->getCurrentEmulatorInstance();
  ASSERT_NE(second, nullptr);
  EXPECT_EQ(static_cast<int>(entry->activeSaveSlot), second->getSaveSlotNumber());
}

/**
 * @brief The core's advertised port devices are exposed, and selecting one
 * calls the core and persists the choice per-game.
 */
TEST_F(EmulatorInstanceTest, ControllerDevicesExposedAndSelectable) {
  FakeCore *fake = nullptr;
  m_emulationService = std::make_unique<EmulationService>(
      *m_service, *m_resolver, *m_settingsService, EmulationContext{},
      [&fake](int, const std::string &,
              std::shared_ptr<firelight::libretro::IConfigurationProvider>,
              const std::string &) -> std::unique_ptr<::libretro::ICore> {
        auto core = std::make_unique<FakeCore>();
        // Advertise the ids the snes9x catalog knows (SNES == platform 6):
        // standard pad, SNES Mouse, Super Scope.
        core->setControllerDevices(
            {{{1, "SNES Pad"}, {2, "Mouse"}, {260, "Super Scope"}}});
        fake = core.get();
        return core;
      });
  EmulationService::setInstance(m_emulationService.get());

  library::ContentFile info{.m_fileSizeBytes = 16777216,
                            // A real file must exist on disk (content is ignored
                            // by the fake core); the platform selects the core.
                            .m_filePath = "test_resources/testrom.gba",
                            .m_fileMd5 = m_testContentHash,
                            .m_inArchive = false,
                            .m_platformId = 6, // SNES -> snes9x catalog
                            .m_contentHash = m_testContentHash};
  m_library->create(info);
  ASSERT_NE(info.m_id, -1);

  auto entry = m_library->getEntryWithContentHash(
      m_testContentHash);
  ASSERT_TRUE(entry.has_value());
  ASSERT_NE(nullptr, m_emulationService->loadEntry(entry->id).get());

  auto *instance = m_emulationService->getCurrentEmulatorInstance();
  ASSERT_NE(instance, nullptr);
  ASSERT_NE(fake, nullptr);

  // The core advertised three devices on port 0.
  const auto devices = instance->getControllerDevices();
  ASSERT_EQ(devices.size(), 1u);
  ASSERT_EQ(devices[0].size(), 3u);

  // The curated variants are surfaced console-natively (the standard pad plus
  // the SNES Mouse / Super Scope the snes9x catalog knows, cross-referenced with
  // the core's advertisement), default first.
  const auto variants = instance->getAvailableControllerVariants(0);
  ASSERT_EQ(variants.size(), 3u);
  EXPECT_TRUE(variants.front().isDefault);
  EXPECT_EQ(variants.front().deviceClass,
            firelight::input::GamepadInputClass::Joypad);
  bool hasSuperScope = false;
  for (const auto &v : variants) {
    if (v.coreDeviceId == 260u) {
      hasSuperScope = true;
      EXPECT_EQ(v.deviceClass, firelight::input::GamepadInputClass::Lightgun);
      EXPECT_EQ(v.friendlyName, "Super Scope");
    }
  }
  EXPECT_TRUE(hasSuperScope);

  // Selecting a variant drives the core and persists per-game (by coreDeviceId).
  instance->setPortControllerVariant(0, 260);
  ASSERT_FALSE(fake->portDeviceCalls().empty());
  EXPECT_EQ(fake->portDeviceCalls().back().first, 0u);
  EXPECT_EQ(fake->portDeviceCalls().back().second, 260u);
  EXPECT_EQ(m_settingsService->getGameValue(m_testContentHash,
                                            "port0-controllervariant"),
            "260");
}

// --- ICore state contract (exercised via FakeCore): getSerializeSize matches
// the serialized length, a valid state round-trips, and a size mismatch is
// refused without touching the core. Guards the deserializeState hardening that
// suspend points / rewind / future rollback rely on. ---
TEST(CoreStateContractTest, SerializeRoundTripAndRejectsSizeMismatch) {
  FakeCore core;
  core.loadGame(nullptr); // seeds SAVE_RAM

  core.run(0);
  core.run(0);
  core.run(0);
  const auto saved = core.serializeState();
  const int savedFrame = core.frameCount();

  EXPECT_EQ(core.getSerializeSize(), saved.size());

  // Diverge from the snapshot.
  core.run(0);
  core.run(0);
  ASSERT_NE(core.frameCount(), savedFrame);

  // A correctly-sized state restores and reports success.
  EXPECT_TRUE(core.deserializeState(saved));
  EXPECT_EQ(core.frameCount(), savedFrame);

  // Wrong-sized states are refused and leave the core untouched.
  const std::vector<uint8_t> tooLong(saved.size() + 1, 0);
  EXPECT_FALSE(core.deserializeState(tooLong));
  EXPECT_EQ(core.frameCount(), savedFrame);
  EXPECT_FALSE(core.deserializeState({}));
  EXPECT_EQ(core.frameCount(), savedFrame);
}

} // namespace firelight::emulation

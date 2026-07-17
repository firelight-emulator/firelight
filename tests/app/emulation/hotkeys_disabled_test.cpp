#include "fake_core.hpp"
#include "fake_input_service.hpp"

#include <emulation/emulation_service.hpp>
#include <firelight/library/entry_resolver.hpp>
#include <firelight/library/library_ingest_service.hpp>
#include <firelight/library/sqlite_user_library.hpp>
#include <firelight/library/user_library_service.hpp>
#include <firelight/settings/settings_service.hpp>
#include <firelight/settings/sqlite_settings_repository.hpp>

#include <gtest/gtest.h>

namespace firelight::emulation {

// The `hotkeys-disabled` setting and the keyboard auto-disable, driven through
// a real EmulatorInstance against a FakeCore and a recording input service.
class HotkeysDisabledTest : public testing::Test {
protected:
  std::unique_ptr<library::SqliteUserLibraryRepository> m_library;
  std::unique_ptr<library::LibraryIngestService> m_ingest;
  std::unique_ptr<library::UserLibraryService> m_libraryService;
  std::unique_ptr<library::EntryResolver> m_resolver;
  std::unique_ptr<settings::SettingsService> m_settingsService;
  std::unique_ptr<EmulationService> m_emulationService;
  FakeInputService m_inputService;
  FakeCore *m_fakeCore = nullptr;
  bool m_coreWantsKeyboard = false;

  const std::string m_hash = "e26ee0d44e809351c8ce2d73c7400cdd";
  static constexpr int PLATFORM_ID = 3;

  void SetUp() override {
    m_library =
        std::make_unique<library::SqliteUserLibraryRepository>(":memory:");
    m_ingest = std::make_unique<library::LibraryIngestService>(*m_library);
    m_libraryService =
        std::make_unique<library::UserLibraryService>(*m_library, ".");
    m_resolver = std::make_unique<library::EntryResolver>(*m_library);
    m_settingsService = std::make_unique<settings::SettingsService>(
        *new settings::SqliteSettingsRepository(":memory:"));
    settings::SettingsService::setInstance(m_settingsService.get());

    EmulationContext context;
    context.inputService = &m_inputService;
    context.settingsService = m_settingsService.get();

    m_emulationService = std::make_unique<EmulationService>(
        *m_libraryService, *m_resolver, *m_settingsService, context,
        [this](const firelight::libretro::CoreRunConfig &)
            -> std::unique_ptr<::libretro::ICore> {
          auto fake = std::make_unique<FakeCore>();
          fake->m_registersKeyboardOnLoad = m_coreWantsKeyboard;
          m_fakeCore = fake.get();
          return fake;
        });
  }

  void TearDown() override {
    m_emulationService.reset();
    settings::SettingsService::setInstance(nullptr);
    m_settingsService.reset();
    m_resolver.reset();
    m_libraryService.reset();
    m_ingest.reset();
    m_library.reset();
  }

  int ingestEntry() {
    library::ContentFile info{.m_fileSizeBytes = 16777216,
                              .m_filePath = "test_resources/testrom.gba",
                              .m_fileMd5 = m_hash,
                              .m_inArchive = false,
                              .m_platformId = PLATFORM_ID,
                              .m_contentHash = m_hash};
    m_library->create(info);
    const auto entry = m_library->getEntryWithContentHash(m_hash);
    return entry.has_value() ? entry->id : -1;
  }

  EmulatorInstance *loadAndInitialize() {
    const int entryId = ingestEntry();
    EXPECT_NE(entryId, -1);
    auto *instance = m_emulationService->loadEntry(entryId).get();
    EXPECT_NE(instance, nullptr);
    EXPECT_TRUE(instance->initialize(nullptr));
    return instance;
  }
};

TEST_F(HotkeysDisabledTest, DefaultsToHotkeysOn) {
  loadAndInitialize();
  EXPECT_TRUE(m_inputService.hotkeysFullyEnabled());
}

TEST_F(HotkeysDisabledTest, GameValueDisablesEveryDevice) {
  m_settingsService->setGameValue(m_hash, "hotkeys-disabled", "true");
  loadAndInitialize();

  ASSERT_FALSE(m_inputService.hotkeyCalls.empty());
  const auto &last = m_inputService.hotkeyCalls.back();
  EXPECT_FALSE(last.enabled);
  EXPECT_FALSE(last.only.has_value()); // every device, not just the keyboard
}

TEST_F(HotkeysDisabledTest, PlatformValueDisablesAndGameValueOverridesIt) {
  m_settingsService->setPlatformValue(PLATFORM_ID, "hotkeys-disabled", "true");
  m_settingsService->setGameValue(m_hash, "hotkeys-disabled", "false");
  loadAndInitialize();

  EXPECT_TRUE(m_inputService.hotkeysFullyEnabled());
}

TEST_F(HotkeysDisabledTest, LiveSettingChangeReachesTheInputService) {
  loadAndInitialize();
  ASSERT_TRUE(m_inputService.hotkeysFullyEnabled());

  m_settingsService->setGameValue(m_hash, "hotkeys-disabled", "true");
  EXPECT_FALSE(m_inputService.hotkeysFullyEnabled());

  m_settingsService->setGameValue(m_hash, "hotkeys-disabled", "false");
  EXPECT_TRUE(m_inputService.hotkeysFullyEnabled());
}

// The core only registers its keyboard callback while loading, so a decision
// made before that (in the constructor) always sees wantsKeyboard() == false.
TEST_F(HotkeysDisabledTest, KeyboardAutoDisableWaitsForTheCoreToLoad) {
  m_coreWantsKeyboard = true;
  loadAndInitialize();

  EXPECT_TRUE(m_inputService.keyboardHotkeysDisabled());
  // Only the keyboard: a controller keeps its hotkeys.
  const auto &last = m_inputService.hotkeyCalls.back();
  ASSERT_TRUE(last.only.has_value());
  EXPECT_EQ(DeviceType::Keyboard, *last.only);
  EXPECT_FALSE(last.enabled);
}

TEST_F(HotkeysDisabledTest, KeyboardAutoDisableSkippedWhenTheCoreIgnoresKeys) {
  m_coreWantsKeyboard = false;
  loadAndInitialize();
  EXPECT_FALSE(m_inputService.keyboardHotkeysDisabled());
}

TEST_F(HotkeysDisabledTest, KeyboardAutoDisableCanBeTurnedOff) {
  m_settingsService->setGlobalValue("auto-disable-keyboard-hotkeys", "false");
  m_coreWantsKeyboard = true;
  loadAndInitialize();

  EXPECT_FALSE(m_inputService.keyboardHotkeysDisabled());
}

// A game that turned hotkeys off must not leave the menu without them.
TEST_F(HotkeysDisabledTest, UnloadingHandsHotkeysBack) {
  m_coreWantsKeyboard = true;
  loadAndInitialize();
  ASSERT_TRUE(m_inputService.keyboardHotkeysDisabled());

  m_emulationService->stopEmulation();
  EXPECT_TRUE(m_inputService.hotkeysFullyEnabled());
}

} // namespace firelight::emulation

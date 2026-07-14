// Proves the Phase 1 spine runs on the REAL emulation object: a TasSession driving
// an EmulatorInstance (through EmulatorInstanceTasEmulator) plays and greenzone-seeks
// correctly, with the instance's own runFrame / serializeState / deserializeState.
// Headless via EmulationService + FakeCore (the proven pattern); FakeCore ignores
// input, so this asserts the drive/seek wiring — input-driven state is covered by
// tas_input_playback_test (real CoreInputRouter) and tas_session_test (fake emu).
#include "../emulation/fake_core.hpp"

#include <emulation/emulation_service.hpp>
#include <emulation/tas_emulator_adapter.hpp>
#include <firelight/library/entry_resolver.hpp>
#include <firelight/library/library_ingest_service.hpp>
#include <firelight/library/sqlite_user_library.hpp>
#include <firelight/library/user_library_service.hpp>
#include <firelight/saves/isave_manager.hpp>
#include <firelight/settings/settings_service.hpp>
#include <firelight/settings/sqlite_core_option_repository.hpp>
#include <firelight/settings/sqlite_settings_repository.hpp>
#include <firelight/cheats/sqlite_cheat_repository.hpp>
#include <firelight/tas/tas_session.hpp>

#include <gtest/gtest.h>

#include <future>
#include <optional>
#include <vector>

namespace firelight::emulation {
namespace {

using input::InputFrame;

// Inert ISaveManager: TAS mode suppresses autosave and these tests don't assert on
// saves, so nothing needs to be recorded.
class NullSaveManager : public saves::ISaveManager {
public:
  std::future<bool> writeSaveData(const std::string &, int,
                                  const saves::Savefile &) override {
    std::promise<bool> p;
    p.set_value(true);
    return p.get_future();
  }
  [[nodiscard]] std::optional<saves::Savefile>
  readSaveData(const std::string &, int) const override {
    return std::nullopt;
  }
  [[nodiscard]] std::vector<saves::SavefileInfo>
  getSaveFileInfoList(const std::string &) const override {
    return {};
  }
  void writeSuspendPoint(const std::string &, int, int,
                         const SuspendPoint &) override {}
  std::optional<SuspendPoint> readSuspendPoint(const std::string &, int,
                                               int) override {
    return std::nullopt;
  }
  void deleteSuspendPoint(const std::string &, int, int) override {}
  [[nodiscard]] std::string getSaveDirectory() const override { return m_dir; }
  void setSaveDirectory(const std::string &dir) override { m_dir = dir; }

private:
  std::string m_dir;
};

std::vector<InputFrame> makeMovie(std::size_t n) {
  std::vector<InputFrame> m;
  m.reserve(n);
  for (std::size_t k = 0; k < n; ++k) {
    InputFrame f;
    f.setButton(static_cast<unsigned>(k % 12), true);
    m.push_back(f);
  }
  return m;
}

class TasSessionEmulatorTest : public testing::Test {
protected:
  std::unique_ptr<library::SqliteUserLibraryRepository> m_library;
  std::unique_ptr<library::LibraryIngestService> m_ingest;
  std::unique_ptr<library::UserLibraryService> m_libraryService;
  std::unique_ptr<library::EntryResolver> m_resolver;
  std::unique_ptr<settings::SettingsService> m_settingsService;
  std::unique_ptr<settings::SqliteCoreOptionRepository> m_coreOptionRepo;
  std::unique_ptr<cheats::SqliteCheatRepository> m_cheatRepo;
  std::unique_ptr<EmulationService> m_emulationService;
  NullSaveManager m_saves;
  FakeCore *m_fakeCore = nullptr;

  const std::string m_hash = "e26ee0d44e809351c8ce2d73c7400cdd";

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
    m_coreOptionRepo =
        std::make_unique<settings::SqliteCoreOptionRepository>(":memory:");
    m_cheatRepo = std::make_unique<cheats::SqliteCheatRepository>(":memory:");

    CoreFactory factory =
        [this](const firelight::libretro::CoreRunConfig &config)
        -> std::unique_ptr<::libretro::ICore> {
      auto fake = std::make_unique<FakeCore>();
      fake->setConfigProvider(config.configProvider);
      fake->setSaveDirectory(config.saveDirectory);
      fake->setSystemRamSize(256);
      m_fakeCore = fake.get();
      return fake;
    };

    EmulationContext context;
    context.saveManager = &m_saves;
    context.coreOptionRepository = m_coreOptionRepo.get();
    context.cheatRepository = m_cheatRepo.get();
    m_emulationService = std::make_unique<EmulationService>(
        *m_libraryService, *m_resolver, *m_settingsService, context,
        std::move(factory));
  }

  void TearDown() override {
    m_emulationService.reset();
    m_coreOptionRepo.reset();
    m_cheatRepo.reset();
    settings::SettingsService::setInstance(nullptr);
    m_settingsService.reset();
    m_resolver.reset();
    m_libraryService.reset();
    m_ingest.reset();
    m_library.reset();
  }

  EmulatorInstance *loadInitializedTasInstance() {
    library::ContentFile info{.m_fileSizeBytes = 16777216,
                              .m_filePath = "test_resources/testrom.gba",
                              .m_fileMd5 = m_hash,
                              .m_inArchive = false,
                              .m_platformId = 3,
                              .m_contentHash = m_hash};
    m_library->create(info);
    const auto entry = m_library->getEntryWithContentHash(m_hash);
    EXPECT_TRUE(entry.has_value());
    auto *instance = m_emulationService->loadEntry(entry->id).get();
    EXPECT_NE(instance, nullptr);
    if (instance) {
      instance->setTasMode(true);
      EXPECT_TRUE(instance->initialize(nullptr));
    }
    return instance;
  }
};

TEST_F(TasSessionEmulatorTest, DrivesRealInstanceForwardAndSeeks) {
  auto *instance = loadInitializedTasInstance();
  ASSERT_NE(instance, nullptr);
  ASSERT_NE(m_fakeCore, nullptr);

  EmulatorInstanceTasEmulator emu(*instance);
  tas::TasSession session(emu, {/*stride=*/4, /*maxKeyframes=*/0});
  session.loadMovie(makeMovie(20));
  EXPECT_EQ(session.currentFrame(), 0u);
  EXPECT_EQ(m_fakeCore->frameCount(), 0);

  for (int i = 0; i < 20; ++i) {
    session.stepForward();
  }
  EXPECT_EQ(session.currentFrame(), 20u);
  EXPECT_EQ(m_fakeCore->frameCount(), 20); // the real instance advanced 20 frames

  // Greenzone seek must restore the instance's own state (via deserializeState).
  session.seekTo(5);
  EXPECT_EQ(session.currentFrame(), 5u);
  EXPECT_EQ(m_fakeCore->frameCount(), 5);

  session.seekTo(17);
  EXPECT_EQ(session.currentFrame(), 17u);
  EXPECT_EQ(m_fakeCore->frameCount(), 17);

  m_emulationService->stopEmulation();
}

TEST_F(TasSessionEmulatorTest, EditRepositionsRealInstance) {
  auto *instance = loadInitializedTasInstance();
  ASSERT_NE(instance, nullptr);
  ASSERT_NE(m_fakeCore, nullptr);

  EmulatorInstanceTasEmulator emu(*instance);
  tas::TasSession session(emu, {/*stride=*/4, /*maxKeyframes=*/0});
  session.loadMovie(makeMovie(16));
  for (int i = 0; i < 16; ++i) {
    session.stepForward();
  }
  ASSERT_EQ(m_fakeCore->frameCount(), 16);

  InputFrame edited;
  edited.setButton(libretro::IRetroPad::Start, true);
  session.editFrame(4, edited);

  EXPECT_EQ(session.rerecordCount(), 1u);
  EXPECT_EQ(session.currentFrame(), 4u);
  EXPECT_EQ(m_fakeCore->frameCount(), 4); // instance rewound to the edit point

  m_emulationService->stopEmulation();
}

} // namespace
} // namespace firelight::emulation

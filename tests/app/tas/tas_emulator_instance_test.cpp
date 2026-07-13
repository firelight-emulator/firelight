// Verifies EmulatorInstance's TAS mode: setTasMode(true) makes runFrame() a pure
// emulation step by suppressing the periodic autosave. Driven headlessly through
// EmulationService + a FakeCore (the proven pattern from emulator_instance_e2e_test),
// with a spy ISaveManager that counts writeSaveData() calls. The autosave's *call*
// is synchronous (only its returned future is async), so the count is deterministic.
#include "../emulation/fake_core.hpp"

#include <emulation/emulation_service.hpp>
#include <firelight/library/entry_resolver.hpp>
#include <firelight/library/library_ingest_service.hpp>
#include <firelight/library/sqlite_user_library.hpp>
#include <firelight/library/user_library_service.hpp>
#include <firelight/saves/isave_manager.hpp>
#include <firelight/settings/settings_service.hpp>
#include <firelight/settings/sqlite_core_option_repository.hpp>
#include <firelight/settings/sqlite_settings_repository.hpp>
#include <firelight/cheats/sqlite_cheat_repository.hpp>

#include <gtest/gtest.h>

#include <future>
#include <optional>
#include <vector>

namespace firelight::emulation {
namespace {

// Counts autosave writes; everything else is inert. writeSaveData records the call
// synchronously and hands back a ready future, so the count reflects runFrame()'s
// autosave decisions with no threading race.
class SpySaveManager : public saves::ISaveManager {
public:
  int writeCount = 0;

  std::future<bool> writeSaveData(const std::string &, int,
                                  const saves::Savefile &) override {
    ++writeCount;
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
  // SuspendPoint is a global-namespace struct (see suspend_point.hpp).
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

class TasModeTest : public testing::Test {
protected:
  std::unique_ptr<library::SqliteUserLibraryRepository> m_library;
  std::unique_ptr<library::LibraryIngestService> m_ingest;
  std::unique_ptr<library::UserLibraryService> m_libraryService;
  std::unique_ptr<library::EntryResolver> m_resolver;
  std::unique_ptr<settings::SettingsService> m_settingsService;
  std::unique_ptr<settings::SqliteCoreOptionRepository> m_coreOptionRepo;
  std::unique_ptr<cheats::SqliteCheatRepository> m_cheatRepo;
  std::unique_ptr<EmulationService> m_emulationService;
  SpySaveManager m_spy;
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
    context.saveManager = &m_spy;
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

  int ingestEntry() {
    library::ContentFile info{.m_fileSizeBytes = 16777216,
                              .m_filePath = "test_resources/testrom.gba",
                              .m_fileMd5 = m_hash,
                              .m_inArchive = false,
                              .m_platformId = 3,
                              .m_contentHash = m_hash};
    m_library->create(info);
    const auto entry = m_library->getEntryWithContentHash(m_hash);
    return entry.has_value() ? entry->id : -1;
  }

  EmulatorInstance *load() {
    const int entryId = ingestEntry();
    EXPECT_NE(entryId, -1);
    auto *instance = m_emulationService->loadEntry(entryId).get();
    EXPECT_NE(instance, nullptr);
    return instance;
  }
};

TEST_F(TasModeTest, FlagRoundTrips) {
  auto *inst = load();
  ASSERT_NE(inst, nullptr);
  EXPECT_FALSE(inst->isTasMode());
  inst->setTasMode(true);
  EXPECT_TRUE(inst->isTasMode());
  inst->setTasMode(false);
  EXPECT_FALSE(inst->isTasMode());
  m_emulationService->stopEmulation();
}

// Control: with the autosave cadence forced to fire every frame, a normal (non-TAS)
// run autosaves — proving the guard's condition is genuinely being reached.
TEST_F(TasModeTest, NonTasModeAutosaves) {
  auto *inst = load();
  ASSERT_NE(inst, nullptr);
  inst->setAutosaveIntervalSeconds(0); // any elapsed time triggers the autosave
  ASSERT_TRUE(inst->initialize(nullptr));
  m_spy.writeCount = 0; // ignore any writes during load/initialize
  for (int i = 0; i < 5; ++i) {
    inst->runFrame();
  }
  EXPECT_GE(m_spy.writeCount, 1) << "non-TAS run should have autosaved";
  m_emulationService->stopEmulation();
}

// Under the identical cadence, TAS mode suppresses the autosave entirely.
TEST_F(TasModeTest, TasModeSuppressesAutosave) {
  auto *inst = load();
  ASSERT_NE(inst, nullptr);
  inst->setAutosaveIntervalSeconds(0); // would fire every frame...
  inst->setTasMode(true);              // ...but TAS mode suppresses it
  ASSERT_TRUE(inst->initialize(nullptr));
  m_spy.writeCount = 0;
  for (int i = 0; i < 5; ++i) {
    inst->runFrame();
  }
  EXPECT_EQ(m_spy.writeCount, 0) << "TAS mode must not autosave";
  EXPECT_EQ(m_fakeCore->frameCount(), 5); // emulation still advances normally
  m_emulationService->stopEmulation();
}

} // namespace
} // namespace firelight::emulation

#include "../saves/fake_userdata_database.hpp"
#include "fake_core.hpp"

#include <emulation/emulation_service.hpp>
#include <firelight/library/entry_resolver.hpp>
#include <firelight/library/library_ingest_service.hpp>
#include <firelight/library/sqlite_user_library.hpp>
#include <firelight/library/user_library_service.hpp>
#include <firelight/saves/save_manager_impl.hpp>
#include <firelight/settings/settings_service.hpp>
#include <firelight/settings/sqlite_settings_repository.hpp>
#include <manager_accessor.hpp>

#include <gtest/gtest.h>

#include <QString>
#include <QTemporaryDir>

namespace firelight::emulation {

// End-to-end flow driven entirely through EmulationService/EmulatorInstance with
// a FakeCore injected via the core factory: load -> initialize -> run frames ->
// save -> rewind (serialize/deserialize) -> reset -> teardown, with NO real
// libretro DLL (so no process-exit on core teardown).
class EmulatorInstanceE2ETest : public testing::Test {
protected:
  std::unique_ptr<library::SqliteUserLibraryRepository> m_library;
  std::unique_ptr<library::LibraryIngestService> m_ingest;
  std::unique_ptr<library::UserLibraryService> m_libraryService;
  std::unique_ptr<library::EntryResolver> m_resolver;
  std::unique_ptr<settings::SettingsService> m_settingsService;
  std::unique_ptr<FakeUserdataDatabase> m_userdataDb;
  std::unique_ptr<saves::SaveManager> m_saveManager;
  std::unique_ptr<EmulationService> m_emulationService;
  QTemporaryDir m_saveDir;
  FakeCore *m_fakeCore = nullptr;

  const std::string m_hash = "e26ee0d44e809351c8ce2d73c7400cdd";

  void SetUp() override {
    ASSERT_TRUE(m_saveDir.isValid());
    m_library =
        std::make_unique<library::SqliteUserLibraryRepository>(":memory:");
    // Turns created content files into entries (subscribes to library events).
    m_ingest = std::make_unique<library::LibraryIngestService>(*m_library);
    m_libraryService =
        std::make_unique<library::UserLibraryService>(*m_library, ".");
    m_resolver = std::make_unique<library::EntryResolver>(*m_library);
    m_settingsService = std::make_unique<settings::SettingsService>(
        *new settings::SqliteSettingsRepository(":memory:"));
    settings::SettingsService::setInstance(m_settingsService.get());

    m_userdataDb = std::make_unique<FakeUserdataDatabase>();
    m_saveManager =
        std::make_unique<saves::SaveManager>(m_saveDir.path(), *m_userdataDb);
    m_saveManager->setSaveDirectory(m_saveDir.path());
    ManagerAccessor::setSaveManager(m_saveManager.get());

    CoreFactory factory =
        [this](int, const std::string &,
               std::shared_ptr<firelight::libretro::IConfigurationProvider>,
               const std::string &) -> std::unique_ptr<::libretro::ICore> {
      auto fake = std::make_unique<FakeCore>();
      m_fakeCore = fake.get();
      return fake;
    };
    m_emulationService = std::make_unique<EmulationService>(
        *m_libraryService, *m_resolver, *m_settingsService, std::move(factory));
  }

  void TearDown() override {
    m_emulationService.reset();
    ManagerAccessor::setSaveManager(nullptr);
    m_saveManager.reset();
    m_userdataDb.reset();
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
    const auto entry =
        m_library->getEntryWithContentHash(QString::fromStdString(m_hash));
    return entry.has_value() ? entry->id : -1;
  }
};

TEST_F(EmulatorInstanceE2ETest, LoadRunSaveRewindResetTeardown) {
  const int entryId = ingestEntry();
  ASSERT_NE(entryId, -1);

  auto *instance = m_emulationService->loadEntry(entryId).get();
  ASSERT_NE(instance, nullptr);
  ASSERT_NE(m_fakeCore, nullptr);

  // Achievements/input services are unset and null-guarded, so initialize()
  // works with just a (null) video receiver.
  ASSERT_TRUE(instance->initialize(nullptr));
  ASSERT_TRUE(instance->isInitialized());
  ASSERT_TRUE(m_fakeCore->initialized());

  // Run frames — the fake advances a frame counter and touches SRAM.
  for (int i = 0; i < 5; ++i) {
    instance->runFrame();
  }
  EXPECT_EQ(m_fakeCore->frameCount(), 5);

  // Save — SRAM (mutated by run) is persisted through the real SaveManager.
  ASSERT_TRUE(instance->save().get());
  const auto sramAtSave = m_fakeCore->sram();
  const auto readBack = m_saveManager->readSaveData(
      QString::fromStdString(m_hash), instance->getSaveSlotNumber());
  ASSERT_TRUE(readBack.has_value());
  EXPECT_EQ(readBack->getSaveRamData(), sramAtSave);

  // Rewind — capture a state, advance, then restore it.
  const auto rewindPoint = instance->serializeState();
  for (int i = 0; i < 3; ++i) {
    instance->runFrame();
  }
  EXPECT_EQ(m_fakeCore->frameCount(), 8);
  instance->deserializeState(rewindPoint);
  EXPECT_EQ(m_fakeCore->frameCount(), 5); // restored to the rewind point

  // Reset.
  instance->reset();
  EXPECT_EQ(m_fakeCore->frameCount(), 0);

  // Teardown — destroying the instance must NOT exit the process (no real DLL).
  m_emulationService->stopEmulation();
  EXPECT_EQ(m_emulationService->getCurrentEmulatorInstance(), nullptr);
}

} // namespace firelight::emulation

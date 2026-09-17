// TODO: NEEDS REVIEW
#pragma once

#include "fake_core.hpp"
#include "fake_save_database.hpp"

#include <firelight/cheats/sqlite_cheat_repository.hpp>
#include <firelight/library/disc_set_service.hpp>
#include <firelight/library/entry_resolver.hpp>
#include <firelight/library/library_ingest_service.hpp>
#include <firelight/library/sqlite_user_library.hpp>
#include <firelight/library/user_library_service.hpp>
#include <firelight/saves/save_manager_impl.hpp>
#include <firelight/settings/settings_service.hpp>
#include <firelight/settings/sqlite_settings_repository.hpp>

#include <QString>
#include <QTemporaryDir>
#include <emulation/emulation_service.hpp>
#include <gtest/gtest.h>
#include <memory>
#include <string>

namespace firelight::emulation {

/**
 * A real EmulationService over in-memory repositories, loading a FakeCore through the core factory,
 * so a game can be loaded, run and torn down with no libretro DLL
 */
class EmulationServiceFixture : public testing::Test {
protected:
  std::unique_ptr<library::SqliteUserLibraryRepository> m_library;
  std::unique_ptr<library::DiscSetService> m_discSets;
  std::unique_ptr<library::LibraryIngestService> m_ingest;
  std::unique_ptr<library::UserLibraryService> m_libraryService;
  std::unique_ptr<library::EntryResolver> m_resolver;
  std::unique_ptr<settings::SettingsService> m_settingsService;
  std::unique_ptr<settings::SqliteSettingsRepository> m_coreOptionRepo;
  std::unique_ptr<cheats::SqliteCheatRepository> m_cheatRepo;
  std::unique_ptr<saves::FakeSaveDatabase> m_userdataDb;
  std::unique_ptr<saves::SaveManager> m_saveManager;
  std::unique_ptr<EmulationService> m_emulationService;
  QTemporaryDir m_saveDir;
  FakeCore *m_fakeCore = nullptr;

  const std::string m_hash = "e26ee0d44e809351c8ce2d73c7400cdd";

  void SetUp() override {
    ASSERT_TRUE(m_saveDir.isValid());
    m_library = std::make_unique<library::SqliteUserLibraryRepository>(":memory:");
    // Turns created content files into entries (subscribes to library events)
    m_discSets = std::make_unique<library::DiscSetService>(*m_library, "");
    m_ingest = std::make_unique<library::LibraryIngestService>(*m_library, *m_discSets);
    m_libraryService = std::make_unique<library::UserLibraryService>(*m_library, ".");
    m_resolver = std::make_unique<library::EntryResolver>(*m_library, "");
    m_settingsService =
        std::make_unique<settings::SettingsService>(*new settings::SqliteSettingsRepository(":memory:"));
    settings::SettingsService::setInstance(m_settingsService.get());

    m_userdataDb = std::make_unique<saves::FakeSaveDatabase>();
    m_saveManager = std::make_unique<saves::SaveManager>(m_saveDir.path().toStdString(), *m_userdataDb);
    m_saveManager->setSaveDirectory(m_saveDir.path().toStdString());

    m_coreOptionRepo = std::make_unique<settings::SqliteSettingsRepository>(":memory:");
    m_cheatRepo = std::make_unique<cheats::SqliteCheatRepository>(":memory:");

    CoreFactory factory =
        [this](const firelight::libretro::CoreRunConfig &config) -> std::unique_ptr<::libretro::ICore> {
      auto fake = std::make_unique<FakeCore>();
      fake->setConfigProvider(config.configProvider);
      fake->setSaveDirectory(config.saveDirectory); // the real Core takes it in its ctor
      fake->setSystemRamSize(256);                  // so the cheat engine has RAM to poke
      m_fakeCore = fake.get();
      return fake;
    };

    EmulationContext context;
    context.saveManager = m_saveManager.get();
    context.coreOptionRepository = m_coreOptionRepo.get();
    context.cheatRepository = m_cheatRepo.get();
    m_emulationService = std::make_unique<EmulationService>(*m_libraryService, *m_resolver, *m_settingsService, context,
                                                            std::move(factory));
  }

  void TearDown() override {
    m_emulationService.reset();
    m_coreOptionRepo.reset();
    m_cheatRepo.reset();
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
    const auto entry = m_library->getEntryWithContentHash(m_hash);
    return entry.has_value() ? entry->id : -1;
  }

  /**
   * Loads the ingested entry and returns its instance, not yet initialized
   */
  EmulatorInstance *loadGame() {
    const int entryId = ingestEntry();
    return entryId == -1 ? nullptr : m_emulationService->loadEntry(entryId).get();
  }
};

} // namespace firelight::emulation

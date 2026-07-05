#include "emulation_service.hpp"
#include <firelight/saves/isave_manager.hpp>

#include <qfile.h>
#include "firelight/event_dispatcher.hpp"
#include <firelight/input/input_service.hpp>
#include <firelight/platforms/platform_service.hpp>

#include <audio/audio_manager.hpp>
#include <firelight/library/content_loader.hpp>
#include <firelight/library/entry_resolver.hpp>
#include <firelight/library/user_library_service.hpp>
#include <firelight/settings/settings_catalog.hpp>
#include <firelight/settings/core_option_repository.hpp>
#include <libretro/core.hpp>
#include <libretro/core_configuration.hpp>
#include <libretro/core_registry.hpp>
#include <platform_metadata.hpp>
#include <spdlog/spdlog.h>

firelight::emulation::EmulationService
*firelight::emulation::EmulationService::s_emuServiceInstance = nullptr;

namespace firelight::emulation {
  EmulationService::EmulationService(library::UserLibraryService &library,
                                     library::EntryResolver &entryResolver,
                                     settings::SettingsService &settingsService,
                                     EmulationContext context,
                                     CoreFactory coreFactory)
    : m_settingsService(settingsService), m_library(library),
      m_resolver(entryResolver), m_context(std::move(context)),
      m_coreFactory(std::move(coreFactory)) {
    // Default factory builds the real dlopen'd Core; tests inject a fake.
    if (!m_coreFactory) {
      m_coreFactory =
          [](int platformId, const std::string &corePath,
             std::shared_ptr<firelight::libretro::IConfigurationProvider>
                 configProvider,
             const std::string &systemDirectory)
          -> std::unique_ptr<::libretro::ICore> {
        return std::make_unique<::libretro::Core>(
            platformId, corePath, configProvider, systemDirectory);
      };
    }

    // Core options are only known once the core declares them during
    // EmulatorInstance::initialize (render thread). That publishes
    // EmulationStartedEvent, at which point we cache the declared options so the
    // advanced editor can list them before the next launch.
    m_emulationStartedConnection =
        EventDispatcher::instance().subscribe<EmulationStartedEvent>(
            [this](const EmulationStartedEvent &) { persistCoreOptions(); });
  }

  void EmulationService::persistCoreOptions() {
    const auto repository = m_context.coreOptionRepository;
    if (!repository || !m_currentCoreConfig) {
      return;
    }
    // Persist under the core actually resolved for this entry (honors any
    // per-platform / per-game core override), so the cache matches what ran.
    const auto coreName = CoreRegistry::instance().resolveCoreName(
        m_currentEntry.platformId, m_currentContentHash);
    if (coreName.empty()) {
      return;
    }

    std::vector<settings::CoreOption> definitions;
    for (const auto &option : m_currentCoreConfig->getOptions()) {
      settings::CoreOption def;
      def.key = option.key;
      def.label = option.label;
      def.description = option.description;
      def.defaultValue = option.defaultValueKey;
      def.category = option.category;
      def.categoryLabel = option.categoryLabel;
      for (const auto &value : option.possibleValues) {
        def.values.push_back({value.key, value.label});
      }
      definitions.push_back(std::move(def));
    }

    if (!definitions.empty()) {
      repository->upsertCoreOptions(coreName, definitions);
    }
  }

  EmulationService::~EmulationService() {
    spdlog::info("[EmulationService] Stopping EmulationService");
  }

  std::future<EmulatorInstance *> EmulationService::loadEntry(int entryId) {
    // Every failure path returns a *ready* future holding nullptr (never a
    // default-constructed, invalid future that would be UB to .get()) and
    // announces the failure so the UI can react.
    const auto failed = [] {
      std::promise<EmulatorInstance *> promise;
      promise.set_value(nullptr);
      EventDispatcher::instance().publish(GameLoadFailedEvent{});
      return promise.get_future();
    };

    if (m_emulatorInstance) {
      stopEmulation();
    }

    spdlog::info("[EmulationService] Loading entry with id {}", entryId);

    auto entry = m_library.getEntry(entryId);

    if (!entry.has_value()) {
      spdlog::warn("[EmulationService] Entry with id {} does not exist", entryId);
      return failed();
    }

    // Pick the most correct content (ROM/disc + optional patch) for this entry.
    const auto resolved = m_resolver.resolve(*entry);
    if (!resolved.valid) {
      spdlog::warn("[EmulationService] No usable content for entry with id {}",
                   entryId);
      return failed();
    }

    const auto &contentFile = resolved.contentFile;

    const auto contentPath =
        contentFile.m_inArchive ? contentFile.m_archivePathName : contentFile.m_filePath;
    if (!std::filesystem::exists(contentPath)) {
      spdlog::error("[EmulationService] Content path doesn't exist: {}",
                    contentPath);
      return failed();
    }

    library::ContentLoader contentLoader;
    auto loaded = contentLoader.load(contentFile);
    if (!loaded.valid) {
      spdlog::error("[EmulationService] Failed to load content for entry {}",
                    entryId);
      return failed();
    }

    if (resolved.patch.has_value()) {
      auto patch = *resolved.patch;
      if (!patch.load()) {
        // The entry resolved to a patched version; running it unpatched would be
        // wrong (and could corrupt saves), so treat this as a load failure.
        spdlog::error("[EmulationService] Failed to load patch {} for entry {}",
                      patch.m_filePath, entryId);
        return failed();
      }
      contentLoader.applyPatch(loaded, contentFile.m_platformId, patch);
    }

    // The one-shot CLI launch overrides apply to this launch only, then are
    // consumed so later launches use the entry's own defaults.
    const LaunchOverrides launch = m_pendingLaunch;
    m_pendingLaunch = {};
    const int saveSlot = launch.saveSlot >= 0
                             ? launch.saveSlot
                             : static_cast<int>(entry->activeSaveSlot);

    QByteArray saveDataBytes;
    if (const auto saveManager = m_context.saveManager) {
      const auto saveData = saveManager->readSaveData(
          QString::fromStdString(loaded.contentHash), saveSlot);
      if (saveData.has_value()) {
        saveDataBytes = QByteArray(saveData->getSaveRamData().data(),
                                   saveData->getSaveRamData().size());
      }
    }

    m_currentEntry = entry.value();
    m_currentContentHash = m_currentEntry.contentHash;

    if (auto platform = platforms::PlatformService::getInstance().getPlatform(
      m_currentEntry.platformId)) {
      m_currentPlatform = platform.value();
    }

    // Resolve the core for this entry (default -> per-platform -> per-game
    // override) and locate its DLL.
    const auto coreName = CoreRegistry::instance().resolveCoreName(
        m_currentEntry.platformId, m_currentContentHash);
    const std::string corePath = CoreRegistry::instance().dllPathFor(coreName);
    const auto &catalog = settings::SettingsCatalog::instance();
    auto coreConfig = std::make_shared<CoreConfiguration>(
      m_currentEntry.contentHash, m_currentEntry.platformId,
      catalog.settingsForCore(coreName), catalog.coreDefaults(coreName),
      m_settingsService);
    m_currentCoreConfig = coreConfig;

    std::unique_ptr<::libretro::ICore> core;
    try {
      core = m_coreFactory(m_currentEntry.platformId, corePath, coreConfig,
                           m_context.coreSystemDirectory);
    } catch (const std::exception &e) {
      spdlog::error("[EmulationService] Failed to load core for entry {}: {}",
                    entryId, e.what());
      return failed();
    }
    if (!core) {
      spdlog::error("[EmulationService] Core factory returned null for entry {}",
                    entryId);
      return failed();
    }

    m_emulatorInstance = std::make_unique<EmulatorInstance>(
      std::move(core), contentFile.m_filePath, m_currentContentHash,
      entry->platformId, saveSlot, std::move(loaded.contentBytes),
      std::vector<uint8_t>(saveDataBytes.begin(), saveDataBytes.end()), m_context);

    // The instance is born muted (if requested) once initialize() creates its
    // AudioManager.
    m_emulatorInstance->setStartMuted(launch.muted);

    EventDispatcher::instance().publish(GameLoadedEvent{});

    std::promise<EmulatorInstance *> promise;
    promise.set_value(m_emulatorInstance.get());
    return promise.get_future();
  }

  void EmulationService::stopEmulation() {
    m_emulatorInstance.reset();
    EventDispatcher::instance().publish(EmulationStoppedEvent{});
  }

  void EmulationService::setPendingLaunchOverrides(LaunchOverrides overrides) {
    m_pendingLaunch = overrides;
  }

  EmulatorInstance *EmulationService::getCurrentEmulatorInstance() {
    return m_emulatorInstance.get();
  }

  bool EmulationService::isGameRunning() const {
    return m_emulatorInstance != nullptr;
  }

  std::optional<std::string> EmulationService::getCurrentGameName() const {
    return isGameRunning() ? m_currentEntry.displayName : "";
  }

  std::optional<library::Entry> EmulationService::getCurrentEntry() {
    return isGameRunning() ? std::optional(m_currentEntry) : std::nullopt;
  }

  std::optional<platforms::Platform>
  EmulationService::getCurrentPlatform() const {
    return m_currentPlatform;
  }
} // namespace firelight::emulation

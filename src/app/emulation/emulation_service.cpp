#include "emulation_service.hpp"
#include "game_loader.hpp"
#include <firelight/saves/isave_manager.hpp>

#include <qfile.h>
#include "firelight/event_dispatcher.hpp"
#include <firelight/input/input_service.hpp>
#include <firelight/platforms/platform_service.hpp>

#include <firelight/library/content_loader.hpp>
#include <firelight/library/entry_resolver.hpp>
#include <firelight/library/user_library_service.hpp>
#include <firelight/settings/settings_catalog.hpp>
#include <firelight/settings/core_option_repository.hpp>
#include <libretro/core.hpp>
#include <libretro/core_configuration.hpp>
#include <libretro/core_registry.hpp>
#include <spdlog/spdlog.h>

firelight::emulation::EmulationService
*firelight::emulation::EmulationService::s_emuServiceInstance = nullptr;

namespace firelight::emulation {
  EmulationService::EmulationService(library::UserLibraryService &library,
                                     library::EntryResolver &entryResolver,
                                     settings::SettingsService &settingsService,
                                     EmulationContext context,
                                     CoreFactory coreFactory)
    : m_settingsService(settingsService), m_context(std::move(context)),
      m_coreFactory(std::move(coreFactory)) {
    // The EmulatorInstance created below inherits this context; make sure it
    // carries the same settings service the service was constructed with,
    // regardless of whether the caller pre-populated the field.
    m_context.settingsService = &m_settingsService;
    // Default factory builds the real dlopen'd Core; tests inject a fake.
    if (!m_coreFactory) {
      m_coreFactory = [](const firelight::libretro::CoreRunConfig &config)
        -> std::unique_ptr<::libretro::ICore> {
            return std::make_unique<::libretro::Core>(config);
          };
    }

    m_loader = std::make_unique<GameLoader>(library, entryResolver,
                                            m_settingsService, m_context);

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
      m_currentEntry.platformId, m_currentContentHash, &m_settingsService);
    if (coreName.empty()) {
      return;
    }

    std::vector<settings::CoreOption> definitions;
    for (const auto &option: m_currentCoreConfig->getOptions()) {
      settings::CoreOption def;
      def.key = option.key;
      def.label = option.label;
      def.description = option.description;
      def.defaultValue = option.defaultValueKey;
      def.category = option.category;
      def.categoryLabel = option.categoryLabel;
      for (const auto &value: option.possibleValues) {
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

    // The one-shot CLI launch overrides apply to this launch only, then are
    // consumed so later launches use the entry's own defaults.
    const LaunchOverrides launch = m_pendingLaunch;
    m_pendingLaunch = {};

    auto result = m_loader->load(entryId, launch, m_coreFactory);
    if (!result.success) {
      return failed();
    }

    m_currentEntry = result.entry;
    m_currentContentHash = result.contentHash;
    if (result.platform) {
      m_currentPlatform = *result.platform;
    }
    m_currentCoreConfig = result.coreConfig;
    {
      std::lock_guard lock(m_instanceMutex);
      m_emulatorInstance = std::move(result.instance);
    }

    EventDispatcher::instance().publish(GameLoadedEvent{});

    std::promise<EmulatorInstance *> promise;
    promise.set_value(m_emulatorInstance.get());
    return promise.get_future();
  }

  void EmulationService::stopEmulation() {
    {
      std::lock_guard lock(m_instanceMutex);
      m_emulatorInstance.reset();
    }
    EventDispatcher::instance().publish(EmulationStoppedEvent{});
  }

  float EmulationService::currentAudioBufferLevel() {
    std::lock_guard lock(m_instanceMutex);
    return m_emulatorInstance ? m_emulatorInstance->getAudioBufferLevel() : -1.0f;
  }

  void EmulationService::resetGame() {
    std::lock_guard lock(m_instanceMutex);
    if (m_emulatorInstance) {
      m_emulatorInstance->reset();
    }
  }

  void EmulationService::setCurrentAudioMuted(const bool muted) {
    std::lock_guard lock(m_instanceMutex);
    if (m_emulatorInstance) {
      m_emulatorInstance->setMuted(muted);
    }
  }

  bool EmulationService::currentAudioMuted() {
    std::lock_guard lock(m_instanceMutex);
    return m_emulatorInstance && m_emulatorInstance->isMuted();
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

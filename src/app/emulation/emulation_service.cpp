// TODO: NEEDS REVIEW
#include "emulation_service.hpp"

#include "firelight/event_dispatcher.hpp"
#include "game_loader.hpp"

#include <firelight/input/input_service.hpp>
#include <firelight/library/content_loader.hpp>
#include <firelight/library/entry_resolver.hpp>
#include <firelight/library/user_library_service.hpp>
#include <firelight/platforms/platform_service.hpp>
#include <firelight/saves/isave_manager.hpp>
#include <firelight/settings/core_option_repository.hpp>
#include <firelight/settings/settings_catalog.hpp>

#include <libretro/core.hpp>
#include <libretro/core_configuration.hpp>
#include <libretro/core_registry.hpp>
#include <qfile.h>
#include <spdlog/spdlog.h>

firelight::emulation::EmulationService *firelight::emulation::EmulationService::s_emuServiceInstance = nullptr;

namespace firelight::emulation {
EmulationService::EmulationService(library::UserLibraryService &library, library::EntryResolver &entryResolver,
                                   settings::SettingsService &settingsService, EmulationContext context,
                                   CoreFactory coreFactory)
    : m_settingsService(settingsService), m_context(std::move(context)), m_coreFactory(std::move(coreFactory)) {

  // Use same settings service for the context as the service itself
  m_context.settingsService = &m_settingsService;

  // Default factory builds the real Core; tests inject a fake
  if (!m_coreFactory) {
    m_coreFactory = [](const libretro::CoreRunConfig &config) -> std::unique_ptr<::libretro::ICore> {
      return std::make_unique<::libretro::Core>(config);
    };
  }

  m_loader = std::make_unique<GameLoader>(library, entryResolver, m_settingsService, m_context);

  m_emulationStartedConnection = EventDispatcher::instance().subscribe<EmulationStartedEvent>(
      [this](const EmulationStartedEvent &) { persistCoreOptions(); });
}

void EmulationService::persistCoreOptions() {
  const auto repository = m_context.coreOptionRepository;
  if (!repository || !m_currentCoreConfig) {
    return;
  }

  const auto coreName =
      CoreRegistry::instance().resolveCoreName(m_currentEntry.platformId, m_currentContentHash, &m_settingsService);
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

EmulationService::~EmulationService() { spdlog::info("[EmulationService] Stopping EmulationService"); }

std::future<EmulatorInstance *> EmulationService::loadEntry(const int entryId) {

  const auto failed = [](std::string reason = {}) {
    std::promise<EmulatorInstance *> promise;
    promise.set_value(nullptr);
    EventDispatcher::instance().publish(GameLoadFailedEvent{.reason = std::move(reason)});
    return promise.get_future();
  };

  if (m_emulatorInstance) {
    stopEmulation();
  }

  // The one-shot CLI launch overrides apply to this launch only, then are consumed so later launches use the entry's
  // own defaults
  const LaunchOverrides launch = m_pendingLaunch;
  m_pendingLaunch = {};

  auto result = m_loader->load(entryId, launch, m_coreFactory);
  if (!result.success) {
    return failed(std::move(result.failureReason));
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
  setSuspended(false);
  {
    std::lock_guard lock(m_instanceMutex);
    m_emulatorInstance.reset();
  }
  EventDispatcher::instance().publish(EmulationStoppedEvent{});
}

void EmulationService::suspend() {
  if (!isGameRunning()) {
    return;
  }

  setSuspended(true);
}

void EmulationService::resume() {
  if (!isGameRunning()) {
    return;
  }

  setSuspended(false);
}

bool EmulationService::isSuspended() const { return m_suspended; }

void EmulationService::setSuspended(const bool suspended) {
  if (m_suspended == suspended) {
    return;
  }

  m_suspended = suspended;
  EventDispatcher::instance().publish(GameSuspendedChangedEvent{.suspended = suspended});
}

float EmulationService::currentAudioBufferLevel() {
  std::lock_guard lock(m_instanceMutex);
  return m_emulatorInstance ? m_emulatorInstance->getAudioBufferLevel() : -1.0f;
}

void EmulationService::resetGame() { submitToCurrentEmulator({.type = EmulatorCommandType::ResetGame}); }

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

bool EmulationService::submitToCurrentEmulator(const EmulatorCommand &command) {
  std::lock_guard lock(m_instanceMutex);

  if (!m_emulatorInstance) {
    return false;
  }

  m_emulatorInstance->submitCommand(command);

  return true;
}

bool EmulationService::isCurrentEmulatorReady() {
  std::lock_guard lock(m_instanceMutex);
  return m_emulatorInstance && m_emulatorInstance->isInitialized();
}

void EmulationService::setPendingLaunchOverrides(const LaunchOverrides overrides) { m_pendingLaunch = overrides; }

EmulatorInstance *EmulationService::getCurrentEmulatorInstance() const { return m_emulatorInstance.get(); }

std::weak_ptr<EmulatorInstance> EmulationService::getCurrentEmulatorInstanceHandle() {
  std::lock_guard lock(m_instanceMutex);
  return m_emulatorInstance;
}

bool EmulationService::isGameRunning() const { return m_emulatorInstance != nullptr; }

std::optional<std::string> EmulationService::getCurrentGameName() const {
  return isGameRunning() ? m_currentEntry.displayName : "";
}

std::optional<library::Entry> EmulationService::getCurrentEntry() {
  return isGameRunning() ? std::optional(m_currentEntry) : std::nullopt;
}

std::optional<platforms::Platform> EmulationService::getCurrentPlatform() const { return m_currentPlatform; }
} // namespace firelight::emulation

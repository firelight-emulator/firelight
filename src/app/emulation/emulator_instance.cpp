#include "emulator_instance.hpp"
#include <rcheevos/ra_client.hpp>
#include <firelight/saves/isave_manager.hpp>

#include "emulation_service.hpp"
#include "firelight/event_dispatcher.hpp"
#include <firelight/input/input_service.hpp>
#include <libretro/game.hpp>

#include <spdlog/spdlog.h>

#include <audio/audio_manager.hpp>
#include <firelight/settings/settings_catalog.hpp>
#include <firelight/settings/settings_service.hpp>
#include <utility>

namespace firelight::emulation {
EmulatorInstance::EmulatorInstance(
    std::unique_ptr<::libretro::ICore> core, std::string contentPath,
    std::string contentHash, const int platformId, const int saveSlotNumber,
    std::vector<uint8_t> gameData, std::vector<uint8_t> saveData,
    EmulationContext context)
    : m_context(std::move(context)), m_core(std::move(core)),
      m_gameData(std::move(gameData)), m_saveData(std::move(saveData)),
      m_contentPath(std::move(contentPath)),
      m_contentHash(std::move(contentHash)), m_platformId(platformId),
      m_saveSlotNumber(saveSlotNumber) {
  m_lastSaveTime = std::chrono::steady_clock::now();

  // Settings resolve by inheritance (game -> platform -> global -> default), so
  // a change at ANY tier can alter this game's effective value: refresh on a
  // platform change for our platform, a game change for our hash, or any global
  // change.
  m_platformSettingChangedConnection =
      EventDispatcher::instance()
          .subscribe<settings::PlatformSettingChangedEvent>(
              [this](const settings::PlatformSettingChangedEvent &e) {
                if (e.platformId != m_platformId) {
                  return;
                }
                refreshAllSettings();
              });

  m_gameSettingChangedConnection =
      EventDispatcher::instance().subscribe<settings::GameSettingChangedEvent>(
          [this](const settings::GameSettingChangedEvent &e) {
            if (e.contentHash != m_contentHash) {
              return;
            }
            refreshAllSettings();
          });

  m_globalSettingChangedConnection =
      EventDispatcher::instance()
          .subscribe<settings::GlobalSettingChangedEvent>(
              [this](const settings::GlobalSettingChangedEvent &) {
                refreshAllSettings();
              });

  refreshAllSettings();
}

EmulatorInstance::~EmulatorInstance() {
  spdlog::info("[EmulatorInstance] Shutting down");

  // Unsubscribe first: the settings callbacks capture `this` and touch members
  // (m_core, ...). Disconnecting before we tear anything down ensures a
  // late-arriving settings event can't fire into a partially-destroyed instance.
  m_platformSettingChangedConnection = {};
  m_gameSettingChangedConnection = {};
  m_globalSettingChangedConnection = {};

  // Restore device-default controller profiles when the game unloads.
  if (const auto inputService = m_context.inputService) {
    inputService->clearGameContext();
  }
  save().wait();
}

bool EmulatorInstance::initialize(
    libretro::IVideoDataReceiver *videoDataReceiver) {
  m_audioManager = std::make_unique<AudioManager>([] {});
  // Apply the requested initial mute now that the AudioManager exists (a QML
  // muted binding fires before this and would otherwise be lost).
  m_audioManager->setMuted(m_startMuted);

  m_core->setVideoReceiver(videoDataReceiver);
  m_core->setAudioReceiver(m_audioManager);
  m_core->setRetropadProvider(m_context.inputService);
  m_core->setPointerInputProvider(m_context.inputService);
  m_core->setSystemDirectory(m_context.coreSystemDirectory);
  m_core->init();

  ::libretro::Game game(m_contentPath, m_gameData);
  m_core->loadGame(&game);

  if (m_saveData.size() > 0) {
    m_core->writeMemoryData(
        ::libretro::SAVE_RAM,
        std::vector<char>(m_saveData.begin(), m_saveData.end()));
  }

  if (const auto achievements = m_context.achievementManager) {
    achievements->loadGame(m_platformId,
                           QString::fromStdString(m_contentHash));
  }

  // Apply per-game controller profile override + platform preferred controller.
  if (const auto inputService = m_context.inputService) {
    inputService->applyGameContext(m_contentHash, m_platformId);
  }

  m_initialized = true;

  EventDispatcher::instance().publish(EmulationStartedEvent{
      .contentHash = m_contentHash, .saveSlotNumber = m_saveSlotNumber});
  return true;
}
bool EmulatorInstance::isInitialized() { return m_initialized; }
std::string EmulatorInstance::getContentHash() const { return m_contentHash; }
int EmulatorInstance::getPlatformId() const { return m_platformId; }
int EmulatorInstance::getSaveSlotNumber() const { return m_saveSlotNumber; }
void EmulatorInstance::runFrame() {
  const auto now = std::chrono::steady_clock::now();
  // spdlog::info("Comparing {} and {}: {}", now.time_since_epoch().count(),
  //              m_lastSaveTime.time_since_epoch().count(),
  //              (now - std::chrono::seconds(m_saveIntervalSeconds))
  //                  .time_since_epoch()
  //                  .count());
  if (now - std::chrono::seconds(m_saveIntervalSeconds) > m_lastSaveTime) {
    m_lastSaveTime = now;
    save();
  }

  m_core->run(0);
  if (const auto achievements = m_context.achievementManager) {
    achievements->doFrame(m_core.get());
  }
}
void EmulatorInstance::reset() {
  m_core->reset();
  if (const auto achievements = m_context.achievementManager) {
    achievements->reset();
  }
}

std::future<bool> EmulatorInstance::save() {
  if (!m_initialized) {
    return std::async(std::launch::deferred, [] { return false; });
  }
  saves::Savefile saveData(m_core->getMemoryData(::libretro::SAVE_RAM));
  // if (!m_currentImage.isNull() && m_currentImage.width() > 0 &&
  //     m_currentImage.height() > 0) {
  //   saveData.setImage(m_currentImage.copy());
  // }
  return m_context.saveManager->writeSaveData(m_contentHash.data(),
                                              m_saveSlotNumber, saveData);
}

void EmulatorInstance::setMuted(const bool muted) {
  if (!m_audioManager) {
    return;
  }
  m_audioManager->setMuted(muted);
}
void EmulatorInstance::setPaused(const bool paused) {
  if (!m_audioManager) {
    return;
  }
  m_audioManager->setPaused(paused);
}
bool EmulatorInstance::isMuted() const {
  if (!m_audioManager) {
    return false;
  }
  return m_audioManager->isMuted();
}
void EmulatorInstance::setRewindEnabled(const bool enabled) {
  m_isRewindEnabled = enabled;
}

bool EmulatorInstance::isRewindEnabled() const {
  if (!m_isRewindEnabled) {
    return false;
  }

  if (m_context.achievementManager->loggedIn() &&
      m_context.achievementManager->hardcoreModeActive()) {
    return false;
  }

  return true;
}
void EmulatorInstance::setPictureMode(const std::string &pictureMode) {
  m_pictureMode = pictureMode;
}
std::string EmulatorInstance::getPictureMode() const { return m_pictureMode; }
void EmulatorInstance::setAspectRatioMode(const std::string &aspectRatioMode) {
  m_aspectRatioMode = aspectRatioMode;
}
std::string EmulatorInstance::getAspectRatioMode() const {
  return m_aspectRatioMode;
}
void EmulatorInstance::setIntegerScale(const int integerScale) {
  m_integerScale = integerScale;
}
int EmulatorInstance::getIntegerScale() const { return m_integerScale; }
void EmulatorInstance::setSyncMethod(const std::string &syncMethod) {
  m_syncMethod = syncMethod;
}
std::string EmulatorInstance::getSyncMethod() const { return m_syncMethod; }
void EmulatorInstance::setTargetFramerate(const int targetFramerate) {
  m_targetFramerate = targetFramerate;
}
int EmulatorInstance::getTargetFramerate() const { return m_targetFramerate; }
float EmulatorInstance::getAudioBufferLevel() const {
  return m_audioManager ? m_audioManager->getBufferLevel() : -1.0f;
}
void EmulatorInstance::setAudioPlaybackRateRatio(const double ratio) {
  if (m_audioManager) {
    m_audioManager->setPlaybackRateRatio(ratio);
  }
}

std::vector<uint8_t> EmulatorInstance::serializeState() {
  return m_core->serializeState();
}
void EmulatorInstance::deserializeState(const std::vector<uint8_t> &state) {
  m_core->deserializeState(state);
}

void EmulatorInstance::refreshAllSettings() {
  auto *service = settings::SettingsService::instance();
  const auto &catalog = settings::SettingsCatalog::instance();

  // Effective value for a common setting: the resolved override
  // (game -> platform -> global), else the catalog-declared default. The catalog
  // JSON is the single source of truth for these defaults.
  const auto value = [&](const std::string &key) {
    return service->getEffectiveValue(m_contentHash, m_platformId, key)
        .value_or(catalog.defaultForCommonKey(key));
  };
  const auto intValue = [&](const std::string &key) {
    try {
      return std::stoi(value(key));
    } catch (const std::exception &) {
      return 0; // missing/non-numeric (e.g. catalog not loaded) -> neutral
    }
  };
  const auto apply = [&](const std::string &key, auto &&setter) {
    setter();
    EventDispatcher::instance().publish(settings::EmulationSettingChangedEvent{
        .contentHash = m_contentHash, .key = key});
  };

  apply("rewind-enabled",
        [&] { setRewindEnabled(value("rewind-enabled") == "true"); });
  apply("picture-mode", [&] { setPictureMode(value("picture-mode")); });
  apply("aspect-ratio", [&] { setAspectRatioMode(value("aspect-ratio")); });
  apply("integer-scale", [&] { setIntegerScale(intValue("integer-scale")); });
  apply("sync-method", [&] { setSyncMethod(value("sync-method")); });
  apply("target-framerate",
        [&] { setTargetFramerate(intValue("target-framerate")); });
}
} // namespace firelight::emulation
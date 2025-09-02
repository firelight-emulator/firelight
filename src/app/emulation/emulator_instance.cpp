#include "emulator_instance.hpp"

#include "emulation_service.hpp"
#include "event_dispatcher.hpp"
#include "input2/input_service.hpp"

#include <spdlog/spdlog.h>

#include <audio/audio_manager.hpp>
#include <settings/settings_service.hpp>
#include <utility>

namespace firelight::emulation {
EmulatorInstance::EmulatorInstance(
    std::unique_ptr<::libretro::Core> core, std::string contentPath,
    std::string contentHash, const int platformId, const int saveSlotNumber,
    std::vector<uint8_t> gameData, std::vector<uint8_t> saveData)
    : m_core(std::move(core)), m_gameData(std::move(gameData)),
      m_saveData(std::move(saveData)), m_contentPath(std::move(contentPath)),
      m_contentHash(std::move(contentHash)), m_platformId(platformId),
      m_saveSlotNumber(saveSlotNumber) {
  m_lastSaveTime = std::chrono::steady_clock::now();

  // m_currentSettingsLevel =
  //     settings::SettingsService::instance()->getSettingsLevel(m_contentHash);

  m_currentSettingsLevel = settings::SettingsLevel::Game;

  m_platformSettingChangedConnection =
      EventDispatcher::instance()
          .subscribe<settings::PlatformSettingChangedEvent>(
              [this](const settings::PlatformSettingChangedEvent &e) {
                if (e.platformId != m_platformId) {
                  return;
                }

                if (m_currentSettingsLevel !=
                    settings::SettingsLevel::Platform) {
                  return;
                }

                refreshAllSettings();
              });

  m_gameSettingChangedConnection =
      EventDispatcher::instance().subscribe<settings::GameSettingChangedEvent>(
          [this](const settings::GameSettingChangedEvent &e) {
            spdlog::info("Game setting changed event for {} (current {})",
                         e.contentHash, m_contentHash);
            if (e.contentHash != m_contentHash) {
              return;
            }

            if (m_currentSettingsLevel != settings::SettingsLevel::Game) {
              return;
            }

            refreshAllSettings();
          });

  m_settingsLevelChangedConnection =
      EventDispatcher::instance()
          .subscribe<settings::SettingsLevelChangedEvent>(
              [this](const settings::SettingsLevelChangedEvent &e) {
                if (e.contentHash != m_contentHash) {
                  return;
                }

                m_currentSettingsLevel = e.level;

                refreshAllSettings();
              });

  refreshAllSettings();
}

EmulatorInstance::~EmulatorInstance() {
  spdlog::info("[EmulatorInstance] Shutting down");
  save().wait();
}

bool EmulatorInstance::initialize(
    libretro::IVideoDataReceiver *videoDataReceiver) {
  m_audioManager = std::make_unique<AudioManager>([] {});

  m_core->setVideoReceiver(videoDataReceiver);
  m_core->setAudioReceiver(m_audioManager);
  m_core->setRetropadProvider(input::InputService::instance());
  m_core->setPointerInputProvider(input::InputService::instance());
  m_core->setSystemDirectory(getCoreSystemDirectory());
  m_core->init();

  ::libretro::Game game(m_contentPath, m_gameData);
  m_core->loadGame(&game);

  if (m_saveData.size() > 0) {
    m_core->writeMemoryData(
        ::libretro::SAVE_RAM,
        std::vector<char>(m_saveData.begin(), m_saveData.end()));
  }

  getAchievementManager()->loadGame(m_platformId,
                                    QString::fromStdString(m_contentHash));

  m_initialized = true;

  EventDispatcher::instance().publish(EmulationStartedEvent{
      .contentHash = m_contentHash, .saveSlotNumber = m_saveSlotNumber});
  return true;
}
bool EmulatorInstance::isInitialized() { return m_initialized; }
std::string EmulatorInstance::getContentHash() const { return m_contentHash; }
int EmulatorInstance::getPlatformId() const { return m_platformId; }
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
  getAchievementManager()->doFrame(m_core.get());
}
void EmulatorInstance::reset() {
  m_core->reset();
  getAchievementManager()->reset();
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
  return getSaveManager()->writeSaveData(m_contentHash.data(), m_saveSlotNumber,
                                         saveData);
}

void EmulatorInstance::setMuted(const bool muted) {
  if (!m_audioManager) {
    return;
  }
  m_audioManager->setMuted(muted);
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

  if (getAchievementManager()->loggedIn() &&
      getAchievementManager()->hardcoreModeActive()) {
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

std::vector<uint8_t> EmulatorInstance::serializeState() {
  return m_core->serializeState();
}
void EmulatorInstance::deserializeState(const std::vector<uint8_t> &state) {
  m_core->deserializeState(state);
}
::libretro::Core *EmulatorInstance::getCore() { return m_core.get(); }

void EmulatorInstance::refreshAllSettings() {

  // Rewind enabled
  setRewindEnabled(settings::SettingsService::instance()
                       ->getValue(m_currentSettingsLevel, m_contentHash,
                                  m_platformId, "rewind-enabled")
                       .value_or("true") == "true");

  EventDispatcher::instance().publish(settings::EmulationSettingChangedEvent{
      .contentHash = m_contentHash, .key = "rewind-enabled"});

  // Picture mode
  setPictureMode(settings::SettingsService::instance()
                     ->getValue(m_currentSettingsLevel, m_contentHash,
                                m_platformId, "picture-mode")
                     .value_or("aspect-ratio-fill"));

  EventDispatcher::instance().publish(settings::EmulationSettingChangedEvent{
      .contentHash = m_contentHash, .key = "picture-mode"});

  // Aspect ratio mode
  setAspectRatioMode(settings::SettingsService::instance()
                         ->getValue(m_currentSettingsLevel, m_contentHash,
                                    m_platformId, "aspect-ratio")
                         .value_or("emulator-corrected"));

  EventDispatcher::instance().publish(settings::EmulationSettingChangedEvent{
      .contentHash = m_contentHash, .key = "aspect-ratio"});
}
} // namespace firelight::emulation
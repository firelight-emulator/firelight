#include "emulator_instance.hpp"
#include <rcheevos/ra_client.hpp>
#include <firelight/saves/isave_manager.hpp>

#include <firelight/cheats/cheat_repository.hpp>
#include "emulation_service.hpp"
#include "firelight/event_dispatcher.hpp"
#include <firelight/input/input_service.hpp>
#include <libretro/game.hpp>

#include <spdlog/spdlog.h>

#include <audio/audio_manager.hpp>
#include <firelight/settings/settings_catalog.hpp>
#include <firelight/settings/settings_service.hpp>
#include <filesystem>
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

  // Managed, per-game/per-slot save directory: cores that write their own files
  // (PPSSPP memstick, N64 pak files, DS NAND) get a Firelight-owned path under
  // the saves tree so those saves are organized/backed up per slot, not dumped
  // in the system dir.
  if (const auto saveManager = m_context.saveManager) {
    const auto base = saveManager->getSaveDirectory().toStdString();
    if (!base.empty()) {
      const auto coreSaveDir = base + "/" + m_contentHash + "/slot" +
                               std::to_string(m_saveSlotNumber) + "/core";
      std::error_code ec;
      std::filesystem::create_directories(coreSaveDir, ec);
      m_core->setSaveDirectory(coreSaveDir);
    }
  }

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

  // Announce the disc set for multi-disc content so subscribers (disc UI, etc.)
  // learn the count without polling.
  if (const auto count = m_core->getDiskCount(); count > 1) {
    EventDispatcher::instance().publish(
        DiscChangedEvent{.contentHash = m_contentHash,
                         .index = m_core->getCurrentDiskIndex(),
                         .count = count});
  }

  // Resolve + apply the per-port controller variant (the game's default unless
  // overridden), driving the core device + any companion core options, and
  // announce availability so the input UI can offer a choice.
  if (const auto portCount = getControllerPortCount(); portCount > 0) {
    for (unsigned port = 0; port < portCount; ++port) {
      const auto variant = resolveSelectedVariant(port);
      m_core->setControllerPortDevice(port, variant.coreDeviceId);
      m_core->setPortInputDeviceClass(port,
                                      static_cast<int>(variant.deviceClass));
      applyCompanionOptions(variant);
    }
    EventDispatcher::instance().publish(
        ControllerDevicesEvent{.contentHash = m_contentHash});
  }

  applyCheats();
  return true;
}
bool EmulatorInstance::isInitialized() { return m_initialized; }
std::string EmulatorInstance::getContentHash() const { return m_contentHash; }
int EmulatorInstance::getPlatformId() const { return m_platformId; }
int EmulatorInstance::getSaveSlotNumber() const { return m_saveSlotNumber; }

unsigned EmulatorInstance::getDiscCount() const {
  return m_core ? m_core->getDiskCount() : 0;
}

unsigned EmulatorInstance::getCurrentDiscIndex() const {
  return m_core ? m_core->getCurrentDiskIndex() : 0;
}

bool EmulatorInstance::swapDisc(const unsigned index) {
  if (!m_core || !m_core->setDiskIndex(index)) {
    return false;
  }
  EventDispatcher::instance().publish(
      DiscChangedEvent{.contentHash = m_contentHash,
                       .index = index,
                       .count = m_core->getDiskCount()});
  return true;
}

std::vector<std::vector<::libretro::ICore::ControllerDeviceOption>>
EmulatorInstance::getControllerDevices() const {
  return m_core ? m_core->getControllerDevices()
                : std::vector<
                      std::vector<::libretro::ICore::ControllerDeviceOption>>{};
}

std::vector<CoreDeviceVariant>
EmulatorInstance::getAvailableControllerVariants(const unsigned port) const {
  if (!m_core) {
    return {};
  }
  const auto coreId =
      CoreRegistry::instance().resolveCoreName(m_platformId, m_contentHash);
  const auto devices = m_core->getControllerDevices();
  std::vector<unsigned> advertised;
  if (port < devices.size()) {
    for (const auto &d : devices[port]) {
      advertised.push_back(d.id);
    }
  }
  return CoreRegistry::instance().availableControllerVariants(coreId, advertised);
}

unsigned EmulatorInstance::getControllerPortCount() const {
  if (!m_core) {
    return 0;
  }
  const auto advertised =
      static_cast<unsigned>(m_core->getControllerDevices().size());
  if (advertised > 0) {
    return advertised;
  }
  // The core didn't advertise its ports via SET_CONTROLLER_INFO, but our catalog
  // may still define alternate devices for it (e.g. FCEUmm's Zapper). Expose a
  // small default port count so the player can still pick one; the picker hides
  // ports with no real choice anyway.
  const auto coreId =
      CoreRegistry::instance().resolveCoreName(m_platformId, m_contentHash);
  if (!CoreRegistry::instance().deviceCatalogForCore(coreId).empty()) {
    return 2;
  }
  return 0;
}

unsigned
EmulatorInstance::getSelectedControllerVariant(const unsigned port) const {
  return resolveSelectedVariant(port).coreDeviceId;
}

CoreDeviceVariant
EmulatorInstance::resolveSelectedVariant(const unsigned port) const {
  const auto variants = getAvailableControllerVariants(port);
  if (variants.empty()) {
    return CoreDeviceVariant{}; // standard joypad
  }

  // Per-game override (a stored coreDeviceId), honored only if still offered by
  // the loaded core (a stale value from another core falls back to the default).
  if (auto *settings = settings::SettingsService::instance()) {
    if (const auto v = settings->getEffectiveValue(
            m_contentHash, m_platformId,
            "port" + std::to_string(port) + "-controllervariant")) {
      try {
        const auto id = static_cast<unsigned>(std::stoul(*v));
        for (const auto &variant : variants) {
          if (variant.coreDeviceId == id) {
            return variant;
          }
        }
      } catch (const std::exception &) {
        // Malformed stored value; fall through to the default.
      }
    }
  }

  for (const auto &variant : variants) {
    if (variant.isDefault) {
      return variant;
    }
  }
  return variants.front();
}

void EmulatorInstance::applyCompanionOptions(const CoreDeviceVariant &variant) {
  auto *settings = settings::SettingsService::instance();
  if (!settings) {
    return;
  }
  // Companion options take effect when the core reads its options (on init);
  // the current session already runs the core's default, and the selection
  // persists for the next launch.
  for (const auto &[key, value] : variant.companionOptions) {
    settings->setGameValue(m_contentHash, key, value);
  }
}

void EmulatorInstance::setPortControllerVariant(const unsigned port,
                                                const unsigned coreDeviceId) {
  if (!m_core) {
    return;
  }
  m_core->setControllerPortDevice(port, coreDeviceId);
  if (auto *settings = settings::SettingsService::instance()) {
    settings->setGameValue(
        m_contentHash, "port" + std::to_string(port) + "-controllervariant",
        std::to_string(coreDeviceId));
  }
  for (const auto &variant : getAvailableControllerVariants(port)) {
    if (variant.coreDeviceId == coreDeviceId) {
      m_core->setPortInputDeviceClass(port,
                                      static_cast<int>(variant.deviceClass));
      applyCompanionOptions(variant);
      break;
    }
  }
}

void EmulatorInstance::applyCheats() {
  if (!m_core) {
    return;
  }
  m_core->clearCheats();
  m_cheatEngine.clear();
  if (!m_context.cheatRepository) {
    return;
  }

  // While RA hardcore mode is active, gameplay-affecting cheats are disallowed.
  const bool hardcore = m_context.achievementManager &&
                        m_context.achievementManager->hardcoreModeActive();

  std::vector<cheats::CheatPoke> pokes;
  unsigned coreIndex = 0;
  for (const auto &cheat : m_context.cheatRepository->getCheats(m_contentHash)) {
    if (!cheat.enabled || (hardcore && cheat.affectsHardcore)) {
      continue;
    }
    if (cheat.isCoreApplied()) {
      // Game Genie / emu-handler: the core must decode it (ROM substitution).
      m_core->setCheat(coreIndex++, true, cheat.rawCode);
    } else {
      // RAM cheats: Firelight replays these pokes every frame.
      pokes.insert(pokes.end(), cheat.pokes.begin(), cheat.pokes.end());
    }
  }
  m_cheatEngine.setActivePokes(std::move(pokes));
}

std::vector<cheats::Cheat> EmulatorInstance::getCheats() const {
  if (!m_context.cheatRepository) {
    return {};
  }
  return m_context.cheatRepository->getCheats(m_contentHash);
}

void EmulatorInstance::setCheatEnabled(const int cheatId, const bool enabled) {
  if (!m_context.cheatRepository) {
    return;
  }
  m_context.cheatRepository->setEnabled(cheatId, enabled);
  applyCheats();
}

void EmulatorInstance::addCheat(cheats::Cheat cheat) {
  if (!m_context.cheatRepository) {
    return;
  }
  cheat.contentHash = m_contentHash;
  m_context.cheatRepository->addCheat(cheat);
  applyCheats();
}

void EmulatorInstance::updateCheat(const cheats::Cheat &cheat) {
  if (!m_context.cheatRepository) {
    return;
  }
  m_context.cheatRepository->updateCheat(cheat);
  applyCheats();
}

void EmulatorInstance::removeCheat(const int cheatId) {
  if (!m_context.cheatRepository) {
    return;
  }
  m_context.cheatRepository->removeCheat(cheatId);
  applyCheats();
}
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
  // Re-apply RAM cheats after each frame so values the game overwrites stick.
  m_cheatEngine.apply(*m_core);
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
  // Maps the friendly pointer-speed choice to a per-frame glide step (fraction
  // of the full ±32767 cursor range moved per frame at full stick deflection).
  const auto pointerSpeed = [&](const std::string &v) -> double {
    if (v == "slow") {
      return 0.015;
    }
    if (v == "fast") {
      return 0.040;
    }
    return 0.025; // medium / default
  };

  apply("rewind-enabled",
        [&] { setRewindEnabled(value("rewind-enabled") == "true"); });
  apply("picture-mode", [&] { setPictureMode(value("picture-mode")); });
  apply("aspect-ratio", [&] { setAspectRatioMode(value("aspect-ratio")); });
  apply("integer-scale", [&] { setIntegerScale(intValue("integer-scale")); });
  apply("sync-method", [&] { setSyncMethod(value("sync-method")); });
  apply("target-framerate",
        [&] { setTargetFramerate(intValue("target-framerate")); });
  apply("analog-pointer-speed", [&] {
    if (m_core) {
      m_core->setAnalogPointerSpeed(pointerSpeed(value("analog-pointer-speed")));
    }
  });
  apply("mouse-controls-lightgun", [&] {
    if (m_core) {
      m_core->setMouseControlsPointerDevices(
          value("mouse-controls-lightgun") == "true");
    }
  });
}
} // namespace firelight::emulation
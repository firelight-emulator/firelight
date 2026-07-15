#include "emulator_instance.hpp"
#include "core_settings_applier.hpp"
#include <firelight/achievements/iachievement_client.hpp>
#include <firelight/saves/isave_manager.hpp>

#include <firelight/cheats/cheat_repository.hpp>
#include "emulation_service.hpp"
#include "firelight/event_dispatcher.hpp"
#include <firelight/input/input_frame.hpp>
#include <firelight/input/input_service.hpp>
#include <libretro/game.hpp>

#include <spdlog/spdlog.h>

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

    // Subscribes to setting changes and applies this game's resolved common
    // settings now (all members it touches are initialized above).
    m_settingsApplier = std::make_unique<CoreSettingsApplier>(
      *this, m_context, m_contentHash, m_platformId);
  }

  EmulatorInstance::~EmulatorInstance() {
    spdlog::info("[EmulatorInstance] Shutting down");

    // Destroy the settings applier first: its subscriptions capture `this` and
    // touch members, so a late event mustn't fire into a half-torn-down instance.
    m_settingsApplier.reset();

    // Restore device-default controller profiles when the game unloads.
    if (const auto inputService = m_context.inputService) {
      inputService->clearGameContext();
    }
    save().wait();
  }

  bool EmulatorInstance::initialize(
    libretro::IVideoDataReceiver *videoDataReceiver) {
    // Audio output + microphone are injected as factories (main.cpp supplies the
    // Qt-Multimedia impls); both are created here on the render thread for Qt
    // audio thread-affinity. Null in headless/tests -> no audio.
    if (m_context.audioOutputFactory) {
      m_audioOutput = m_context.audioOutputFactory();
      // Apply the requested initial mute now that the output exists (a QML muted
      // binding fires before this and would otherwise be lost).
      m_audioOutput->setMuted(m_startMuted);
      // Apply the resolved DRC setting (refreshAllSettings may have run before
      // the output existed, so it only stored the value on this instance).
      m_audioOutput->setDynamicRateControlEnabled(m_dynamicRateControl);
      m_core->setAudioReceiver(m_audioOutput);
    }

    m_core->setVideoReceiver(videoDataReceiver);
    m_core->setRetropadProvider(m_context.inputService);
    m_core->setPointerInputProvider(m_context.inputService);
    if (m_context.audioInputFactory) {
      m_audioInput = m_context.audioInputFactory();
      m_core->setAudioInputProvider(m_audioInput.get());
    }
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
      achievements->loadGame(m_platformId, m_contentHash);
    }

    // Apply per-game controller profile override + platform preferred controller.
    if (const auto inputService = m_context.inputService) {
      inputService->applyGameContext(m_contentHash, m_platformId);
    }

    m_initialized = true;

    EventDispatcher::instance().publish(EmulationStartedEvent{
      .contentHash = m_contentHash, .saveSlotNumber = m_saveSlotNumber
    });

    // Announce the disc set for multi-disc content so subscribers (disc UI, etc.)
    // learn the count without polling.
    if (const auto count = m_core->getDiskCount(); count > 1) {
      EventDispatcher::instance().publish(
        DiscChangedEvent{
          .contentHash = m_contentHash,
          .index = m_core->getCurrentDiskIndex(),
          .count = count
        });
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
      DiscChangedEvent{
        .contentHash = m_contentHash,
        .index = index,
        .count = m_core->getDiskCount()
      });
    return true;
  }

  std::vector<std::vector<::libretro::ICore::ControllerDeviceOption> >
  EmulatorInstance::getControllerDevices() const {
    return m_core
             ? m_core->getControllerDevices()
             : std::vector<
               std::vector<::libretro::ICore::ControllerDeviceOption> >{};
  }

  std::vector<CoreDeviceVariant>
  EmulatorInstance::getAvailableControllerVariants(const unsigned port) const {
    if (!m_core) {
      return {};
    }
    const auto coreId =
        CoreRegistry::instance().resolveCoreName(m_platformId, m_contentHash,
                                                 m_context.settingsService);
    const auto devices = m_core->getControllerDevices();
    std::vector<unsigned> advertised;
    if (port < devices.size()) {
      for (const auto &d: devices[port]) {
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
        CoreRegistry::instance().resolveCoreName(m_platformId, m_contentHash,
                                                 m_context.settingsService);
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
    if (auto *settings = m_context.settingsService) {
      if (const auto v = settings->getEffectiveValue(
        m_contentHash, m_platformId,
        "port" + std::to_string(port) + "-controllervariant")) {
        try {
          const auto id = static_cast<unsigned>(std::stoul(*v));
          for (const auto &variant: variants) {
            if (variant.coreDeviceId == id) {
              return variant;
            }
          }
        } catch (const std::exception &) {
          // Malformed stored value; fall through to the default.
        }
      }
    }

    for (const auto &variant: variants) {
      if (variant.isDefault) {
        return variant;
      }
    }
    return variants.front();
  }

  void EmulatorInstance::applyCompanionOptions(const CoreDeviceVariant &variant) {
    auto *settings = m_context.settingsService;
    if (!settings) {
      return;
    }
    // Companion options take effect when the core reads its options (on init);
    // the current session already runs the core's default, and the selection
    // persists for the next launch.
    for (const auto &[key, value]: variant.companionOptions) {
      settings->setGameValue(m_contentHash, key, value);
    }
  }

  void EmulatorInstance::setPortControllerVariant(const unsigned port,
                                                  const unsigned coreDeviceId) {
    if (!m_core) {
      return;
    }
    m_core->setControllerPortDevice(port, coreDeviceId);
    if (auto *settings = m_context.settingsService) {
      settings->setGameValue(
        m_contentHash, "port" + std::to_string(port) + "-controllervariant",
        std::to_string(coreDeviceId));
    }
    for (const auto &variant: getAvailableControllerVariants(port)) {
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
    for (const auto &cheat: m_context.cheatRepository->getCheats(m_contentHash)) {
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
    // In TAS mode the periodic autosave is suppressed so runFrame() is a pure
    // emulation step (no wall-clock-triggered disk IO to perturb a replay).
    if (!m_tasMode &&
        now - std::chrono::seconds(m_saveIntervalSeconds) > m_lastSaveTime) {
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

  void EmulatorInstance::setRetropadProvider(
      firelight::libretro::IRetropadProvider *provider) {
    if (m_core) {
      m_core->setRetropadProvider(provider);
    }
  }

  void EmulatorInstance::restoreLiveRetropadProvider() {
    if (m_core) {
      m_core->setRetropadProvider(m_context.inputService);
    }
  }

  uint16_t EmulatorInstance::captureCurrentInputButtons(const int port) const {
    if (!m_context.inputService) {
      return 0;
    }
    const auto pad = m_context.inputService->getRetropadForPlayerIndex(port);
    if (!pad) {
      return 0;
    }
    // Same sampling the CoreInputRouter does each frame, so the recorded input
    // matches what the core reads.
    return input::captureJoypadFrame(*pad, m_platformId, /*controllerTypeId=*/1)
        .buttons;
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
    if (!m_audioOutput) {
      return;
    }
    m_audioOutput->setMuted(muted);
  }

  void EmulatorInstance::setPaused(const bool paused) {
    if (!m_audioOutput) {
      return;
    }
    m_audioOutput->setPaused(paused);
  }

  bool EmulatorInstance::isMuted() const {
    if (!m_audioOutput) {
      return false;
    }
    return m_audioOutput->isMuted();
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
    return m_audioOutput ? m_audioOutput->getBufferLevel() : -1.0f;
  }

  void EmulatorInstance::setAudioPlaybackRateRatio(const double ratio) {
    if (m_audioOutput) {
      m_audioOutput->setPlaybackRateRatio(ratio);
    }
  }

  void EmulatorInstance::setDynamicRateControlEnabled(const bool enabled) {
    m_dynamicRateControl = enabled;
    if (m_audioOutput) {
      m_audioOutput->setDynamicRateControlEnabled(enabled);
    }
  }

  bool EmulatorInstance::getDynamicRateControlEnabled() const {
    return m_dynamicRateControl;
  }

  void EmulatorInstance::setInstantReplayEnabled(const bool enabled) {
    m_instantReplayEnabled = enabled;
  }

  bool EmulatorInstance::getInstantReplayEnabled() const {
    return m_instantReplayEnabled;
  }

  std::vector<uint8_t> EmulatorInstance::serializeState() {
    return m_core->serializeState();
  }

  bool EmulatorInstance::deserializeState(const std::vector<uint8_t> &state) {
    return m_core->deserializeState(state);
  }

  void EmulatorInstance::setAnalogPointerSpeed(const double stepPerFrame) {
    if (m_core) {
      m_core->setAnalogPointerSpeed(stepPerFrame);
    }
  }

  void EmulatorInstance::setMouseControlsPointerDevices(const bool enabled) {
    if (m_core) {
      m_core->setMouseControlsPointerDevices(enabled);
    }
  }
} // namespace firelight::emulation

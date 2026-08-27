#include "emulator_instance.hpp"

#include "core_settings_applier.hpp"
#include "emulation_service.hpp"
#include "firelight/event_dispatcher.hpp"

#include <firelight/achievements/iachievement_client.hpp>
#include <firelight/cheats/cheat_repository.hpp>
#include <firelight/input/input_service.hpp>
#include <firelight/input/keyboard_keycodes.hpp>
#include <firelight/saves/isave_manager.hpp>
#include <firelight/settings/settings_catalog.hpp>
#include <firelight/settings/settings_service.hpp>

#include <chrono>
#include <filesystem>
#include <libretro/game.hpp>
#include <spdlog/spdlog.h>
#include <utility>

namespace firelight::emulation {
EmulatorInstance::EmulatorInstance(std::unique_ptr<::libretro::ICore> core, std::string contentPath,
                                   std::string contentHash, const int platformId, const int saveSlotNumber,
                                   std::vector<uint8_t> gameData, std::vector<uint8_t> saveData,
                                   EmulationContext context)
    : m_context(std::move(context)), m_core(std::move(core)), m_gameData(std::move(gameData)),
      m_saveData(std::move(saveData)), m_contentPath(std::move(contentPath)), m_contentHash(std::move(contentHash)),
      m_platformId(platformId), m_saveSlotNumber(saveSlotNumber) {
  m_lastSaveTime = std::chrono::steady_clock::now();

  // Subscribes to setting changes and applies this game's resolved common
  // settings now (all members it touches are initialized above)
  m_settingsApplier = std::make_unique<CoreSettingsApplier>(*this, m_context, m_contentHash, m_platformId);

  // Keys for a core that reads the keyboard. Queued rather than delivered
  // here: this fires on the GUI thread, and the core is the render thread's
  m_keyboardKeyConnection =
      EventDispatcher::instance().subscribe<input::KeyboardKeyEvent>([this](const input::KeyboardKeyEvent &event) {
        const auto key = input::retroKeyFor(event.key);
        if (key == input::RETROK_UNKNOWN) {
          return;
        }
        std::lock_guard lock(m_pendingKeysMutex);
        m_pendingKeys.push_back({event.pressed, key, 0});
      });
}

void EmulatorInstance::setHotkeysDisabled(const bool disabled) {
  m_hotkeysDisabled = disabled;
  applyHotkeyState();
}

void EmulatorInstance::applyHotkeyState() {
  const auto inputService = m_context.inputService;
  if (!inputService) {
    return;
  }

  if (m_hotkeysDisabled) {
    inputService->setHotkeysEnabled(false);
    return;
  }

  inputService->setHotkeysEnabled(true);

  // Nothing turned them off by hand, so honour the core: one that asked for
  // the keyboard gets it, and only the keyboard — a controller keeps its
  // hotkeys. This is what makes it unnecessary to know in advance which
  // systems need the keys
  const auto autoDisable =
      m_context.settingsService &&
      m_context.settingsService->getGlobalValue("auto-disable-keyboard-hotkeys").value_or("true") == "true";
  if (autoDisable && m_core && m_core->wantsKeyboard()) {
    inputService->setHotkeysEnabled(false, DeviceType::Keyboard);
  }
}

void EmulatorInstance::drainKeyboardEvents() {
  // Most cores never ask for the keyboard, so this costs one atomic read a
  // frame for them
  if (!m_core->wantsKeyboard()) {
    std::lock_guard lock(m_pendingKeysMutex);
    m_pendingKeys.clear();
    return;
  }

  std::vector<PendingKey> keys;
  {
    std::lock_guard lock(m_pendingKeysMutex);
    keys.swap(m_pendingKeys);
  }
  for (const auto &[down, key, modifiers] : keys) {
    m_core->sendKeyboardEvent(down, key, 0, modifiers);
  }
}

EmulatorInstance::~EmulatorInstance() {
  spdlog::info("[EmulatorInstance] Shutting down");

  // Destroy the settings applier first: its subscriptions capture `this` and
  // touch members, so a late event mustn't fire into a half-torn-down instance
  m_settingsApplier.reset();

  // Restore device-default controller profiles when the game unloads, and
  // hand back any hotkeys this game turned off — the menu needs them
  if (const auto inputService = m_context.inputService) {
    inputService->clearGameContext();
    inputService->setHotkeysEnabled(true);
  }
  save().wait();
}

bool EmulatorInstance::initialize(libretro::IVideoDataReceiver *videoDataReceiver) {
  // Audio output + microphone are injected as factories (main.cpp supplies the
  // Qt-Multimedia impls); both are created here on the render thread for Qt
  // audio thread-affinity. Null in headless/tests -> no audio
  if (m_context.audioOutputFactory) {
    m_audioOutput = m_context.audioOutputFactory(m_contentHash, m_platformId);
    // TODO
    // Through the same two flags everything else uses, because a binding that fired before the
    // output existed has already set one of them and writing past it would strand that value
    m_mutedByRequest = m_mutedByRequest || m_startMuted;
    applyMuted();
    // Apply the resolved DRC setting (refreshAllSettings may have run before
    // the output existed, so it only stored the value on this instance)
    applyAudioRateControl();
    m_core->setAudioReceiver(m_audioOutput);
  }

  m_core->setVideoReceiver(videoDataReceiver);
  m_core->setRetropadProvider(m_context.retropadProvider
                                  ? m_context.retropadProvider
                                  : static_cast<libretro::IRetropadProvider *>(m_context.inputService));
  m_core->setPointerInputProvider(m_context.inputService);
  if (m_context.audioInputFactory) {
    m_audioInput = m_context.audioInputFactory();
    m_core->setAudioInputProvider(m_audioInput.get());
  }
  m_core->setSystemDirectory(m_context.coreSystemDirectory);

  m_core->init();

  ::libretro::Game game(m_contentPath, m_gameData);
  m_core->loadGame(&game);

  // The core registers its keyboard callback while loading, so only now can
  // the auto-disable see whether this one wants the keys
  applyHotkeyState();

  if (m_saveData.size() > 0) {
    m_core->writeMemoryData(::libretro::SAVE_RAM, std::vector<char>(m_saveData.begin(), m_saveData.end()));
  }

  if (const auto achievements = m_context.achievementManager) {
    achievements->loadGame(m_platformId, m_contentHash);
  }

  // Apply per-game controller profile override + platform preferred controller
  if (const auto inputService = m_context.inputService) {
    inputService->applyGameContext(m_contentHash, m_platformId);
  }

  m_initialized = true;

  EventDispatcher::instance().publish(
      EmulationStartedEvent{.contentHash = m_contentHash, .saveSlotNumber = m_saveSlotNumber});

  // Announce the disc set for multi-disc content so subscribers (disc UI, etc.)
  // learn the count without polling
  if (const auto count = m_core->getDiskCount(); count > 1) {
    EventDispatcher::instance().publish(
        DiscChangedEvent{.contentHash = m_contentHash, .index = m_core->getCurrentDiskIndex(), .count = count});
  }

  // Resolve + apply the per-port controller variant (the game's default unless
  // overridden), driving the core device + any companion core options, and
  // announce availability so the input UI can offer a choice
  if (const auto portCount = getControllerPortCount(); portCount > 0) {
    for (unsigned port = 0; port < portCount; ++port) {
      const auto variant = resolveSelectedVariant(port);
      m_core->setControllerPortDevice(port, variant.coreDeviceId);
      m_core->setPortInputDeviceClass(port, static_cast<int>(variant.deviceClass));
      applyCompanionOptions(variant);
    }
    EventDispatcher::instance().publish(ControllerDevicesEvent{.contentHash = m_contentHash});
  }

  applyCheats();
  return true;
}

bool EmulatorInstance::isInitialized() { return m_initialized; }

std::string EmulatorInstance::getContentHash() const { return m_contentHash; }

int EmulatorInstance::getPlatformId() const { return m_platformId; }

int EmulatorInstance::getSaveSlotNumber() const { return m_saveSlotNumber; }

unsigned EmulatorInstance::getDiscCount() const { return m_core ? m_core->getDiskCount() : 0; }

unsigned EmulatorInstance::getCurrentDiscIndex() const { return m_core ? m_core->getCurrentDiskIndex() : 0; }

bool EmulatorInstance::swapDisc(const unsigned index) {
  if (!m_core || !m_core->setDiskIndex(index)) {
    return false;
  }
  EventDispatcher::instance().publish(
      DiscChangedEvent{.contentHash = m_contentHash, .index = index, .count = m_core->getDiskCount()});
  return true;
}

std::vector<std::vector<::libretro::ICore::ControllerDeviceOption>> EmulatorInstance::getControllerDevices() const {
  return m_core ? m_core->getControllerDevices()
                : std::vector<std::vector<::libretro::ICore::ControllerDeviceOption>>{};
}

std::vector<CoreDeviceVariant> EmulatorInstance::getAvailableControllerVariants(const unsigned port) const {
  if (!m_core) {
    return {};
  }
  const auto coreId = CoreRegistry::instance().resolveCoreName(m_platformId, m_contentHash, m_context.settingsService);
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
  const auto advertised = static_cast<unsigned>(m_core->getControllerDevices().size());
  if (advertised > 0) {
    return advertised;
  }
  // The core didn't advertise its ports via SET_CONTROLLER_INFO, but our catalog
  // may still define alternate devices for it (e.g. FCEUmm's Zapper). Expose a
  // small default port count so the player can still pick one; the picker hides
  // ports with no real choice anyway
  const auto coreId = CoreRegistry::instance().resolveCoreName(m_platformId, m_contentHash, m_context.settingsService);
  if (!CoreRegistry::instance().deviceCatalogForCore(coreId).empty()) {
    return 2;
  }
  return 0;
}

unsigned EmulatorInstance::getSelectedControllerVariant(const unsigned port) const {
  return resolveSelectedVariant(port).coreDeviceId;
}

CoreDeviceVariant EmulatorInstance::resolveSelectedVariant(const unsigned port) const {
  const auto variants = getAvailableControllerVariants(port);
  if (variants.empty()) {
    return CoreDeviceVariant{}; // standard joypad
  }

  // Per-game override (a stored coreDeviceId), honored only if still offered by
  // the loaded core (a stale value from another core falls back to the default)
  if (auto *settings = m_context.settingsService) {
    if (const auto v = settings->getEffectiveValue(m_contentHash, m_platformId,
                                                   "port" + std::to_string(port) + "-controllervariant")) {
      try {
        const auto id = static_cast<unsigned>(std::stoul(*v));
        for (const auto &variant : variants) {
          if (variant.coreDeviceId == id) {
            return variant;
          }
        }
      } catch (const std::exception &) {
        // Malformed stored value; fall through to the default
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
  auto *settings = m_context.settingsService;
  if (!settings) {
    return;
  }
  // Companion options take effect when the core reads its options (on init);
  // the current session already runs the core's default, and the selection
  // persists for the next launch
  for (const auto &[key, value] : variant.companionOptions) {
    settings->setGameValue(m_contentHash, key, value);
  }
}

void EmulatorInstance::setPortControllerVariant(const unsigned port, const unsigned coreDeviceId) {
  if (!m_core) {
    return;
  }
  m_core->setControllerPortDevice(port, coreDeviceId);
  if (auto *settings = m_context.settingsService) {
    settings->setGameValue(m_contentHash, "port" + std::to_string(port) + "-controllervariant",
                           std::to_string(coreDeviceId));
  }
  for (const auto &variant : getAvailableControllerVariants(port)) {
    if (variant.coreDeviceId == coreDeviceId) {
      m_core->setPortInputDeviceClass(port, static_cast<int>(variant.deviceClass));
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

  // While RA hardcore mode is active, gameplay-affecting cheats are disallowed
  const bool hardcore = m_context.achievementManager && m_context.achievementManager->hardcoreModeActive();

  std::vector<cheats::CheatPoke> pokes;
  unsigned coreIndex = 0;
  for (const auto &cheat : m_context.cheatRepository->getCheats(m_contentHash)) {
    if (!cheat.enabled || (hardcore && cheat.affectsHardcore)) {
      continue;
    }
    if (cheat.isCoreApplied()) {
      // Game Genie / emu-handler: the core must decode it (ROM substitution)
      m_core->setCheat(coreIndex++, true, cheat.rawCode);
    } else {
      // RAM cheats: Firelight replays these pokes every frame
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
  drainKeyboardEvents();

  const auto now = std::chrono::steady_clock::now();
  // spdlog::info("Comparing {} and {}: {}", now.time_since_epoch().count(),
  //              m_lastSaveTime.time_since_epoch().count(),
  //              (now - std::chrono::seconds(m_saveIntervalSeconds))
  //                  .time_since_epoch()
  //                  .count());
  if (now - std::chrono::seconds(m_saveIntervalSeconds) > m_lastSaveTime) {
    m_lastSaveTime = now;
    // Keep the future so its destructor doesn't block this (render) thread
    // until the write completes. The write runs on its own copy of the data
    m_pendingSave = save();
  }

  m_core->run(0);
  // Re-apply RAM cheats after each frame so values the game overwrites stick
  m_cheatEngine.apply(*m_core);
  if (const auto achievements = m_context.achievementManager) {
    achievements->doFrame(m_core.get());
  }
}

void EmulatorInstance::submitCommand(const EmulatorCommand &command) {
  std::lock_guard lock(m_commandQueueMutex);
  m_commandQueue.push_back(command);
}

void EmulatorInstance::drainCommands() {
  std::deque<EmulatorCommand> pending;

  {
    std::lock_guard lock(m_commandQueueMutex);
    pending.swap(m_commandQueue);
  }

  // Handled without the lock: serializing a state is heavy, and whoever queued the next command
  // shouldn't wait behind it
  while (!pending.empty()) {
    const auto command = pending.front();
    pending.pop_front();
    handleCommand(command);
  }
}

SuspendPoint EmulatorInstance::capturePoint() {
  SuspendPoint point;
  point.state = serializeState();
  point.timestamp =
      std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
          .count();
  point.saveSlot = m_saveSlotNumber;

  if (m_thumbnailProvider) {
    point.image = m_thumbnailProvider();
  }

  if (const auto achievements = m_context.achievementManager) {
    point.retroachievementsState = achievements->serializeState();
  }

  return point;
}

void EmulatorInstance::restoreFrame(const Image &image) {
  if (m_frameRestorer && !image.isNull()) {
    m_frameRestorer(image);
  }
}

void EmulatorInstance::handleCommand(const EmulatorCommand &command) {
  switch (command.type) {
  case EmulatorCommandType::RunFrame:
    // TODO
    // A hardware core draws into the graphics context whatever is showing the game holds, so the
    // frame has to run there. With nothing showing it, this side runs it
    if (m_commandSink) {
      m_commandSink(command);
      break;
    }

    runFrame();
    break;

  case EmulatorCommandType::WriteRewindPoint: {
    // Rolling snapshots take whatever picture is on hand rather than forcing a readback, which
    // stalled hardware cores every few seconds. The state is always current either way
    m_rewindPoints.push_front(capturePoint());

    if (m_rewindPoints.size() > MAX_REWIND_POINTS) {
      m_rewindPoints.pop_back();
    }
  } break;

  case EmulatorCommandType::LoadRewindPoint: {
    const auto index = static_cast<size_t>(command.rewindPointIndex - 1);

    if (index >= m_rewindPoints.size()) {
      break;
    }

    const auto &point = m_rewindPoints.at(index);
    deserializeState(point.state);

    if (const auto achievements = m_context.achievementManager; achievements && !point.retroachievementsState.empty()) {
      achievements->deserializeState(point.retroachievementsState);
    }

    restoreFrame(point.image);
  } break;

  case EmulatorCommandType::WriteSuspendPoint: {
    if (!m_context.saveManager) {
      break;
    }

    m_context.saveManager->writeSuspendPoint(m_contentHash, m_saveSlotNumber, command.suspendPointIndex,
                                             capturePoint());
  } break;

  case EmulatorCommandType::LoadSuspendPoint: {
    if (!m_context.saveManager) {
      break;
    }

    const auto point =
        m_context.saveManager->readSuspendPoint(m_contentHash, m_saveSlotNumber, command.suspendPointIndex);
    if (!point.has_value()) {
      break;
    }

    // What was replaced is kept so it can be put back, which is the whole of undo
    m_beforeLastLoadSuspendPoint = capturePoint();

    deserializeState(point->state);

    if (const auto achievements = m_context.achievementManager;
        achievements && !point->retroachievementsState.empty()) {
      achievements->deserializeState(point->retroachievementsState);
    }

    restoreFrame(point->image);
  } break;

  case EmulatorCommandType::UndoLoadSuspendPoint: {
    if (m_beforeLastLoadSuspendPoint.state.empty()) {
      break;
    }

    deserializeState(m_beforeLastLoadSuspendPoint.state);

    if (const auto achievements = m_context.achievementManager;
        achievements && !m_beforeLastLoadSuspendPoint.retroachievementsState.empty()) {
      achievements->deserializeState(m_beforeLastLoadSuspendPoint.retroachievementsState);
    }

    restoreFrame(m_beforeLastLoadSuspendPoint.image);
    m_beforeLastLoadSuspendPoint = {};
  } break;

  case EmulatorCommandType::SetPlaybackMultiplier:
    setSpeedMultiplier(command.playbackMultiplier);
    if (m_commandSink) {
      m_commandSink(command);
    }
    break;

  case EmulatorCommandType::EmitRewindPoints:
  case EmulatorCommandType::CaptureScreenshot:
  case EmulatorCommandType::CaptureVideoClip:
    // Pixels off a GPU, or an image provider QML reads: none of that is the emulator's to do
    if (m_commandSink) {
      m_commandSink(command);
    }
    break;
  }
}

void EmulatorInstance::reset() {
  m_core->reset();
  if (const auto achievements = m_context.achievementManager) {
    achievements->reset();
  }
}

std::future<bool> EmulatorInstance::save() {
  if (!m_initialized || !m_context.saveManager) {
    return std::async(std::launch::deferred, [] { return false; });
  }
  saves::Savefile saveData(m_core->getMemoryData(::libretro::SAVE_RAM));
  return m_context.saveManager->writeSaveData(m_contentHash.data(), m_saveSlotNumber, saveData);
}

void EmulatorInstance::setMuted(const bool muted) {
  m_mutedByRequest = muted;
  applyMuted();
}

void EmulatorInstance::setSpeedMultiplier(const float multiplier) {
  m_mutedBySpeed = shouldMuteAtSpeed(multiplier);
  applyMuted();

  // TODO
  // Heard at speed means heard at pitch: the core hands over as much sound per emulated second
  // whatever the speed, so playing it in less time is what makes it fit. Without this the sink is
  // handed twice the sound it can take and spends the fast forward full and the recovery draining
  m_speedAudioRatio = m_mutedBySpeed ? 1.0 : multiplier;
  applyAudioRatio();
}

void EmulatorInstance::applyMuted() const {
  if (!m_audioOutput) {
    return;
  }
  m_audioOutput->setMuted(m_mutedByRequest || m_mutedBySpeed);
}

bool EmulatorInstance::shouldMuteAtSpeed(const float multiplier) const {
  if (multiplier == 1.0f || m_context.settingsService == nullptr) {
    return false;
  }

  // TODO
  // Sound produced faster than it can be played has to lose either its pitch or its samples, and
  // slower than it is played leaves gaps. Silence is the third option, and the default
  const auto *key = multiplier > 1.0f ? "mute-on-fast-forward" : "mute-on-slow-motion";
  return m_context.settingsService->getEffectiveValue(m_contentHash, m_platformId, key).value_or("true") != "false";
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

void EmulatorInstance::setRewindEnabled(const bool enabled) { m_isRewindEnabled = enabled; }

bool EmulatorInstance::isRewindEnabled() const {
  if (!m_isRewindEnabled) {
    return false;
  }

  if (m_context.achievementManager->loggedIn() && m_context.achievementManager->hardcoreModeActive()) {
    return false;
  }

  return true;
}

void EmulatorInstance::setPictureMode(const std::string &pictureMode) { m_pictureMode = pictureMode; }

std::string EmulatorInstance::getPictureMode() const { return m_pictureMode; }

void EmulatorInstance::setAspectRatioMode(const std::string &aspectRatioMode) { m_aspectRatioMode = aspectRatioMode; }

std::string EmulatorInstance::getAspectRatioMode() const { return m_aspectRatioMode; }

void EmulatorInstance::setIntegerScale(const int integerScale) { m_integerScale = integerScale; }

int EmulatorInstance::getIntegerScale() const { return m_integerScale; }

void EmulatorInstance::setSyncMethod(const std::string &syncMethod) { m_syncMethod = syncMethod; }

std::string EmulatorInstance::getSyncMethod() const { return m_syncMethod; }

void EmulatorInstance::setTargetFramerate(const int targetFramerate) { m_targetFramerate = targetFramerate; }

int EmulatorInstance::getTargetFramerate() const { return m_targetFramerate; }

float EmulatorInstance::getAudioBufferLevel() const { return m_audioOutput ? m_audioOutput->getBufferLevel() : -1.0f; }

void EmulatorInstance::setAudioPlaybackRateRatio(const double ratio) {
  m_pacingAudioRatio = ratio;
  applyAudioRatio();
}

void EmulatorInstance::applyAudioRatio() const {
  if (m_audioOutput) {
    m_audioOutput->setPlaybackRateRatio(m_pacingAudioRatio * m_speedAudioRatio);
  }
}

void EmulatorInstance::setDynamicRateControlEnabled(const bool enabled) {
  m_dynamicRateControl = enabled;
  applyAudioRateControl();
}

void EmulatorInstance::applyAudioRateControl() const {
  if (m_audioOutput) {
    m_audioOutput->setDynamicRateControlEnabled(m_dynamicRateControl);
  }
}

bool EmulatorInstance::getDynamicRateControlEnabled() const { return m_dynamicRateControl; }

void EmulatorInstance::setInstantReplayEnabled(const bool enabled) { m_instantReplayEnabled = enabled; }

bool EmulatorInstance::getInstantReplayEnabled() const { return m_instantReplayEnabled; }

std::vector<uint8_t> EmulatorInstance::serializeState() { return m_core->serializeState(); }

bool EmulatorInstance::deserializeState(const std::vector<uint8_t> &state) { return m_core->deserializeState(state); }

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

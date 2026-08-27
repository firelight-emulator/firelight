#include "emulator_item.hpp"

#include "diagnostics/performance_stats.hpp"
#include "emulation/emulation_service.hpp"
#include "emulation/shortcut_actions.hpp"
#include "emulator_item_renderer.hpp"

#include <firelight/discord/discord_manager.hpp>
#include <firelight/input/input_service.hpp>
#include <firelight/library/content_file.hpp>
#include <firelight/settings/settings_service.hpp>

#include <QQuickWindow>
#include <QScreen>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <mutex>
#include <patching/bps_patch.hpp>
#include <patching/ups_patch.hpp>
#include <rhi/qrhi_platform.h>
#include <spdlog/spdlog.h>

void EmulatorItem::feedPointer(const QPointF &pos) {
  const auto bounds = boundingRect();
  // Off-screen is authoritatively derived from whether the cursor is actually
  // over the game surface (light guns read this; when off-screen some cores —
  // e.g. FCEUmm's Zapper — zero the aim). Clamp the normalized position to
  // [-1, 1] so an out-of-bounds drag can't overflow the int16 the core reads
  const bool offscreen = !bounds.contains(pos);
  const auto x = std::clamp((pos.x() - bounds.width() / 2) / (bounds.width() / 2), -1.0, 1.0);
  const auto y = std::clamp((pos.y() - bounds.height() / 2) / (bounds.height() / 2), -1.0, 1.0);

  auto *input = getInputService();
  input->updateMouseState(x, y, m_mousePressed);
  input->updateMouseOffscreen(offscreen);

  // Relative motion for RETRO_DEVICE_MOUSE: pixel delta since the last event
  if (m_hasLastMousePos) {
    input->updateMouseMotion(static_cast<int>(std::lround(pos.x() - m_lastMousePos.x())),
                             static_cast<int>(std::lround(pos.y() - m_lastMousePos.y())));
  }
  m_lastMousePos = pos;
  m_hasLastMousePos = true;
}

void EmulatorItem::mouseMoveEvent(QMouseEvent *event) { feedPointer(event->position()); }

namespace {

// TODO
// What the mode is called in the log, in terms of what it paces against rather than what it is named
const char *pacedAgainst(const firelight::emulation::SyncMode mode) {
  switch (mode) {
  case firelight::emulation::SyncMode::Auto:
    return "auto";
  case firelight::emulation::SyncMode::Audio:
    return "the audio sink";
  case firelight::emulation::SyncMode::Fixed:
    return "a clock";
  case firelight::emulation::SyncMode::Display:
    return "display refreshes";
  }

  return "nothing";
}
} // namespace

EmulatorItem::EmulatorItem(QQuickItem *parent) : QQuickRhiItem(parent) {
  // The emulator a hotkey acts on. Registered from here rather than when the
  // game starts, because the ScopeAlways actions can fire before then
  if (const auto actions = getShortcutActions()) {
    actions->setController(this);
  }

  connect(this, &QQuickItem::windowChanged, this, [this](QQuickWindow *w) {
    if (w == nullptr) {
      return;
    }

    // TODO
    // frameSwapped says a frame was queued for the display, not that the display showed one — Qt
    // emits it as soon as the present is submitted, and the wait for the refresh happens elsewhere.
    // It is still the closest thing to a refresh available, and RefreshCounter is written to take a
    // gap shorter than one refresh because of it. Direct
    // because it arrives on the render thread and there is nothing here that needs the GUI's
    disconnect(m_frameSwappedConnection);
    m_frameSwappedConnection = connect(
        w, &QQuickWindow::frameSwapped, this,
        [this] {
          const auto nowNs = std::chrono::steady_clock::now().time_since_epoch().count();

          if (const auto gapNs = m_pacer.noteSubmit(nowNs); gapNs > 0) {
            firelight::diagnostics::PerformanceStats::instance().recordSubmit(gapNs);
          }

          m_loopWake.notify_one();

          // TODO
          // Counting the frames that reach the display only works while they keep arriving, and one
          // only arrives if something asked to draw. Safe here and nowhere else, because this runs
          // on the thread that draws
          if (m_renderContinuously.load()) {
            update();
          }
        },
        Qt::DirectConnection);

    if (auto *screen = w->screen()) {
      connect(screen, &QScreen::refreshRateChanged, this,
              [this](qreal) { QMetaObject::invokeMethod(this, "reconfigurePacing", Qt::QueuedConnection); });
    }

    connect(w, &QWindow::screenChanged, this,
            [this](QScreen *) { QMetaObject::invokeMethod(this, "reconfigurePacing", Qt::QueuedConnection); });
  });

  setFlag(ItemHasContents);
  setAcceptHoverEvents(true);
  setAcceptTouchEvents(true);
  setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton | Qt::MiddleButton);

  m_threadPool.setMaxThreadCount(1);

  // Re-pace when the sync method or target framerate changes (any tier)
  m_settingChangedConnection = EventDispatcher::instance().subscribe<firelight::settings::EmulationSettingChangedEvent>(
      [this](const firelight::settings::EmulationSettingChangedEvent &e) {
        if (e.key == "sync-method" || e.key == "target-framerate") {
          QMetaObject::invokeMethod(this, "reconfigurePacing", Qt::QueuedConnection);
        }
      });

  m_rewindPointTimer.setInterval(3000);
  m_rewindPointTimer.setSingleShot(false);
  connect(&m_rewindPointTimer, &QTimer::timeout, [this] {
    // TODO
    // A game that isn't advancing is the same state every time this fires, and ten of those would be
    // the whole history — a minute in the quick menu would leave nothing to rewind to
    if (m_renderer && !m_paused && m_pacer.getResolvedMode() != firelight::emulation::SyncMode::Audio) {
      submitToEmulator({.type = firelight::emulation::EmulatorCommandType::WriteRewindPoint});
      update();
    }
  });
  m_rewindPointTimer.start();

  m_emulationThread.setServiceLevel(QThread::QualityOfService::High);

  // The loop is the thread: it waits for the next frame to be due, runs it, and goes back to
  // waiting. Nothing else is posted here, so there is no event loop to run
  connect(&m_emulationThread, &QThread::started, this, [this] { runEmulationLoop(); }, Qt::DirectConnection);

  m_emulationThread.start();
  m_emulationThread.setPriority(QThread::TimeCriticalPriority);
}

void EmulatorItem::submitToEmulator(const firelight::emulation::EmulatorCommand &command) {
  firelight::emulation::EmulationService::getInstance()->submitToCurrentEmulator(command);
}

void EmulatorItem::waitForNextFrame() {
  const auto deadlineNs = m_pacer.getNextDeadlineNs();

  if (deadlineNs == 0) {
    // Nothing has established a cadence yet — the first frame of a game, or a mode that hasn't been
    // configured. Wait a moment rather than spinning
    std::unique_lock lock(m_loopMutex);
    // Audio asks the sink often enough that a frame is never late by more than this, and Display
    // is woken by a refresh rather than the timeout
    m_loopWake.wait_for(lock, std::chrono::milliseconds(1), [this] { return m_emulationStopping.load(); });
    return;
  }

  const auto marginNs = m_waiter.getSpinMarginNs();
  const auto sleepUntilNs = deadlineNs - marginNs;
  const auto nowNs = std::chrono::steady_clock::now().time_since_epoch().count();

  if (sleepUntilNs > nowNs) {
    if (!m_waiter.sleepFor(sleepUntilNs - nowNs)) {
      std::unique_lock lock(m_loopMutex);
      m_loopWake.wait_for(lock, std::chrono::nanoseconds(sleepUntilNs - nowNs),
                          [this] { return m_emulationStopping.load(); });
    }

    const auto overshootNs = std::chrono::steady_clock::now().time_since_epoch().count() - sleepUntilNs;
    firelight::diagnostics::PerformanceStats::instance().recordWake(marginNs, overshootNs);
    m_waiter.noteOvershoot(overshootNs);
  }

  // The last stretch is spun rather than slept, because a sleep that overshoots costs a frame where
  // presentation follows production. A margin of 0 makes this a no-op
  while (!m_emulationStopping && std::chrono::steady_clock::now().time_since_epoch().count() < deadlineNs) {
  }
}

void EmulatorItem::runEmulationLoop() {
  while (!m_emulationStopping) {
    waitForNextFrame();

    if (m_emulationStopping) {
      return;
    }

    const auto nowNs = std::chrono::steady_clock::now().time_since_epoch().count();
    auto *emulation = firelight::emulation::EmulationService::getInstance();

    m_pacer.setPaused(m_paused);
    m_pacer.setReady(emulation->isCurrentEmulatorReady());

    if (m_pacer.getResolvedMode() == firelight::emulation::SyncMode::Audio) {
      m_pacer.setAudioBufferLevel(emulation->currentAudioBufferLevel());
    }

    const auto decision = m_pacer.tick(nowNs);

    if (decision.shouldRequestRender && decision.framesToRun == 0) {
      QMetaObject::invokeMethod(this, "update", Qt::QueuedConnection);
    }

    if (decision.framesToRun == 0) {
      continue;
    }

    const auto frames = decision.framesToRun;

    // TODO
    // The frame itself runs on the render thread, inside the pass that puts it on screen — deciding
    // when is all that happens here. Every frame owed is handed over, because a pass runs as many as
    // it is given; how far behind it is worth trying to catch up was already decided above
    for (auto frame = 0; frame < frames; ++frame) {
      submitToEmulator({.type = firelight::emulation::EmulatorCommandType::RunFrame});
    }

    QMetaObject::invokeMethod(this, "update", Qt::QueuedConnection);
  }
}

EmulatorItem::~EmulatorItem() {
  // TODO
  // First, before anything else here: the handler below touches members of this object from the
  // render thread, and that thread keeps presenting until the window goes. Leaving it to ~QObject
  // runs it after every one of those members has been destroyed
  disconnect(m_frameSwappedConnection);

  m_stopping = true;
  if (const auto actions = getShortcutActions()) {
    actions->setController(nullptr);
  }
  getDiscordManager()->clearActivity();

  // Wake the loop so it sees the flag rather than sleeping out its current wait, then let the
  // thread finish the frame it may be in the middle of
  m_emulationStopping = true;
  m_loopWake.notify_all();
  m_emulationThread.quit();
  m_emulationThread.wait();

  m_threadPool.waitForDone();
}

bool EmulatorItem::paused() const { return m_paused; }

void EmulatorItem::setPaused(const bool paused) {
  if (m_paused != paused) {
    m_paused = paused;
    emit pausedChanged();
    update();
  }
}

void EmulatorItem::advanceOneFrame() {
  if (!m_renderer) {
    return;
  }
  // Pause first so the pacing thread stops queueing frames of its own, then
  // hand the renderer exactly one — the same command the pacer would have sent
  setPaused(true);
  submitToEmulator({.type = firelight::emulation::EmulatorCommandType::RunFrame});
  update();
}

bool EmulatorItem::isRewindEnabled() const { return m_rewindEnabled; }

void EmulatorItem::setRewindEnabled(const bool rewindEnabled) {
  if (m_rewindEnabled == rewindEnabled) {
    return;
  }

  m_rewindEnabled = rewindEnabled;
  emit rewindEnabledChanged();

  if (rewindEnabled && !m_rewindPointTimer.isActive()) {
    m_rewindPointTimer.start();
  } else if (!rewindEnabled && m_rewindPointTimer.isActive()) {
    m_rewindPointTimer.stop();
  }
}

bool EmulatorItem::isMuted() const { return m_muted; }

void EmulatorItem::setMuted(const bool muted) {
  // TODO
  // Compared against what was last asked for rather than against what is currently heard. The two
  // differ whenever something else has a reason to silence the game, and a request skipped because
  // the answer already looked right is a request that never arrives
  if (m_muted == muted) {
    return;
  }

  m_muted = muted;
  firelight::emulation::EmulationService::getInstance()->setCurrentAudioMuted(muted);
  emit mutedChanged();
}

float EmulatorItem::audioBufferLevel() const {
  const auto emulator = firelight::emulation::EmulationService::getInstance()->getCurrentEmulatorInstance();
  if (!emulator) {
    return 0.0f;
  }
  const float level = emulator->getAudioBufferLevel();
  return level >= 0.0f ? level : 0.0f;
}

EmulatorItem::SyncMethod EmulatorItem::syncMethodFromString(const std::string &method) {
  if (method == "auto") {
    return SyncMethod::Auto;
  }
  if (method == "monitor") {
    return SyncMethod::Monitor;
  }
  if (method == "fixed") {
    return SyncMethod::Fixed;
  }
  if (method == "audio") {
    return SyncMethod::Audio;
  }
  return SyncMethod::Native;
}

void EmulatorItem::reconfigurePacing() {
  const auto emulator = firelight::emulation::EmulationService::getInstance()->getCurrentEmulatorInstance();
  if (!emulator) {
    return;
  }

  const auto method = syncMethodFromString(emulator->getSyncMethod());
  const double coreFps = m_coreFps.load() > 0.0 ? m_coreFps.load() : 60.0;
  const int targetFramerate = emulator->getTargetFramerate();

  // TODO
  // 0 rather than a guess: the controller reads it as "no display to pace against" and picks
  // something that doesn't depend on one, which a made-up 60 would have talked it out of
  double refreshHz = 0.0;
  bool presentationLocked = false;

  if (const auto *w = window()) {
    if (const auto *screen = w->screen(); screen && screen->refreshRate() > 0.0) {
      refreshHz = screen->refreshRate();
    }

    // TODO
    // The same thing Qt reads to decide whether to ask the swapchain for vsync, so it is also what
    // decides whether a present stands for a refresh
    presentationLocked = w->requestedFormat().swapInterval() != 0;
  }

  // Audio sync needs a working audio device; without one it is just a clock at the core's rate
  const bool audioAvailable = emulator->getAudioBufferLevel() >= 0.0f;

  firelight::emulation::PacingContext context;
  context.contentFps = coreFps;
  context.displayHz = refreshHz;
  context.presentationLocked = presentationLocked;

  switch (method) {
  case SyncMethod::Auto:
    // TODO
    // Which mode this becomes is the controller's to work out, from the same three facts
    context.mode = firelight::emulation::SyncMode::Auto;
    break;
  case SyncMethod::Audio:
    context.mode = audioAvailable ? firelight::emulation::SyncMode::Audio : firelight::emulation::SyncMode::Fixed;
    break;
  case SyncMethod::Monitor:
    context.mode = firelight::emulation::SyncMode::Display;
    break;
  case SyncMethod::Fixed:
    context.mode = firelight::emulation::SyncMode::Fixed;
    context.contentFps = targetFramerate > 0 ? static_cast<double>(targetFramerate) : coreFps;
    break;
  case SyncMethod::Native:
    context.mode = firelight::emulation::SyncMode::Fixed;
    break;
  }

  firelight::emulation::SyncMode resolved{};
  bool followingDisplay = false;
  double effectiveFps = 0.0;
  double audioRatio = 1.0;
  int refreshesPerFrame = 0;

  m_pacer.configure(context);
  resolved = m_pacer.getResolvedMode();
  followingDisplay = m_pacer.isFollowingTheDisplay();
  effectiveFps = m_pacer.getEffectiveFps();
  audioRatio = m_pacer.getAudioRatio();
  refreshesPerFrame = m_pacer.getRefreshesPerFrame();

  const auto onRefreshes = resolved == firelight::emulation::SyncMode::Display;
  m_renderContinuously.store(onRefreshes);

  if (onRefreshes) {
    update();
  }

  // TODO
  // Auto is a choice between the named modes, so it says which one it made rather than leaving the
  // reader to work out whether this is monitor's behaviour or native's
  const auto chose = method == SyncMethod::Auto ? fmt::format(" (chose '{}')", followingDisplay ? "monitor" : "native")
                                                : std::string();

  spdlog::info("Pacing '{}'{}: against {} at {:.3f}fps, {} refresh(es) per frame, audio x{:.4f} "
               "(core {:.3f}fps, display {:.3f}Hz, {}, audio sink {})",
               emulator->getSyncMethod(), chose, pacedAgainst(resolved), effectiveFps, refreshesPerFrame, audioRatio,
               coreFps, refreshHz,
               presentationLocked ? "presentation waits for the display" : "presentation does not wait",
               audioAvailable ? "readable" : "absent");

  // TODO
  // The size the picture ends up at on screen, which the renderer cannot see because its target is
  // sized to the core's output rather than to the window
  const auto pixelRatio = window() != nullptr ? window()->devicePixelRatio() : 1.0;
  firelight::diagnostics::PerformanceStats::instance().setViewport(static_cast<int>(width() * pixelRatio),
                                                                   static_cast<int>(height() * pixelRatio));

  firelight::diagnostics::PerformanceStats::instance().setPacing(fmt::format("{}{}", emulator->getSyncMethod(), chose),
                                                                 refreshHz, audioRatio);
  emulator->setAudioPlaybackRateRatio(audioRatio);
}

void EmulatorItem::writeSuspendPoint(const int index) {
  if (m_renderer) {
    submitToEmulator(
        {.type = firelight::emulation::EmulatorCommandType::WriteSuspendPoint, .suspendPointIndex = index});
    update();
  }
}

void EmulatorItem::captureScreenshot() {
  if (m_renderer) {
    submitToEmulator({.type = firelight::emulation::EmulatorCommandType::CaptureScreenshot});
    update();
  }
}

void EmulatorItem::captureVideoClip() {
  if (m_renderer) {
    submitToEmulator({.type = firelight::emulation::EmulatorCommandType::CaptureVideoClip});
    update();
  }
}

void EmulatorItem::loadSuspendPoint(const int index) {
  if (m_renderer) {
    submitToEmulator({.type = firelight::emulation::EmulatorCommandType::LoadSuspendPoint, .suspendPointIndex = index});
    update();
  }
}

void EmulatorItem::undoLastLoadSuspendPoint() {
  if (m_renderer) {
    submitToEmulator({.type = firelight::emulation::EmulatorCommandType::UndoLoadSuspendPoint});
    update();
  }
}

void EmulatorItem::createRewindPoints() {
  if (m_renderer) {
    submitToEmulator({.type = firelight::emulation::EmulatorCommandType::EmitRewindPoints});
    update();
  }
}

void EmulatorItem::loadRewindPoint(const int index) {
  if (m_renderer) {
    submitToEmulator({.type = firelight::emulation::EmulatorCommandType::LoadRewindPoint, .rewindPointIndex = index});
    update();
  }
}

void EmulatorItem::setPlaybackMultiplier(float playbackMultiplier) {
  if (playbackMultiplier < 0.1f) {
    return;
  }

  if (m_playbackMultiplier != playbackMultiplier) {
    m_playbackMultiplier = playbackMultiplier;
    emit playbackMultiplierChanged();

    if (m_renderer) {
      submitToEmulator({.type = firelight::emulation::EmulatorCommandType::SetPlaybackMultiplier,
                        .playbackMultiplier = m_playbackMultiplier});
      update();
    }
  }
}

void EmulatorItem::hoverMoveEvent(QHoverEvent *event) { feedPointer(event->position()); }

void EmulatorItem::hoverLeaveEvent(QHoverEvent *event) {
  // Pointer left the game surface: light guns read this as "off-screen" and
  // relative motion should not jump across the gap on re-entry. Ignore the
  // spurious leave Qt emits when a mouse-button grab begins (a button is held
  // and the cursor is still over the surface) — otherwise pulling the trigger
  // would momentarily flag off-screen and zero the aim
  m_hasLastMousePos = false;
  if (!m_mousePressed && !m_mouseRightPressed && !m_mouseMiddlePressed) {
    getInputService()->updateMouseOffscreen(true);
  }
}

void EmulatorItem::mousePressEvent(QMouseEvent *event) {
  switch (event->button()) {
  case Qt::LeftButton:
    m_mousePressed = true;
    break;
  case Qt::RightButton:
    m_mouseRightPressed = true;
    break;
  case Qt::MiddleButton:
    m_mouseMiddlePressed = true;
    break;
  default:
    break;
  }
  // Refresh the aim from the click location (and clear off-screen) so a light
  // gun fires exactly where the cursor is, even if hover updates were stale
  feedPointer(event->position());
  getInputService()->updateMouseButtons(m_mousePressed, m_mouseRightPressed, m_mouseMiddlePressed);
}

void EmulatorItem::mouseReleaseEvent(QMouseEvent *event) {
  switch (event->button()) {
  case Qt::LeftButton:
    m_mousePressed = false;
    break;
  case Qt::RightButton:
    m_mouseRightPressed = false;
    break;
  case Qt::MiddleButton:
    m_mouseMiddlePressed = false;
    break;
  default:
    break;
  }
  feedPointer(event->position());
  getInputService()->updateMouseButtons(m_mousePressed, m_mouseRightPressed, m_mouseMiddlePressed);
}

void EmulatorItem::startGame() {
  QThreadPool::globalInstance()->start([this] {
    const auto emuInstance = firelight::emulation::EmulationService::getInstance()->getCurrentEmulatorInstance();

    auto entry = firelight::emulation::EmulationService::getInstance()->getCurrentEntry();
    if (!entry) {
      return;
    }

    m_entryId = entry->id;
    m_gameName = QString::fromStdString(entry->displayName);
    m_contentHash = QString::fromStdString(entry->contentHash);
    m_saveSlotNumber = entry->activeSaveSlot;
    m_platformId = entry->platformId;
    m_iconSourceUrl1x1 = QString::fromStdString(entry->icon1x1SourceUrl);

    emit contentHashChanged();
    emit platformIdChanged();
    emit entryIdChanged();
    emit saveSlotNumberChanged();
    emit gameNameChanged();

    // Qt owns the renderer, so it will destroy it. EmulatorItem is the
    // ServiceAccessor; it hands the renderer its dependencies directly
    m_renderer = new EmulatorItemRenderer(window()->rendererInterface()->graphicsApi(), window(), emuInstance,
                                          getActivityService(), getAchievementManager(), getGameImageProvider(),
                                          getSaveManager(), getMediaService());

    m_renderer->onGeometryChanged([this](unsigned int width, unsigned int height, float aspectRatio, double framerate) {
      updateGeometry(width, height, aspectRatio);
      if (framerate > 0.0) {
        m_coreFps = framerate;
      }
      QMetaObject::invokeMethod(this, "reconfigurePacing", Qt::QueuedConnection);
    });

    // Setting these causes the item's geometry to be visible, and the renderer
    // is initialized. If an item is not visible, the renderer is not
    // initialized
    m_coreBaseWidth = 1;
    m_coreBaseHeight = 1;
    m_calculatedAspectRatio = 1;
    m_coreAspectRatio = 1;

    emit videoWidthChanged();
    emit videoHeightChanged();
    emit videoAspectRatioChanged();

    m_started = true;
    emit gameStarted();

    getDiscordManager()->startGameActivity(m_contentHash.toStdString(), m_gameName.toStdString(), m_platformId,
                                           m_iconSourceUrl1x1.toStdString());
  });
}

QQuickRhiItemRenderer *EmulatorItem::createRenderer() { return m_renderer; }

void EmulatorItem::updateGeometry(unsigned int width, unsigned int height, float aspectRatio) {
  m_coreBaseWidth = width;
  m_coreBaseHeight = height;
  m_coreAspectRatio = aspectRatio;
  m_calculatedAspectRatio = static_cast<float>(m_coreBaseWidth) / static_cast<float>(m_coreBaseHeight);
  if (m_coreAspectRatio == 1 / m_calculatedAspectRatio) {
    m_coreBaseWidth = height;
    m_coreBaseHeight = width;
  }

  spdlog::info("width: {}, height: {}, aspectRatio: {}, calculatedAspectRatio: {}", m_coreBaseWidth, m_coreBaseHeight,
               m_coreAspectRatio, m_calculatedAspectRatio);
  setFixedColorBufferWidth(m_coreBaseWidth);
  setFixedColorBufferHeight(m_coreBaseHeight);
  emit videoWidthChanged();
  emit videoHeightChanged();
  emit videoAspectRatioChanged();
}

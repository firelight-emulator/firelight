#include "emulator_item.hpp"

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
#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <deque>
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

// FLDIAG (temporary instrumentation — remove before the real change lands)
// Counts the gaps between repeated events into 0.5ms buckets, so a cadence can be read as a shape
// rather than a stream of per-frame lines
namespace fldiag {
class IntervalHistogram {
public:
  IntervalHistogram(const char *name, std::atomic<int> *extra = nullptr) : m_name(name), m_extra(extra) {}

  void record() {
    const auto now = std::chrono::steady_clock::now().time_since_epoch().count();
    std::lock_guard lock(m_mutex);

    if (m_lastNs != 0) {
      const auto ms = static_cast<double>(now - m_lastNs) / 1e6;
      m_buckets[std::clamp(static_cast<int>(ms * 2.0), 0, BUCKETS - 1)]++;
      m_count++;
      m_sumMs += ms;
      m_minMs = std::min(m_minMs, ms);
      m_maxMs = std::max(m_maxMs, ms);
    }

    m_lastNs = now;

    if (m_dumpAtNs == 0) {
      m_dumpAtNs = now + 5000000000LL;
    } else if (now >= m_dumpAtNs) {
      m_dumpAtNs = now + 5000000000LL;
      dump();
    }
  }

private:
  void dump() {
    if (m_count == 0) {
      return;
    }

    std::string shape;
    for (auto i = 0; i < BUCKETS; ++i) {
      if (m_buckets[i] > 0) {
        shape += fmt::format(" {:.1f}:{}", i * 0.5, m_buckets[i]);
      }
    }

    const auto extra = m_extra != nullptr ? m_extra->exchange(0) : -1;

    spdlog::info("FLDIAG {} n={} mean={:.2f}ms min={:.2f} max={:.2f}{} |{}", m_name, m_count, m_sumMs / m_count,
                 m_minMs, m_maxMs, extra >= 0 ? fmt::format(" skipped-renders={}", extra) : std::string(), shape);

    m_buckets.fill(0);
    m_count = 0;
    m_sumMs = 0.0;
    m_minMs = 1e9;
    m_maxMs = 0.0;
  }

  static constexpr int BUCKETS = 81; // 0-40ms in 0.5ms steps, last bucket catches everything above

  const char *m_name;
  std::atomic<int> *m_extra;
  std::mutex m_mutex;
  std::array<int, BUCKETS> m_buckets{};
  int64_t m_lastNs = 0;
  int64_t m_dumpAtNs = 0;
  int m_count = 0;
  double m_sumMs = 0.0;
  double m_minMs = 1e9;
  double m_maxMs = 0.0;
};

std::atomic<int> skippedRenders{0};

// FLDIAG (temporary)
// How long each frame stays on screen, counted in refreshes. This is what smoothness actually is:
// an even number every time looks right, a mix of 1 and 2 is judder however evenly the frames were
// produced
class DisplayDurations {
public:
  void recordPresent(const uint64_t frameId) {
    std::lock_guard lock(m_mutex);

    if (frameId == m_currentId) {
      m_refreshes++;
      return;
    }

    if (m_currentId != 0 && m_refreshes > 0) {
      m_counts[std::min<size_t>(m_refreshes, m_counts.size() - 1)]++;
      m_total++;
    }

    m_currentId = frameId;
    m_refreshes = 1;

    if (m_total >= 300) {
      std::string shape;
      for (size_t i = 0; i < m_counts.size(); ++i) {
        if (m_counts[i] > 0) {
          shape += fmt::format(" {}x:{}", i, m_counts[i]);
        }
      }
      spdlog::info("FLDIAG shown-for  {} frames |{}", m_total, shape);
      m_counts.fill(0);
      m_total = 0;
    }
  }

private:
  std::mutex m_mutex;
  std::array<int, 9> m_counts{};
  uint64_t m_currentId = 0;
  int m_refreshes = 0;
  int m_total = 0;
};

DisplayDurations displayDurations;
std::atomic<uint64_t> lastUploadedFrameId{0};

IntervalHistogram presentIntervals("present   ");
IntervalHistogram submitIntervals("submit    ");
IntervalHistogram runFrameIntervals("runframe  ", &skippedRenders);
} // namespace fldiag

void EmulatorItem::fldiagRecordRunFrame() { fldiag::runFrameIntervals.record(); }

void EmulatorItem::fldiagRecordSkippedRender() { fldiag::skippedRenders++; }

void EmulatorItem::fldiagRecordUploadedFrame(const uint64_t frameId) { fldiag::lastUploadedFrameId = frameId; }

EmulatorItem::EmulatorItem(QQuickItem *parent) : QQuickRhiItem(parent) {
  // The emulator a hotkey acts on. Registered from here rather than when the
  // game starts, because the ScopeAlways actions can fire before then
  if (const auto actions = getShortcutActions()) {
    actions->setController(this);
  }

  // FLDIAG (temporary): watch what the display is actually doing, and when frames reach it
  connect(this, &QQuickItem::windowChanged, this, [this](QQuickWindow *w) {
    if (w == nullptr) {
      return;
    }

    connect(
        w, &QQuickWindow::frameSwapped, this,
        [this] {
          fldiag::presentIntervals.record(); // FLDIAG (temporary)
          fldiag::displayDurations.recordPresent(fldiag::lastUploadedFrameId.load());

          // Display mode counts these
          m_presentCount.fetch_add(1);
          m_loopWake.notify_one();
        },
        Qt::DirectConnection);

    if (auto *screen = w->screen()) {
      spdlog::info("FLDIAG screen '{}' refreshRate={:.3f}Hz", screen->name().toStdString(), screen->refreshRate());
      // A panel whose rate moves — ProMotion, or a VRR monitor — has to re-pace, or Display mode
      // keeps holding frames for a number of refreshes that stopped being right
      connect(screen, &QScreen::refreshRateChanged, this, [this](const qreal rate) {
        spdlog::info("FLDIAG refreshRate changed to {:.3f}Hz", rate); // FLDIAG (temporary)
        QMetaObject::invokeMethod(this, "reconfigurePacing", Qt::QueuedConnection);
      });
    }

    connect(w, &QWindow::screenChanged, this, [](QScreen *screen) {
      if (screen != nullptr) {
        spdlog::info("FLDIAG moved to screen '{}' refreshRate={:.3f}Hz", screen->name().toStdString(),
                     screen->refreshRate());
      }
    });
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
    if (m_renderer) {
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
  if (const auto emulator = firelight::emulation::EmulationService::getInstance()->getCurrentEmulatorInstance()) {
    emulator->submitCommand(command);
  }
}

void EmulatorItem::waitForNextFrame() {
  const auto deadlineNs = m_rateController.getNextDeadlineNs();

  if (deadlineNs == 0) {
    // Nothing has established a cadence yet — the first frame of a game, or a mode that hasn't been
    // configured. Wait a moment rather than spinning
    std::unique_lock lock(m_loopMutex);
    // Audio asks the sink often enough that a frame is never late by more than this, and Display
    // is woken by a refresh rather than the timeout
    m_loopWake.wait_for(lock, std::chrono::milliseconds(1),
                        [this] { return m_presentCount.load() > 0 || m_emulationStopping; });
    return;
  }

  const auto marginNs = m_spinMarginNs.load();
  const auto sleepUntilNs = deadlineNs - marginNs;
  const auto nowNs = std::chrono::steady_clock::now().time_since_epoch().count();

  if (sleepUntilNs > nowNs) {
    std::unique_lock lock(m_loopMutex);
    m_loopWake.wait_for(lock, std::chrono::nanoseconds(sleepUntilNs - nowNs),
                        [this] { return m_emulationStopping.load(); });
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

    const auto decoupled = m_renderer != nullptr && m_renderer->isDecoupled();

    if (!decoupled) {
      // A core tied to the render thread still has its frames run there; all this decides is when
      if (m_rateController.framesDue(std::chrono::steady_clock::now().time_since_epoch().count()) > 0 && m_renderer) {
        submitToEmulator({.type = firelight::emulation::EmulatorCommandType::RunFrame});
        QMetaObject::invokeMethod(this, "update", Qt::QueuedConnection);
      }
      continue;
    }

    auto *emulator = firelight::emulation::EmulationService::getInstance()->getCurrentEmulatorInstance();

    if (emulator == nullptr) {
      continue;
    }

    // Anything queued runs on this thread too, so a save state can't be taken from the middle of a
    // frame
    emulator->drainCommands();

    if (m_paused) {
      continue;
    }

    if (m_rateController.getContext().mode == firelight::emulation::SyncMode::Audio) {
      m_rateController.setAudioBufferLevel(
          firelight::emulation::EmulationService::getInstance()->currentAudioBufferLevel());
    }

    const auto nowNs = std::chrono::steady_clock::now().time_since_epoch().count();
    auto frames = 0;

    // Display puts frames on refreshes; everything else runs on a clock
    for (auto refreshes = m_presentCount.exchange(0); refreshes > 0; --refreshes) {
      frames += m_rateController.framesDueOnPresent();
    }

    frames += m_rateController.framesDue(nowNs);

    if (frames > 0) {
      m_renderer->runFrames(frames);
      QMetaObject::invokeMethod(this, "update", Qt::QueuedConnection);
    }

    emulator->drainCommands();

    // The GUI's undo affordance follows what the emulator actually has to undo
    if (const auto canUndo = emulator->canUndoLoadSuspendPoint(); canUndo != m_canUndoLoadSuspendPoint) {
      m_canUndoLoadSuspendPoint = canUndo;
      QMetaObject::invokeMethod(this, "canUndoLoadSuspendPointChanged", Qt::QueuedConnection);
    }
  }
}

EmulatorItem::~EmulatorItem() {
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

bool EmulatorItem::isMuted() const {
  return firelight::emulation::EmulationService::getInstance()->currentAudioMuted();
}

void EmulatorItem::setMuted(const bool muted) {
  auto *service = firelight::emulation::EmulationService::getInstance();
  if (service->currentAudioMuted() == muted) {
    return;
  }

  service->setCurrentAudioMuted(muted);
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

int64_t EmulatorItem::computeTargetIntervalNs(const SyncMethod method, const double coreFps, const int targetFramerate,
                                              const double refreshHz) {
  double fps = 0.0;
  switch (method) {
  case SyncMethod::Fixed:
    fps = static_cast<double>(targetFramerate);
    break;
  case SyncMethod::Native:
    fps = coreFps;
    break;
  case SyncMethod::Monitor: // resolved via monitorPacingRate()
  case SyncMethod::Audio:   // audio-driven: no wall-clock target
    return 0;
  }
  if (fps <= 0.0) {
    return 0;
  }
  return static_cast<int64_t>(1e9 / fps);
}

double EmulatorItem::monitorPacingRate(const double coreFps, const double refreshHz) {
  if (coreFps <= 0.0 || refreshHz <= 0.0) {
    return 0.0;
  }
  // Divide the refresh rate down to the integer fraction closest to the content
  // rate (e.g. 120 Hz / 2 = 60 for a 60 fps game; 144 Hz / 2 = 72, which won't
  // match below)
  const double multiple = std::round(refreshHz / coreFps);
  if (multiple < 1.0) {
    return 0.0;
  }
  const double effectiveRate = refreshHz / multiple;
  constexpr double TOLERANCE = 0.05; // within 5% of the content rate
  if (std::abs(effectiveRate - coreFps) / coreFps <= TOLERANCE) {
    return effectiveRate;
  }
  return 0.0; // display doesn't line up with the content rate
}

void EmulatorItem::reconfigurePacing() {
  const auto emulator = firelight::emulation::EmulationService::getInstance()->getCurrentEmulatorInstance();
  if (!emulator) {
    return;
  }

  const auto method = syncMethodFromString(emulator->getSyncMethod());
  const double coreFps = m_coreFps.load() > 0.0 ? m_coreFps.load() : 60.0;
  const int targetFramerate = emulator->getTargetFramerate();

  double refreshHz = 60.0;
  if (const auto *w = window()) {
    if (const auto *screen = w->screen(); screen && screen->refreshRate() > 0.0) {
      refreshHz = screen->refreshRate();
    }
  }

  // Audio sync needs a working audio device; without one it is just a clock at the core's rate
  const bool audioAvailable = emulator->getAudioBufferLevel() >= 0.0f;

  firelight::emulation::PacingContext context;
  context.contentFps = coreFps;

  switch (method) {
  case SyncMethod::Audio:
    context.mode = audioAvailable ? firelight::emulation::SyncMode::Audio : firelight::emulation::SyncMode::Fixed;
    break;
  case SyncMethod::Monitor:
    context.mode = firelight::emulation::SyncMode::Display;
    context.displayHz = refreshHz;
    break;
  case SyncMethod::Fixed:
    context.mode = firelight::emulation::SyncMode::Fixed;
    context.contentFps = targetFramerate > 0 ? static_cast<double>(targetFramerate) : coreFps;
    break;
  case SyncMethod::Native:
    context.mode = firelight::emulation::SyncMode::Fixed;
    break;
  }

  m_rateController.configure(context);

  emulator->setAudioPlaybackRateRatio(m_rateController.getAudioRatio());
  // Nothing here changes the rate audio is produced at any more — Audio mode gates whole frames on
  // there being room, which is what the resampler's own correction is for the fine end of
  emulator->setPacingOwnsAudioRate(false);
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

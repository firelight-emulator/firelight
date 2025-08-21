#include "emulator_item.hpp"
#include <QQuickWindow>
#include <rhi/qrhi_platform.h>
#include <spdlog/spdlog.h>

#include "emulation/emulation_service.hpp"
#include "emulator_item_renderer.hpp"
#include "input2/input_service.hpp"
#include "platform_metadata.hpp"

#include <library/rom_file.hpp>
#include <patching/bps_patch.hpp>
#include <patching/ups_patch.hpp>

void EmulatorItem::mouseMoveEvent(QMouseEvent *event) {
  const auto pos = event->position();
  const auto bounds = boundingRect();

  const auto x = (pos.x() - bounds.width() / 2) / (bounds.width() / 2);
  const auto y = (pos.y() - bounds.height() / 2) / (bounds.height() / 2);

  firelight::input::InputService::instance()->updateMouseState(x, y,
                                                               m_mousePressed);
}

EmulatorItem::EmulatorItem(QQuickItem *parent) : QQuickRhiItem(parent) {
  setFlag(ItemHasContents);
  setAcceptHoverEvents(true);
  setAcceptTouchEvents(true);
  setAcceptedMouseButtons(Qt::LeftButton);

  m_threadPool.setMaxThreadCount(1);

  m_autosaveTimer.setInterval(10000);
  m_autosaveTimer.setSingleShot(false);
  connect(&m_autosaveTimer, &QTimer::timeout, [this] {
    if (m_renderer) {
      m_renderer->submitCommand({.type = EmulatorItemRenderer::WriteSaveFile});
      update();
    }
  });

  m_autosaveTimer.start();

  m_rewindPointTimer.setInterval(3000);
  m_rewindPointTimer.setSingleShot(false);
  connect(&m_rewindPointTimer, &QTimer::timeout, [this] {
    if (m_renderer) {
      m_renderer->submitCommand(
          {.type = EmulatorItemRenderer::WriteRewindPoint});
      update();
    }
  });
  m_rewindPointTimer.start();

  const int64_t BUSY_WAIT_MARGIN_NS = 800000LL;

  m_emulationThread.setServiceLevel(QThread::QualityOfService::High);
  m_emulationTimer.setInterval(std::chrono::nanoseconds(BUSY_WAIT_MARGIN_NS));
  // m_emulatorTimer.setSingleShot(false);
  m_emulationTimer.setTimerType(Qt::PreciseTimer);

  connect(&m_emulationTimer, &QChronoTimer::timeout, [this] {
    auto actualTargetNs = m_emulationTimingTargetNs;
    // Static variables for timing and rolling average
    static int64_t previousFrameActualEndTimeNs =
        0; // When the last frame *actually* ended (after spin)
    static int64_t timingCorrectionNs =
        0; // Adaptive correction for the spin target

    const std::size_t windowSize = 200;
    static std::deque<int64_t> recentSignedDifferencesNs_deque;
    static std::deque<int64_t> recentAbsoluteDifferencesNs_deque;
    static int64_t rollingSumOfSignedDifferencesNs = 0;
    static int64_t rollingSumOfAbsoluteDifferencesNs = 0;

    // auto osTimerWakeupTimeNs =
    // std::chrono::high_resolution_clock::now().time_since_epoch().count();

    if (previousFrameActualEndTimeNs == 0) {
      // First call, or after a reset. Perform work and set the baseline.
      // CALL YOUR FRAME UPDATE/WORK FUNCTION HERE (e.g.,
      // your_main_update_function();)

      QMetaObject::invokeMethod(this, "update", Qt::QueuedConnection);
      previousFrameActualEndTimeNs =
          std::chrono::high_resolution_clock::now().time_since_epoch().count();
      spdlog::info(
          "Timing initialized. First frame processed. End time recorded.");
      return;
    }

    // Determine the target end time for *this* frame's spin.
    // Add the adaptive timingCorrectionNs here.
    int64_t intendedTargetFrameEndTimeNs =
        previousFrameActualEndTimeNs + actualTargetNs + timingCorrectionNs;
    int64_t spinStartTimeNs =
        std::chrono::high_resolution_clock::now().time_since_epoch().count();

    // --- Busy Wait (Spin Loop) ---
    if (spinStartTimeNs < intendedTargetFrameEndTimeNs) {
      while (
          std::chrono::high_resolution_clock::now().time_since_epoch().count() <
          intendedTargetFrameEndTimeNs) {
        // This is a hard spin, consumes 100% CPU on one core.
        // Optional: If there's significant time left (e.g., > 0.2-0.5 ms),
        // you could std::this_thread::yield(); or a platform-specific short
        // pause to reduce CPU load, at the cost of slightly less precision.
        // Example:
        // if (intendedTargetFrameEndTimeNs -
        //         std::chrono::high_resolution_clock::now()
        //             .time_since_epoch()
        //             .count() >
        //     200000) { // > 0.2ms
        //   std::this_thread::yield();
        // }
      }
    }
    // --- End of Busy Wait ---

    auto currentFrameActualEndTimeNs =
        std::chrono::high_resolution_clock::now().time_since_epoch().count();
    int64_t achievedFrameDurationNs =
        currentFrameActualEndTimeNs - previousFrameActualEndTimeNs;
    previousFrameActualEndTimeNs =
        currentFrameActualEndTimeNs; // Update for the next frame's
                                     // calculation

    if (achievedFrameDurationNs <= 0 ||
        achievedFrameDurationNs > (actualTargetNs * 5)) {
      // spdlog::warn(
      //     "Anomalous achieved frame duration: {} ns. Skipping this sample.",
      //     achievedFrameDurationNs);
      // TODO: Consider resetting timingCorrectionNs if this happens often, or
      // if the pause was very long timingCorrectionNs = 0;
      return;
    }

    int64_t currentSignedDifferenceNs =
        actualTargetNs - achievedFrameDurationNs; // Target - Actual
    int64_t currentAbsoluteDifferenceNs = std::abs(currentSignedDifferenceNs);

    rollingSumOfSignedDifferencesNs += currentSignedDifferenceNs;
    recentSignedDifferencesNs_deque.push_back(currentSignedDifferenceNs);

    rollingSumOfAbsoluteDifferencesNs += currentAbsoluteDifferenceNs;
    recentAbsoluteDifferencesNs_deque.push_back(currentAbsoluteDifferenceNs);

    if (recentSignedDifferencesNs_deque.size() > windowSize) {
      rollingSumOfSignedDifferencesNs -=
          recentSignedDifferencesNs_deque.front();
      recentSignedDifferencesNs_deque.pop_front();
      rollingSumOfAbsoluteDifferencesNs -=
          recentAbsoluteDifferencesNs_deque.front();
      recentAbsoluteDifferencesNs_deque.pop_front();
    }

    std::size_t currentSamplesInWindow = recentSignedDifferencesNs_deque.size();

    if (currentSamplesInWindow > 0) {
      double averageSignedDifferenceNs =
          static_cast<double>(rollingSumOfSignedDifferencesNs) /
          currentSamplesInWindow;
      double averageAbsoluteDifferenceNs =
          static_cast<double>(rollingSumOfAbsoluteDifferencesNs) /
          currentSamplesInWindow;
      double averageSignedDifferenceFraction =
          averageSignedDifferenceNs / static_cast<double>(actualTargetNs);

      // spdlog::info("Rolling Perf (last {} samples, Target: {:.1f}ms): "
      //              "SignedDev: {:.0f}ns ({:.2f}%), AbsDev: {:.0f}ns",
      //              currentSamplesInWindow, actualTargetNs / 1.0e6,
      //              averageSignedDifferenceNs,
      //              averageSignedDifferenceFraction * 100.0,
      //              averageAbsoluteDifferenceNs);

      if (currentSamplesInWindow >
          windowSize / 2) { // Wait for some stability in average
        timingCorrectionNs = static_cast<int64_t>(
            averageSignedDifferenceNs / 10.0); // Apply 10% of the average error
      }
      // Clamp the correction to avoid excessive adjustments
      const int64_t maxCorrectionNs =
          BUSY_WAIT_MARGIN_NS /
          2; // Don't correct by more than half the spin margin
      timingCorrectionNs = std::max(
          -maxCorrectionNs, std::min(timingCorrectionNs, maxCorrectionNs));
    }

    if (m_renderer) {
      m_renderer->submitCommand({.type = EmulatorItemRenderer::RunFrame});
    }
    QMetaObject::invokeMethod(this, "update", Qt::QueuedConnection);
  });

  m_emulationThread.start();
  m_emulationThread.setPriority(QThread::TimeCriticalPriority);
  m_emulationTimer.moveToThread(&m_emulationThread);
  QMetaObject::invokeMethod(&m_emulationTimer, "start", Qt::QueuedConnection);
}

EmulatorItem::~EmulatorItem() {
  m_stopping = true;
  getDiscordManager()->clearActivity();
  m_autosaveTimer.stop();
  // QMetaObject::invokeMethod(&m_emulationTimer, "stop", Qt::QueuedConnection);
  m_emulationThread.quit();
  m_emulationThread.exit();
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
bool EmulatorItem::isMuted() const { return m_audioManager->isMuted(); }

void EmulatorItem::setMuted(bool muted) {
  if (firelight::emulation::EmulationService::getInstance().isMuted() ==
      muted) {
    return;
  }

  firelight::emulation::EmulationService::getInstance().setMuted(muted);
  emit mutedChanged();
}
float EmulatorItem::audioBufferLevel() const {
  return m_audioManager ? m_audioManager->getBufferLevel() : 0.0f;
}

void EmulatorItem::resetGame() {
  if (m_renderer) {
    m_renderer->submitCommand({.type = EmulatorItemRenderer::ResetGame});
    update();
  }
}

void EmulatorItem::writeSuspendPoint(const int index) {
  if (m_renderer) {
    m_renderer->submitCommand({.type = EmulatorItemRenderer::WriteSuspendPoint,
                               .suspendPointIndex = index});
    update();
  }
}

void EmulatorItem::loadSuspendPoint(const int index) {
  if (m_renderer) {
    m_renderer->submitCommand({.type = EmulatorItemRenderer::LoadSuspendPoint,
                               .suspendPointIndex = index});
    update();
  }
}
void EmulatorItem::undoLastLoadSuspendPoint() {
  if (m_renderer) {
    m_renderer->submitCommand(
        {.type = EmulatorItemRenderer::UndoLoadSuspendPoint});
    update();
  }
}

void EmulatorItem::createRewindPoints() {
  if (m_renderer) {
    m_renderer->submitCommand({.type = EmulatorItemRenderer::EmitRewindPoints});
    update();
  }
}

void EmulatorItem::loadRewindPoint(const int index) {
  if (m_renderer) {
    m_renderer->submitCommand({.type = EmulatorItemRenderer::LoadRewindPoint,
                               .rewindPointIndex = index});
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
      m_renderer->submitCommand(
          {.type = EmulatorItemRenderer::SetPlaybackMultiplier,
           .playbackMultiplier = m_playbackMultiplier});
      update();
    }
  }
}

void EmulatorItem::hoverMoveEvent(QHoverEvent *event) {
  const auto pos = event->position();
  const auto bounds = boundingRect();

  const auto x = (pos.x() - bounds.width() / 2) / (bounds.width() / 2);
  const auto y = (pos.y() - bounds.height() / 2) / (bounds.height() / 2);

  firelight::input::InputService::instance()->updateMouseState(x, y,
                                                               m_mousePressed);
}

void EmulatorItem::mousePressEvent(QMouseEvent *event) {
  m_mousePressed = true;
  firelight::input::InputService::instance()->updateMousePressed(
      m_mousePressed);
}

void EmulatorItem::mouseReleaseEvent(QMouseEvent *event) {
  m_mousePressed = false;
  firelight::input::InputService::instance()->updateMousePressed(
      m_mousePressed);
}

void EmulatorItem::startGame() {
  QThreadPool::globalInstance()->start([this] {
    const auto emuInstance =
        firelight::emulation::EmulationService::getInstance()
            .getCurrentEmulatorInstance();

    auto entry =
        firelight::emulation::EmulationService::getInstance().getCurrentEntry();
    if (!entry) {
      return;
    }

    m_entryId = entry->id;
    m_gameName = entry->displayName;
    m_contentHash = entry->contentHash;
    m_saveSlotNumber = entry->activeSaveSlot;
    m_platformId = entry->platformId;
    m_iconSourceUrl1x1 = entry->icon1x1SourceUrl;

    // Qt owns the renderer, so it will destroy it.
    m_renderer = new EmulatorItemRenderer(
        window()->rendererInterface()->graphicsApi(), window(), emuInstance);

    m_renderer->onGeometryChanged([this](unsigned int width,
                                         unsigned int height, float aspectRatio,
                                         double framerate) {
      updateGeometry(width, height, aspectRatio);
      m_emulationTimingTargetNs = static_cast<int64_t>(1e9 / framerate);
    });

    // Setting these causes the item's geometry to be visible, and the renderer
    // is initialized. If an item is not visible, the renderer is not
    // initialized.
    m_coreBaseWidth = 1;
    m_coreBaseHeight = 1;
    m_calculatedAspectRatio = 1;
    m_coreAspectRatio = 1;

    emit videoWidthChanged();
    emit videoHeightChanged();
    emit videoAspectRatioChanged();

    m_started = true;
    emit gameStarted();

    getDiscordManager()->startGameActivity(
        m_contentHash.toStdString(), m_gameName.toStdString(), m_platformId,
        m_iconSourceUrl1x1.toStdString());
  });
}

QQuickRhiItemRenderer *EmulatorItem::createRenderer() { return m_renderer; }

void EmulatorItem::updateGeometry(unsigned int width, unsigned int height,
                                  float aspectRatio) {
  m_coreBaseWidth = width;
  m_coreBaseHeight = height;
  m_coreAspectRatio = aspectRatio;
  m_calculatedAspectRatio = static_cast<float>(m_coreBaseWidth) /
                            static_cast<float>(m_coreBaseHeight);
  if (m_coreAspectRatio == 1 / m_calculatedAspectRatio) {
    m_coreBaseWidth = height;
    m_coreBaseHeight = width;
  }

  spdlog::info(
      "width: {}, height: {}, aspectRatio: {}, calculatedAspectRatio: {}",
      m_coreBaseWidth, m_coreBaseHeight, m_coreAspectRatio,
      m_calculatedAspectRatio);
  setFixedColorBufferWidth(m_coreBaseWidth);
  setFixedColorBufferHeight(m_coreBaseHeight);
  emit videoWidthChanged();
  emit videoHeightChanged();
  emit videoAspectRatioChanged();
}

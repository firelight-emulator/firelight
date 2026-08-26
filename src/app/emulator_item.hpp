#pragma once

#include "audio/audio_manager.hpp"
#include "emulation/emulation_rate_controller.hpp"
#include "emulation/emulator_command.hpp"
#include "emulation/emulator_controller.hpp"
#include "emulation/refresh_counter.hpp"
#include "emulator_item_renderer.hpp"
#include "libretro/core_configuration.hpp"
#include "service_accessor.hpp"

#include <firelight/event_dispatcher.hpp>

#include <QThreadPool>
#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <qchronotimer.h>
#include <rcheevos/ra_client.hpp>
#include <string>

// TODO
// Threading: a QML item — constructed and driven (properties/slots) on the GUI
// thread. Owns the frame-pacing thread (m_emulationThread), which decides when a
// frame is due and enqueues RunFrame onto the emulator; the frame itself runs on
// the render thread, in the pass that shows it
// m_paused is atomic because the pacing thread reads it each tick
class EmulatorItem : public QQuickRhiItem,
                     public firelight::ServiceAccessor,
                     public firelight::emulation::IEmulatorController {
protected:
  void mouseMoveEvent(QMouseEvent *event) override;

private:
  Q_OBJECT
  Q_PROPERTY(int entryId MEMBER m_entryId NOTIFY entryIdChanged)
  Q_PROPERTY(int platformId MEMBER m_platformId NOTIFY platformIdChanged)
  Q_PROPERTY(QString contentHash MEMBER m_contentHash NOTIFY contentHashChanged)
  Q_PROPERTY(QString gameName MEMBER m_gameName NOTIFY gameNameChanged)
  Q_PROPERTY(int saveSlotNumber MEMBER m_saveSlotNumber NOTIFY saveSlotNumberChanged)
  Q_PROPERTY(bool started MEMBER m_started NOTIFY startedChanged)
  Q_PROPERTY(int videoWidth MEMBER m_coreBaseWidth NOTIFY videoWidthChanged)
  Q_PROPERTY(int videoHeight MEMBER m_coreBaseHeight NOTIFY videoHeightChanged)
  Q_PROPERTY(float videoAspectRatio MEMBER m_coreAspectRatio NOTIFY videoAspectRatioChanged)
  Q_PROPERTY(float trueAspectRatio MEMBER m_calculatedAspectRatio NOTIFY videoAspectRatioChanged)
  Q_PROPERTY(float canUndoLoadSuspendPoint MEMBER m_canUndoLoadSuspendPoint NOTIFY canUndoLoadSuspendPointChanged)
  Q_PROPERTY(bool paused READ paused WRITE setPaused NOTIFY pausedChanged)
  Q_PROPERTY(float audioBufferLevel READ audioBufferLevel NOTIFY audioBufferLevelChanged)
  Q_PROPERTY(
      float playbackMultiplier READ playbackMultiplier WRITE setPlaybackMultiplier NOTIFY playbackMultiplierChanged)
  Q_PROPERTY(bool muted READ isMuted WRITE setMuted NOTIFY mutedChanged)
  Q_PROPERTY(bool rewindEnabled READ isRewindEnabled WRITE setRewindEnabled NOTIFY rewindEnabledChanged)

public:
  explicit EmulatorItem(QQuickItem *parent = nullptr);

  ~EmulatorItem() override;

  float m_playbackMultiplier = 1;

  bool m_startAfterLoading = true;
  bool m_loaded = false;
  bool m_started = false;

  QString m_gameName;

  int m_entryId;
  QByteArray m_gameData;
  QByteArray m_saveData;
  QString m_corePath;
  QString m_contentHash;
  unsigned int m_saveSlotNumber;
  unsigned int m_platformId;
  QString m_contentPath;
  QString m_iconSourceUrl1x1;
  bool m_gameReady;

  bool m_canUndoLoadSuspendPoint = false;

  // Emulator state. Atomic: written on the GUI thread (setPaused), read on the
  // frame-pacing thread
  std::atomic<bool> m_paused = false;

  uint m_coreBaseWidth = 0;
  uint m_coreBaseHeight = 0;
  uint m_coreMaxWidth = 0;
  uint m_coreMaxHeight = 0;
  float m_coreAspectRatio = 0.0f;
  float m_calculatedAspectRatio = 0.0f;

  // std::shared_ptr<libretro::Core> m_core = nullptr;
  std::shared_ptr<CoreConfiguration> m_coreConfiguration = nullptr;

  [[nodiscard]] bool paused() const override;

  void setPaused(bool paused) override;

  // Runs a single frame and pauses again, so a paused game can be stepped
  Q_INVOKABLE void advanceOneFrame() override;

  bool isRewindEnabled() const;

  void setRewindEnabled(bool rewindEnabled);

  bool isMuted() const;

  void setMuted(bool muted);

  [[nodiscard]] float audioBufferLevel() const;

  Q_INVOKABLE void writeSuspendPoint(int index) override;

  // Captures the current frame to disk (bound to the "screenshot" shortcut)
  Q_INVOKABLE void captureScreenshot() override;

  Q_INVOKABLE void captureVideoClip() override;

  Q_INVOKABLE void loadSuspendPoint(int index) override;

  Q_INVOKABLE void undoLastLoadSuspendPoint();

  Q_INVOKABLE void createRewindPoints();

  Q_INVOKABLE void loadRewindPoint(int index);

  [[nodiscard]] float playbackMultiplier() const override { return m_playbackMultiplier; }

  void setPlaybackMultiplier(float playbackMultiplier) override;

  Q_INVOKABLE void incrementPlaybackMultiplier() {
    if (m_playbackMultiplier >= 1) {
      setPlaybackMultiplier(m_playbackMultiplier + 1);
    } else {
      setPlaybackMultiplier(m_playbackMultiplier * 2);
    }
  }

  Q_INVOKABLE void decrementPlaybackMultiplier() {
    if (m_playbackMultiplier > 1) {
      setPlaybackMultiplier(m_playbackMultiplier - 1);
    } else if (!getAchievementManager()->hardcoreModeActive()) {
      setPlaybackMultiplier(m_playbackMultiplier / 2);
    }
  }

protected:
  void hoverMoveEvent(QHoverEvent *event) override;

  void hoverLeaveEvent(QHoverEvent *event) override;

  void mousePressEvent(QMouseEvent *event) override;

  void mouseReleaseEvent(QMouseEvent *event) override;

public slots:
  void startGame();

  // Recomputes the frame-pacing target/mode from the current sync-method /
  // target-framerate settings, the core fps, and the display refresh rate
  // Must run on the GUI thread (reads window()/screen())
  void reconfigurePacing();

signals:
  void aboutToRunFrame();

  void startedChanged();

  void gameStarted();

  void pausedChanged();

  void videoWidthChanged();

  void videoHeightChanged();

  void videoAspectRatioChanged();

  void rewindPointsReady(QList<QJsonObject> points);

  void audioBufferLevelChanged();

  void entryIdChanged();

  void platformIdChanged();

  void saveSlotNumberChanged();

  void contentHashChanged();

  void gameNameChanged();

  void playbackMultiplierChanged();

  void canUndoLoadSuspendPointChanged();

  void rewindEnabledChanged();

  void mutedChanged();

protected:
  QQuickRhiItemRenderer *createRenderer() override;

private:
  // TODO
  // The least of a frame ever spun, and the most. The floor keeps a host with an accurate timer from
  // spinning at all; the ceiling keeps one with a bad timer from burning a whole core to hide it
  static constexpr int64_t MIN_SPIN_MARGIN_NS = 500000;
  static constexpr int64_t MAX_SPIN_MARGIN_NS = 8000000;

  // TODO
  // Added to a measured overshoot before it becomes the margin, so that the margin covers the sleeps
  // it was widened for rather than landing exactly on them
  static constexpr int64_t SPIN_MARGIN_HEADROOM_NS = 250000;

  /**
   * Runs frames for as long as the emulator is alive, on its own thread
   */
  void runEmulationLoop();

  /**
   * Waits for the next frame to be due: a deadline for a clock-driven mode, a present for Display
   */
  void waitForNextFrame();

  /**
   * Queues something for the running emulator, if there is one
   */
  static void submitToEmulator(const firelight::emulation::EmulatorCommand &command);

  bool m_stopping = false;
  QThreadPool m_threadPool;
  QTimer m_rewindPointTimer;
  EmulatorItemRenderer *m_renderer = nullptr;

  bool m_rewindEnabled = true;

  QThread m_emulationThread;

  // What decides when a frame is due, for whichever mode is in force
  firelight::emulation::EmulationRateController m_rateController;

  // TODO
  // Guards the controller: the pacing thread asks it for frames while the GUI thread reconfigures it
  // for a new setting, display or core rate. Uncontended in the ordinary case, since reconfiguring
  // happens on a setting change rather than on a frame
  std::mutex m_rateControllerMutex;

  // The loop runs until this is set; the condition variable is how a sleeping loop is woken early
  // to shut down rather than sleeping out the rest of its wait
  std::atomic<bool> m_emulationStopping = false;
  std::mutex m_loopMutex;
  std::condition_variable m_loopWake;

  // TODO
  // Whether a present should ask for the next render. Only true while frames are being put on
  // refreshes, which is also the only time presentation waits for the display and so the only time
  // this can't run away. Atomic: written on the GUI thread, read on the render thread
  std::atomic<bool> m_renderContinuously = false;

  // TODO
  // When the loop last saw a present. Only touched on the emulation thread
  int64_t m_lastPresentNs = 0;

  // TODO
  // Whether frames have stopped reaching the screen. Frames run on the render thread, so while it
  // isn't drawing there is nobody to run what the loop would queue — a minimised window would
  // otherwise collect an hour of them and pay for all of it at once on restore. Starts true because
  // nothing has been drawn yet. Atomic: set on the emulation thread, read on the GUI thread too
  std::atomic<bool> m_renderStalled = true;

  // TODO
  // One refresh of the display, in nanoseconds, or 0 when the rate isn't known. Lets a late present
  // be read as the number of refreshes that actually went by. Atomic: written on the GUI thread when
  // pacing is reconfigured, read on the render thread
  std::atomic<int64_t> m_displayPeriodNs = 0;

  // TODO
  // When the last present arrived. Only the render thread writes it
  std::atomic<int64_t> m_lastPresentAtNs = 0;

  // TODO
  // Reads the gap between two presents as a number of refreshes. Only the render thread touches it
  firelight::emulation::RefreshCounter m_refreshCounter;

  // TODO
  // The most refreshes one present may be read as covering, for whatever a frame is currently held
  // for. Atomic: written on the GUI thread when pacing is reconfigured, read on the render thread
  std::atomic<int> m_refreshCeiling = firelight::emulation::RefreshCounter::MIN_CEILING;

  // Refreshes since the loop last looked. A count rather than a flag: two can land between wakes,
  // and holding a frame for a number of refreshes only works if every one of them is seen
  std::atomic<int> m_presentCount = 0;

  // How much of the wait before a frame is spent spinning rather than sleeping. Sleeping alone
  // overshoots by more than a frame can afford where presentation follows production; 0 is pure sleep
  // TODO
  // How much of the frame is spun rather than slept, learned from how late the sleeps actually
  // return. A host whose timer is accurate settles at the floor and spins almost nothing; one whose
  // sleeps overshoot grows this until they are covered. Encoding a constant here instead is a claim
  // about the host, and the two this runs on do not agree
  /**
   * Widens or narrows the spun part of the frame to cover how late a sleep just returned
   */
  void noteWakeOvershoot(int64_t overshootNs);

  std::atomic<int64_t> m_spinMarginNs = MIN_SPIN_MARGIN_NS;
  // Wall-clock target interval for native/monitor/fixed pacing. Written on the
  // GUI thread (reconfigurePacing), read on the emulation thread
  // When true, pace off audio buffer occupancy instead of the wall clock
  // Core's native fps, cached from the renderer geometry callback
  std::atomic<double> m_coreFps = 60.0;

  ScopedConnection m_settingChangedConnection;

  bool m_mousePressed = false;
  bool m_mouseRightPressed = false;
  bool m_mouseMiddlePressed = false;
  // Last pointer position (item pixels) for computing relative mouse motion
  QPointF m_lastMousePos;
  bool m_hasLastMousePos = false;

  // Normalizes a pointer position, feeds absolute + relative motion to the
  // input service, and clears the light-gun off-screen flag
  void feedPointer(const QPointF &pos);

  void updateGeometry(unsigned int width, unsigned int height, float aspectRatio);

  // Frame-pacing strategy (maps to the "sync-method" emulation setting)
  enum class SyncMethod { Auto, Native, Monitor, Fixed, Audio };
  static SyncMethod syncMethodFromString(const std::string &method);
};

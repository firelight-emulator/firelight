// TODO: NEEDS REVIEW
#pragma once

#include "audio/audio_manager.hpp"
#include "diagnostics/vblank_probe.hpp"
#include "emulation/emulation_loop.hpp"
#include "emulation/emulator_command.hpp"
#include "emulation/emulator_controller.hpp"
#include "emulator_item_renderer.hpp"
#include "libretro/core_configuration.hpp"
#include "service_accessor.hpp"

#include <firelight/event_dispatcher.hpp>
#include <firelight/monitoring/monitor.hpp>

#include <QThreadPool>
#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <memory>
#include <mutex>
#include <qchronotimer.h>
#include <rcheevos/ra_client.hpp>
#include <string>

// TODO
// Threading: a QML item — constructed and driven (properties/slots) on the GUI
// thread. Owns the emulation thread (m_emulationThread), which runs the loop that
// brings the game up and runs its frames. paused() and playbackMultiplier() are
// read by that loop every tick, so what they read is atomic
class QScreen;

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

  std::atomic<float> m_playbackMultiplier{1.0F};

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

  /**
   * @return How many refreshes the last present was held for. A pass reads this to decide whether it
   *         can afford to run a second frame
   */

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

public:
  /**
   * How long the pass just run took to put a picture on the target. Render thread
   */
  void notePassDuration(int64_t durationNs);

private:
  /**
   * Re-paces when this screen's refresh rate changes, dropping the previous screen's
   */
  void followScreen(QScreen *screen);

  /**
   * Queues something for the running emulator, if there is one
   */
  static void submitToEmulator(const firelight::emulation::EmulatorCommand &command);

  bool m_stopping = false;
  QThreadPool m_threadPool;
  QTimer m_rewindPointTimer;
  // TODO
  // Made on the thread that starts the game, read by the loop
  std::atomic<EmulatorItemRenderer *> m_renderer{nullptr};

  bool m_rewindEnabled = true;

  // TODO
  // What was last asked for, which outlives any one emulator instance and is not the same thing as
  // whether the game can currently be heard
  bool m_muted = false;

  firelight::monitoring::Marker m_presentMarker =
      firelight::monitoring::Monitor::instance().marker("present", "A frame was handed to the display");

  // TODO
  // The render thread inside the frame begin, where the swapchain makes it wait. Begun and ended on
  // that thread only
  firelight::monitoring::Span m_swapchainWaitSpan = firelight::monitoring::Monitor::instance().span(
      "swapchain_wait", "The render thread waiting in the frame begin for the swapchain");
  int64_t m_swapchainWaitStartNs = 0;

  firelight::diagnostics::VblankProbe m_vblankProbe;

  // TODO
  // The loop and the clock it waits on; m_emulationThread runs it
  firelight::emulation::PrecisionLoopClock m_clock;
  std::unique_ptr<firelight::emulation::EmulationLoop> m_loop;

  QThread m_emulationThread;

  // TODO
  // Held so the handler can be taken off the window before this object's members go. ~QObject would
  // do it too, but only after every member below has already been destroyed, and the handler runs on
  // the render thread which is still presenting by then
  QMetaObject::Connection m_frameSwappedConnection;
  QMetaObject::Connection m_frameBeginConnection;
  QMetaObject::Connection m_frameSyncConnection;
  QMetaObject::Connection m_frameRenderConnection;

  // TODO
  // The screen whose refresh-rate changes re-pace, rebound when the window moves to another
  QMetaObject::Connection m_refreshRateConnection;

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

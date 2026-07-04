#pragma once

#include "audio/audio_manager.hpp"
#include <rcheevos/ra_client.hpp>
#include <QThreadPool>
#include "emulator_item_renderer.hpp"
#include "libretro/core_configuration.hpp"
#include "service_accessor.hpp"
#include <firelight/event_dispatcher.hpp>

#include <atomic>
#include <cstdint>
#include <qchronotimer.h>
#include <string>

class EmulatorItem : public QQuickRhiItem,
                     public firelight::ServiceAccessor {
protected:
  void mouseMoveEvent(QMouseEvent *event) override;

private:
  Q_OBJECT
  Q_PROPERTY(int entryId MEMBER m_entryId NOTIFY entryIdChanged)
  Q_PROPERTY(int platformId MEMBER m_platformId NOTIFY platformIdChanged)
  Q_PROPERTY(QString contentHash MEMBER m_contentHash NOTIFY contentHashChanged)
  Q_PROPERTY(QString gameName MEMBER m_gameName NOTIFY gameNameChanged)
  Q_PROPERTY(
      int saveSlotNumber MEMBER m_saveSlotNumber NOTIFY saveSlotNumberChanged)
  Q_PROPERTY(bool started MEMBER m_started NOTIFY startedChanged)
  Q_PROPERTY(int videoWidth MEMBER m_coreBaseWidth NOTIFY videoWidthChanged)
  Q_PROPERTY(int videoHeight MEMBER m_coreBaseHeight NOTIFY videoHeightChanged)
  Q_PROPERTY(float videoAspectRatio MEMBER m_coreAspectRatio NOTIFY
                 videoAspectRatioChanged)
  Q_PROPERTY(float trueAspectRatio MEMBER m_calculatedAspectRatio NOTIFY
                 videoAspectRatioChanged)
  Q_PROPERTY(float canUndoLoadSuspendPoint MEMBER m_canUndoLoadSuspendPoint
                 NOTIFY canUndoLoadSuspendPointChanged)
  Q_PROPERTY(bool paused READ paused WRITE setPaused NOTIFY pausedChanged)
  Q_PROPERTY(float audioBufferLevel READ audioBufferLevel NOTIFY
                 audioBufferLevelChanged)
  Q_PROPERTY(float playbackMultiplier READ playbackMultiplier WRITE
                 setPlaybackMultiplier NOTIFY playbackMultiplierChanged)
  Q_PROPERTY(bool muted READ isMuted WRITE setMuted NOTIFY mutedChanged)
  Q_PROPERTY(bool rewindEnabled READ isRewindEnabled WRITE setRewindEnabled
                 NOTIFY rewindEnabledChanged)

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

  // Emulator state
  bool m_paused = false;

  uint m_coreBaseWidth = 0;
  uint m_coreBaseHeight = 0;
  uint m_coreMaxWidth = 0;
  uint m_coreMaxHeight = 0;
  float m_coreAspectRatio = 0.0f;
  float m_calculatedAspectRatio = 0.0f;

  // std::shared_ptr<libretro::Core> m_core = nullptr;
  std::shared_ptr<CoreConfiguration> m_coreConfiguration = nullptr;

  [[nodiscard]] bool paused() const;

  void setPaused(bool paused);

  bool isRewindEnabled() const;

  void setRewindEnabled(bool rewindEnabled);

  bool isMuted() const;

  void setMuted(bool muted);

  [[nodiscard]] float audioBufferLevel() const;

  Q_INVOKABLE void writeSuspendPoint(int index);

  // Captures the current frame to disk (bound to the "screenshot" shortcut).
  Q_INVOKABLE void captureScreenshot();

  Q_INVOKABLE void loadSuspendPoint(int index);

  Q_INVOKABLE void undoLastLoadSuspendPoint();

  Q_INVOKABLE void createRewindPoints();

  Q_INVOKABLE void loadRewindPoint(int index);

  [[nodiscard]] float playbackMultiplier() const {
    return m_playbackMultiplier;
  }
  void setPlaybackMultiplier(float playbackMultiplier);

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
  // target-framerate settings, the core fps, and the display refresh rate.
  // Must run on the GUI thread (reads window()/screen()).
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
  bool m_stopping = false;
  QThreadPool m_threadPool;
  QTimer m_rewindPointTimer;
  EmulatorItemRenderer *m_renderer = nullptr;

  bool m_rewindEnabled = true;

  QThread m_emulationThread;
  QChronoTimer m_emulationTimer{};
  // Wall-clock target interval for native/monitor/fixed pacing. Written on the
  // GUI thread (reconfigurePacing), read on the emulation thread.
  std::atomic<int64_t> m_emulationTimingTargetNs = 16666667;
  // When true, pace off audio buffer occupancy instead of the wall clock.
  std::atomic<bool> m_audioSyncActive = false;
  // Core's native fps, cached from the renderer geometry callback.
  std::atomic<double> m_coreFps = 60.0;

  ScopedConnection m_settingChangedConnection;

  bool m_mousePressed = false;
  bool m_mouseRightPressed = false;
  bool m_mouseMiddlePressed = false;
  // Last pointer position (item pixels) for computing relative mouse motion.
  QPointF m_lastMousePos;
  bool m_hasLastMousePos = false;

  // Normalizes a pointer position, feeds absolute + relative motion to the
  // input service, and clears the light-gun off-screen flag.
  void feedPointer(const QPointF &pos);

  void updateGeometry(unsigned int width, unsigned int height,
                      float aspectRatio);

  // Frame-pacing strategy (maps to the "sync-method" emulation setting).
  enum class SyncMethod { Native, Monitor, Fixed, Audio };
  static SyncMethod syncMethodFromString(const std::string &method);

  // Wall-clock frame interval (ns) for native/fixed pacing:
  //   Native -> 1e9 / coreFps
  //   Fixed  -> 1e9 / targetFramerate
  // Returns 0 for Audio (audio-driven) and Monitor (resolved via
  // monitorPacingRate, which needs the refresh/content-rate relationship), or
  // when the input is non-positive.
  static int64_t computeTargetIntervalNs(SyncMethod method, double coreFps,
                                          int targetFramerate, double refreshHz);

  // The rate to pace at for "sync to monitor", or 0 if the display doesn't line
  // up with the content rate (caller falls back to native). Divides the refresh
  // rate down to the nearest integer fraction and only matches when that lands
  // within tolerance of coreFps — so 60/120/240 Hz match a 60 fps game but 144 Hz
  // (2.4x) does not, avoiding a sped-up game on high-refresh displays.
  static double monitorPacingRate(double coreFps, double refreshHz);
};

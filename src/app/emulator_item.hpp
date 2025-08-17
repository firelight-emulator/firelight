#pragma once

#include "audio/audio_manager.hpp"
#include "emulator_item_renderer.hpp"
#include "libretro/core_configuration.hpp"
#include "manager_accessor.hpp"

#include <qchronotimer.h>

class EmulatorItem : public QQuickRhiItem, public firelight::ManagerAccessor {
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
  std::shared_ptr<AudioManager> m_audioManager = nullptr;
  std::shared_ptr<CoreConfiguration> m_coreConfiguration = nullptr;

  [[nodiscard]] bool paused() const;

  void setPaused(bool paused);

  bool isRewindEnabled() const;

  void setRewindEnabled(bool rewindEnabled);

  bool isMuted() const;

  void setMuted(bool muted);

  [[nodiscard]] float audioBufferLevel() const;

  Q_INVOKABLE void resetGame();

  Q_INVOKABLE void writeSuspendPoint(int index);

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

  void mousePressEvent(QMouseEvent *event) override;

  void mouseReleaseEvent(QMouseEvent *event) override;

public slots:
  void loadGame(int entryId);
  void startGame();

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
  QTimer m_autosaveTimer;
  QTimer m_rewindPointTimer;
  EmulatorItemRenderer *m_renderer = nullptr;

  bool m_rewindEnabled = true;

  QThread m_emulationThread;
  QChronoTimer m_emulationTimer{};
  int64_t m_emulationTimingTargetNs = 16666667;

  bool m_mousePressed = false;

  void updateGeometry(unsigned int width, unsigned int height,
                      float aspectRatio);
};

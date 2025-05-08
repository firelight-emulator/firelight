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
  Q_PROPERTY(QString contentHash MEMBER m_contentHash NOTIFY contentHashChanged)
  Q_PROPERTY(QString gameName MEMBER m_gameName NOTIFY gameNameChanged)
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
  Q_PROPERTY(int playbackMultiplier READ playbackMultiplier WRITE
                 setPlaybackMultiplier NOTIFY playbackMultiplierChanged)

public:
  explicit EmulatorItem(QQuickItem *parent = nullptr);

  ~EmulatorItem() override;

  int m_playbackMultiplier = 1;

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

  [[nodiscard]] float audioBufferLevel() const;

  Q_INVOKABLE void resetGame();

  Q_INVOKABLE void writeSuspendPoint(int index);

  Q_INVOKABLE void loadSuspendPoint(int index);

  Q_INVOKABLE void undoLastLoadSuspendPoint();

  Q_INVOKABLE void createRewindPoints();

  Q_INVOKABLE void loadRewindPoint(int index);

  int playbackMultiplier() const { return m_playbackMultiplier; }
  void setPlaybackMultiplier(int playbackMultiplier);

  Q_INVOKABLE void incrementPlaybackMultiplier() {
    setPlaybackMultiplier(m_playbackMultiplier + 1);
  }

  Q_INVOKABLE void decrementPlaybackMultiplier() {
    setPlaybackMultiplier(m_playbackMultiplier - 1);
  }

protected:
  void hoverMoveEvent(QHoverEvent *event) override;

  void mousePressEvent(QMouseEvent *event) override;

  void mouseReleaseEvent(QMouseEvent *event) override;

public slots:
  void loadGame(int entryId);
  void startGame();

signals:
  void startedChanged();

  void gameStarted();

  void pausedChanged();

  void videoWidthChanged();

  void videoHeightChanged();

  void videoAspectRatioChanged();

  void rewindPointsReady(QList<QJsonObject> points);

  void audioBufferLevelChanged();

  void entryIdChanged();

  void contentHashChanged();

  void gameNameChanged();

  void playbackMultiplierChanged();

  void canUndoLoadSuspendPointChanged();

protected:
  QQuickRhiItemRenderer *createRenderer() override;

private:
  QThreadPool m_threadPool;
  QChronoTimer m_emulationTimer{};
  QTimer m_autosaveTimer;
  QTimer m_rewindPointTimer;
  EmulatorItemRenderer *m_renderer = nullptr;
  bool m_mousePressed = false;

  void updateGeometry(unsigned int width, unsigned int height,
                      float aspectRatio);
};

#pragma once

#include "library/library_scanner.hpp"
#include "libretro/core.hpp"
#include "manager_accessor.hpp"
#include "saves/suspend_point.hpp"

#include <QOpenGLTexture>
#include <QQuickFramebufferObject>
#include <firelight/play_session.hpp>

class EmulationManager : public QQuickFramebufferObject,
                         public firelight::ManagerAccessor,
                         public firelight::libretro::IVideoDataReceiver {
  Q_OBJECT
  Q_PROPERTY(QString currentGameName READ currentGameName NOTIFY
                 currentGameNameChanged)
  Q_PROPERTY(int nativeWidth READ nativeWidth NOTIFY nativeWidthChanged)
  Q_PROPERTY(int nativeHeight READ nativeHeight NOTIFY nativeHeightChanged)
  Q_PROPERTY(float nativeAspectRatio READ nativeAspectRatio NOTIFY
                 nativeAspectRatioChanged)
  Q_PROPERTY(bool running READ isRunning NOTIFY emulationStarted NOTIFY
                 emulationStopped)

public:
  [[nodiscard]] Renderer *createRenderer() const override;

  explicit EmulationManager(QQuickItem *parent = nullptr);
  ~EmulationManager() override;

  void setCurrentFboId(int fboId);
  void
  setGetProcAddressFunction(const std::function<proc_address_t(const char *)>
                                &getProcAddressFunction);
  [[nodiscard]] std::function<void()> consumeContextResetFunction();

  void setReceiveVideoDataFunction(
      const std::function<void(const void *data, unsigned width,
                               unsigned height, size_t pitch)>
          &receiveVideoDataFunction);

  [[nodiscard]] QString currentGameName() const;
  [[nodiscard]] int nativeWidth() const;
  [[nodiscard]] int nativeHeight() const;
  [[nodiscard]] float nativeAspectRatio() const;

  void receive(const void *data, unsigned width, unsigned height,
               size_t pitch) override;
  proc_address_t getProcAddress(const char *sym) override;
  void setResetContextFunc(context_reset_func) override;
  uintptr_t getCurrentFramebufferId() override;
  void setSystemAVInfo(retro_system_av_info *info) override;

  [[nodiscard]] bool runFrame() const;

public slots:
  void loadLibraryEntry(int entryId);
  void startEmulation();
  void pauseGame();
  void resumeGame();
  void stopEmulation();
  void resetEmulation();
  bool isRunning() const;

signals:
  void gameLoadSucceeded();
  void gameLoadFailed();

  void readyToStart();

  void gamePaused();
  void gameResumed();
  void emulationStarted();
  void emulationStopped();

  void currentGameNameChanged();
  void nativeWidthChanged();
  void nativeHeightChanged();
  void nativeAspectRatioChanged();

  void loadAchievements(QString contentId);

private:
  void save(bool waitForFinish = false);

  std::unique_ptr<libretro::Core> m_core;
  firelight::db::LibraryEntry m_currentEntry;

  bool m_gameLoadedSignalReady = false;
  bool m_achievementsLoadedSignalReady = false;

  bool m_isRunning = false;
  bool m_paused = false;

  std::unique_ptr<firelight::db::PlaySession> m_currentPlaySession;
  std::vector<SuspendPoint> m_suspendPoints;
  QElapsedTimer m_playtimeTimer;

  QByteArray m_gameData;
  QByteArray m_saveData;
  QString m_corePath;

  int m_nativeWidth = 0;
  int m_nativeHeight = 0;
  float m_nativeAspectRatio = 0;

  int numFramesRun = 0;

  /****************************************************************************
   * Ugly rendering stuff
   ***************************************************************************/
  std::function<proc_address_t(const char *)> m_getProcAddressFunction =
      nullptr;
  std::function<void()> m_resetContextFunction = nullptr;
  std::function<void(const void *data, unsigned width, unsigned height,
                     size_t pitch)>
      m_receiveVideoDataFunction = nullptr;
  bool m_usingHwRendering = false;
  int m_currentFboId = -1;
};

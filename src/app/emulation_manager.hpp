//
// Created by alexs on 12/20/2023.
//

#ifndef FIRELIGHT_EMULATION_MANAGER_HPP
#define FIRELIGHT_EMULATION_MANAGER_HPP

#include "../gui/QLibraryManager.hpp"
#include "libretro/core.hpp"
#include "manager_accessor.hpp"
#include <QOpenGLFramebufferObject>
#include <QOpenGLFunctions>
#include <QOpenGLTexture>
#include <QQuickFramebufferObject>
#include <QSGDynamicTexture>

static QLibraryManager *library_manager_ = nullptr;

class EmulationManager : public QQuickFramebufferObject,
                         public QOpenGLFunctions,
                         public Firelight::ManagerAccessor {
  Q_OBJECT

  typedef uintptr_t (*get_framebuffer_func)();

public:
  [[nodiscard]] Renderer *createRenderer() const override;

  explicit EmulationManager(QQuickItem *parent = nullptr);
  ~EmulationManager() override;

  int getEntryId() const;
  QByteArray getGameData();
  QByteArray getSaveData();
  QString getCorePath();

  bool takeShouldLoadGameFlag();
  bool takeShouldPauseGameFlag();
  bool takeShouldResumeGameFlag();
  bool takeShouldStartEmulationFlag();
  bool takeShouldStopEmulationFlag();

  void setIsRunning(bool isRunning);

public slots:
  void loadGame(int entryId, const QByteArray &gameData,
                const QByteArray &saveData, const QString &corePath);
  void pauseGame();
  void resumeGame();
  void startEmulation();
  void stopEmulation();
  bool isRunning();

signals:
  void gameLoaded();
  void gamePaused();
  void gameResumed();
  void emulationStarted();
  void emulationStopped();

private:
  bool m_shouldLoadGame = false;
  bool m_shouldPauseGame = false;
  bool m_shouldResumeGame = false;
  bool m_shouldStartEmulation = false;
  bool m_shouldStopEmulation = false;

  int m_entryId;
  QByteArray m_gameData;
  QByteArray m_saveData;
  QString m_corePath;

  bool m_shouldShutdown = false;
  bool m_isRunning = false;

  QMetaObject::Connection m_renderConnection;
  LibEntry m_currentEntry;
  int m_millisSinceLastSave{};
  retro_system_av_info *core_av_info_;
  bool glInitialized = false;
  QSGTexture *gameTexture = nullptr;
  bool usingHwRendering = false;
  std::unique_ptr<QOpenGLFramebufferObject> gameFbo = nullptr;
  QImage gameImage;
  context_reset_func reset_context = nullptr;
  bool running = false;
  Uint64 thisTick;
  Uint64 lastTick;
  std::unique_ptr<libretro::Core> core;
  double totalFrameWorkDurationMillis = 0;

  long long int frameCount = 0;
  int frameSkipRatio = 0;
  long numFrames = 0;
};

#endif // FIRELIGHT_EMULATION_MANAGER_HPP

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

class EmulationManager : public QQuickFramebufferObject,
                         public firelight::ManagerAccessor {
  Q_OBJECT
  Q_PROPERTY(QString currentGameName READ currentGameName NOTIFY
                 currentGameNameChanged)
  Q_PROPERTY(int nativeWidth READ nativeWidth NOTIFY nativeWidthChanged)
  Q_PROPERTY(int nativeHeight READ nativeHeight NOTIFY nativeHeightChanged)
  Q_PROPERTY(float nativeAspectRatio READ nativeAspectRatio NOTIFY
                 nativeAspectRatioChanged)

  typedef uintptr_t (*get_framebuffer_func)();

public:
  [[nodiscard]] Renderer *createRenderer() const override;

  explicit EmulationManager(QQuickItem *parent = nullptr);

  int getEntryId() const;
  QByteArray getGameData();
  QByteArray getSaveData();
  QString getCorePath();

  QString currentGameName() const;
  int nativeWidth() const;
  int nativeHeight() const;
  float nativeAspectRatio() const;

  bool takeShouldLoadGameFlag();
  bool takeShouldPauseGameFlag();
  bool takeShouldResumeGameFlag();
  bool takeShouldStartEmulationFlag();
  bool takeShouldStopEmulationFlag();
  bool takeShouldResetEmulationFlag();

  void setIsRunning(bool isRunning);
  void setCurrentEntry(LibEntry entry);
  void setNativeWidth(int nativeWidth);
  void setNativeHeight(int nativeHeight);
  void setNativeAspectRatio(float nativeAspectRatio);

public slots:
  void loadGame(int entryId, const QByteArray &gameData,
                const QByteArray &saveData, const QString &corePath);
  void pauseGame();
  void resumeGame();
  void startEmulation();
  void stopEmulation();
  void resetEmulation();
  bool isRunning();

signals:
  void gameLoaded();
  void gamePaused();
  void gameResumed();
  void emulationStarted();
  void emulationStopped();

  void currentGameNameChanged();
  void nativeWidthChanged();
  void nativeHeightChanged();
  void nativeAspectRatioChanged();

private:
  bool m_shouldLoadGame = false;
  bool m_shouldPauseGame = false;
  bool m_shouldResumeGame = false;
  bool m_shouldStartEmulation = false;
  bool m_shouldStopEmulation = false;
  bool m_shouldResetEmulation = false;

  int m_entryId;
  QByteArray m_gameData;
  QByteArray m_saveData;
  QString m_corePath;

  bool m_shouldShutdown = false;
  bool m_isRunning = false;

  QMetaObject::Connection m_renderConnection;
  LibEntry m_currentEntry;
  int m_nativeWidth = 0;
  int m_nativeHeight = 0;
  float m_nativeAspectRatio = 0;
  bool running = false;
};

#endif // FIRELIGHT_EMULATION_MANAGER_HPP

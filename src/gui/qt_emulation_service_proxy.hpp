#pragma once
#include "emulation/emulation_service.hpp"
#include "event_dispatcher.hpp"

#include <QObject>
#include <settings/settings_service.hpp>

namespace firelight::gui {

class QtEmulationServiceProxy final : public QObject {
  Q_OBJECT
  Q_PROPERTY(bool isGameRunning READ isGameRunning NOTIFY gameRunningChanged)
  Q_PROPERTY(QString currentGameName READ getCurrentGameName NOTIFY
                 currentGameNameChanged)
  Q_PROPERTY(
      int currentEntryId READ getCurrentEntryId NOTIFY currentGameNameChanged)
  Q_PROPERTY(QString currentContentHash READ getCurrentContentHash NOTIFY
                 gameRunningChanged)
  Q_PROPERTY(
      int currentPlatformId READ getCurrentPlatformId NOTIFY gameRunningChanged)
  Q_PROPERTY(
      bool rewindEnabled READ isRewindEnabled NOTIFY rewindEnabledChanged)
  Q_PROPERTY(QString pictureMode READ getPictureMode NOTIFY pictureModeChanged)
  Q_PROPERTY(QString aspectRatioMode READ getAspectRatioMode NOTIFY
                 aspectRatioModeChanged)

public:
  explicit QtEmulationServiceProxy(QObject *parent = nullptr);
  ~QtEmulationServiceProxy() override;

  bool isGameRunning() const;
  QString getCurrentGameName() const;
  QString getCurrentContentHash() const;
  int getCurrentEntryId() const;
  int getCurrentPlatformId() const;

  bool isRewindEnabled() const;
  QString getPictureMode() const;
  QString getAspectRatioMode() const;

  Q_INVOKABLE void loadEntry(int entryId);
  Q_INVOKABLE void stopEmulation();
  Q_INVOKABLE void resetGame();

signals:
  void gameLoaded();
  void emulationStopped();
  void gameRunningChanged(bool isGameRunning);
  void currentGameNameChanged();

  void rewindEnabledChanged();
  void pictureModeChanged();
  void aspectRatioModeChanged();

private:
  emulation::EmulationService *m_emulationService;

  ScopedConnection m_gameLoadedConnection;
  ScopedConnection m_emulationStartedConnection;
  ScopedConnection m_emulationStoppedConnection;

  ScopedConnection m_emulationSettingChangedConnection;
};

} // namespace firelight::gui
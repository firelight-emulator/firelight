#pragma once
#include "emulation/emulation_service.hpp"
#include "firelight/event_dispatcher.hpp"

#include <QObject>
#include <QVariant>
#include <firelight/settings/settings_service.hpp>

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
  Q_PROPERTY(QString currentPlatformName READ getCurrentPlatformName NOTIFY
                 gameRunningChanged)
  Q_PROPERTY(int currentSaveSlotNumber READ getCurrentSaveSlotNumber NOTIFY
                 gameRunningChanged)
  Q_PROPERTY(
      bool rewindEnabled READ isRewindEnabled NOTIFY rewindEnabledChanged)
  Q_PROPERTY(QString pictureMode READ getPictureMode NOTIFY pictureModeChanged)
  Q_PROPERTY(QString aspectRatioMode READ getAspectRatioMode NOTIFY
                 aspectRatioModeChanged)
  Q_PROPERTY(int integerScale READ getIntegerScale NOTIFY integerScaleChanged)

public:
  explicit QtEmulationServiceProxy(QObject *parent = nullptr);
  ~QtEmulationServiceProxy() override;

  bool isGameRunning() const;
  QString getCurrentGameName() const;
  QString getCurrentContentHash() const;
  int getCurrentEntryId() const;
  int getCurrentPlatformId() const;
  QString getCurrentPlatformName() const;
  int getCurrentSaveSlotNumber() const;

  bool isRewindEnabled() const;
  QString getPictureMode() const;
  QString getAspectRatioMode() const;
  int getIntegerScale() const;

  Q_INVOKABLE void loadEntry(int entryId);
  Q_INVOKABLE void stopEmulation();
  Q_INVOKABLE void resetGame();

  // --- per-game controller device selection (Mouse / Light Gun / Gamepad) ---
  // Number of controller ports the running game's core exposes
  Q_INVOKABLE int controllerPortCount() const;
  // For `port`, the selectable device variants as a list of maps:
  // { coreDeviceId:int, name:string, deviceClass:int (1=Joypad,2=Mouse,
  // 3=LightGun), isCurrent:bool }. Empty (or a single entry) means no real
  // choice — the UI hides ports with nothing to pick
  Q_INVOKABLE QVariantList controllerVariantsForPort(int port) const;
  // Selects a variant for `port` (applies live + persists for this game)
  Q_INVOKABLE void setControllerVariant(int port, int coreDeviceId);

signals:
  void gameLoadStarted();
  void gameLoaded();
  void emulationStopped();
  void gameRunningChanged(bool isGameRunning);
  void currentGameNameChanged();

  void rewindEnabledChanged();
  void pictureModeChanged();
  void aspectRatioModeChanged();
  void integerScaleChanged();
  void controllerDevicesChanged();

private:
  emulation::EmulationService *m_emulationService;

  ScopedConnection m_gameLoadedConnection;
  ScopedConnection m_emulationStartedConnection;
  ScopedConnection m_emulationStoppedConnection;

  ScopedConnection m_emulationSettingChangedConnection;
  ScopedConnection m_controllerDevicesConnection;
};

} // namespace firelight::gui

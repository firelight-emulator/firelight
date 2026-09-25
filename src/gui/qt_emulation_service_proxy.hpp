#pragma once
#include "emulation/emulation_service.hpp"
#include "firelight/event_dispatcher.hpp"

#include <firelight/settings/settings_service.hpp>

#include <QObject>
#include <QVariant>

namespace firelight::gui {

class QtEmulationServiceProxy final : public QObject {
  Q_OBJECT
  Q_PROPERTY(bool isGameRunning READ isGameRunning NOTIFY gameRunningChanged)
  Q_PROPERTY(QString currentGameName READ getCurrentGameName NOTIFY currentGameNameChanged)
  Q_PROPERTY(int currentEntryId READ getCurrentEntryId NOTIFY currentGameNameChanged)
  Q_PROPERTY(QString currentContentHash READ getCurrentContentHash NOTIFY gameRunningChanged)
  Q_PROPERTY(int currentPlatformId READ getCurrentPlatformId NOTIFY gameRunningChanged)
  Q_PROPERTY(QString currentPlatformName READ getCurrentPlatformName NOTIFY gameRunningChanged)
  Q_PROPERTY(int currentSaveSlotNumber READ getCurrentSaveSlotNumber NOTIFY gameRunningChanged)
  Q_PROPERTY(bool rewindEnabled READ isRewindEnabled NOTIFY rewindEnabledChanged)
  Q_PROPERTY(QString pictureMode READ getPictureMode NOTIFY pictureModeChanged)
  Q_PROPERTY(QString aspectRatioMode READ getAspectRatioMode NOTIFY aspectRatioModeChanged)
  Q_PROPERTY(int integerScale READ getIntegerScale NOTIFY integerScaleChanged)
  Q_PROPERTY(bool suspended READ isSuspended NOTIFY suspendedChanged)
  Q_PROPERTY(bool canUndoLoadSuspendPoint READ canUndoLoadSuspendPoint NOTIFY canUndoLoadSuspendPointChanged)

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

  /**
   * @return Whether the running game is off the foreground
   */
  bool isSuspended() const;

  /**
   * @return Whether the last suspend-point load can be undone
   */
  bool canUndoLoadSuspendPoint() const;

  Q_INVOKABLE void loadEntry(int entryId);
  Q_INVOKABLE void stopEmulation();
  Q_INVOKABLE void resetGame();

  /**
   * Takes the running game off the foreground
   */
  Q_INVOKABLE void suspend();

  /**
   * Hands the running game back to the foreground
   */
  Q_INVOKABLE void resume();

  /**
   * Queues a suspend-point write into the given slot on the running game
   */
  Q_INVOKABLE void writeSuspendPoint(int index);

  /**
   * Queues a suspend-point load from the given slot on the running game
   */
  Q_INVOKABLE void loadSuspendPoint(int index);

  /**
   * Queues an undo of the last suspend-point load on the running game
   */
  Q_INVOKABLE void undoLoadSuspendPoint();

  /**
   * Asks the running game for its rewind points, which the emulator page shows as the rewind menu
   */
  Q_INVOKABLE void openRewindMenu();

  // TODO: same as below. maybe a model
  Q_INVOKABLE int controllerPortCount() const;

  // TODO: Probably go through emulation service
  // For `port`, the selectable device variants as a list of maps:
  // { coreDeviceId:int, name:string, deviceClass:int (1=Joypad,2=Mouse,
  // 3=LightGun), isCurrent:bool }. Empty (or a single entry) means no real
  // choice — the UI hides ports with nothing to pick
  Q_INVOKABLE QVariantList controllerVariantsForPort(int port) const;

  // TODO: probably go straight to EmulationService for this
  Q_INVOKABLE void setControllerVariant(int port, int coreDeviceId);

signals:
  void gameLoadStarted();
  void gameLoaded();
  void gameLoadFailed(QString reason);
  void emulationStopped();
  void gameRunningChanged(bool isGameRunning);
  void currentGameNameChanged();

  void rewindEnabledChanged();
  void pictureModeChanged();
  void aspectRatioModeChanged();
  void integerScaleChanged();
  void controllerDevicesChanged();
  void suspendedChanged();
  void canUndoLoadSuspendPointChanged();

private:
  emulation::EmulationService *m_emulationService;

  ScopedConnection m_gameLoadedConnection;
  ScopedConnection m_gameLoadFailedConnection;
  ScopedConnection m_emulationStartedConnection;
  ScopedConnection m_emulationStoppedConnection;

  ScopedConnection m_emulationSettingChangedConnection;
  ScopedConnection m_controllerDevicesConnection;
  ScopedConnection m_suspendedConnection;
  ScopedConnection m_undoLoadSuspendPointConnection;
};

} // namespace firelight::gui

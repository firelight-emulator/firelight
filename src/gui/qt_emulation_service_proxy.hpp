#pragma once
#include "emulation/emulation_service.hpp"
#include "event_dispatcher.hpp"

#include <QObject>

namespace firelight::gui {

class QtEmulationServiceProxy final : public QObject {
  Q_OBJECT
  Q_PROPERTY(bool isGameRunning READ isGameRunning NOTIFY gameRunningChanged)
  Q_PROPERTY(QString currentGameName READ getCurrentGameName NOTIFY
                 currentGameNameChanged)
  Q_PROPERTY(
      int currentEntryId READ getCurrentEntryId NOTIFY currentGameNameChanged)

public:
  explicit QtEmulationServiceProxy(QObject *parent = nullptr);
  ~QtEmulationServiceProxy() override;

  bool isGameRunning() const;
  QString getCurrentGameName() const;
  int getCurrentEntryId() const;

  Q_INVOKABLE void loadEntry(int entryId);
  Q_INVOKABLE void stopEmulation();
  Q_INVOKABLE void resetGame();

signals:
  void gameLoaded();
  void emulationStopped();
  void gameRunningChanged(bool isGameRunning);
  void currentGameNameChanged();

private:
  emulation::EmulationService *m_emulationService;

  ScopedConnection m_gameLoadedConnection;
  ScopedConnection m_emulationStartedConnection;
  ScopedConnection m_emulationStoppedConnection;
};

} // namespace firelight::gui
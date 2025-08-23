#pragma once
#include "emulation/emulation_service.hpp"
#include "event_dispatcher.hpp"

#include <QObject>

namespace firelight::gui {

class QtEmulationServiceProxy final : public QObject {
  Q_OBJECT
  Q_PROPERTY(bool isGameRunning READ isGameRunning NOTIFY gameRunningChanged)

public:
  explicit QtEmulationServiceProxy(QObject *parent = nullptr);
  ~QtEmulationServiceProxy() override;

  bool isGameRunning() const;

  Q_INVOKABLE void loadEntry(int entryId);
  Q_INVOKABLE void stopEmulation();

signals:
  void gameLoaded();
  void gameRunningChanged(bool isGameRunning);

private:
  emulation::EmulationService *m_emulationService;

  ScopedConnection m_gameLoadedConnection;
  ScopedConnection m_emulationStartedConnection;
  ScopedConnection m_emulationStoppedConnection;
};

} // namespace firelight::gui
#include "qt_emulation_service_proxy.hpp"

#include "emulation/emulation_service.hpp"

#include <qtconcurrentrun.h>
#include <spdlog/spdlog.h>

namespace firelight::gui {
QtEmulationServiceProxy::QtEmulationServiceProxy(QObject *parent)
    : QObject(parent),
      m_emulationService(emulation::EmulationService::getInstance()) {

  m_gameLoadedConnection =
      EventDispatcher::instance().subscribe<emulation::GameLoadedEvent>(
          [this](emulation::GameLoadedEvent) { emit gameLoaded(); });

  m_emulationStartedConnection =
      EventDispatcher::instance().subscribe<emulation::EmulationStartedEvent>(
          [this](emulation::EmulationStartedEvent) {
            emit gameRunningChanged(true);
          });

  m_emulationStoppedConnection =
      EventDispatcher::instance().subscribe<emulation::EmulationStoppedEvent>(
          [this](emulation::EmulationStoppedEvent) {
            emit gameRunningChanged(false);
          });
}
QtEmulationServiceProxy::~QtEmulationServiceProxy() {}

bool QtEmulationServiceProxy::isGameRunning() const {
  return m_emulationService->isGameRunning();
}

void QtEmulationServiceProxy::loadEntry(const int entryId) {
  QThreadPool::globalInstance()->start(
      [this, entryId] { m_emulationService->loadEntry(entryId); });
}
void QtEmulationServiceProxy::stopEmulation() {
  m_emulationService->stopEmulation();
}
} // namespace firelight::gui
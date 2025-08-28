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
          [this](emulation::GameLoadedEvent) {
            emit gameLoaded();
            emit currentGameNameChanged();
          });

  m_emulationStartedConnection =
      EventDispatcher::instance().subscribe<emulation::EmulationStartedEvent>(
          [this](const emulation::EmulationStartedEvent &) {
            emit gameRunningChanged(true);
          });

  m_emulationStoppedConnection =
      EventDispatcher::instance().subscribe<emulation::EmulationStoppedEvent>(
          [this](emulation::EmulationStoppedEvent) {
            emit emulationStopped();
            emit gameRunningChanged(false);
            emit currentGameNameChanged();
          });
}
QtEmulationServiceProxy::~QtEmulationServiceProxy() = default;

bool QtEmulationServiceProxy::isGameRunning() const {
  return m_emulationService->isGameRunning();
}
QString QtEmulationServiceProxy::getCurrentGameName() const {
  return QString::fromStdString(
      m_emulationService->getCurrentGameName().value_or(""));
}
int QtEmulationServiceProxy::getCurrentEntryId() const {
  const auto entry = m_emulationService->getCurrentEntry();
  if (entry.has_value()) {
    return entry->id;
  }

  return -1;
}

void QtEmulationServiceProxy::loadEntry(const int entryId) {
  QThreadPool::globalInstance()->start(
      [this, entryId] { m_emulationService->loadEntry(entryId); });
}
void QtEmulationServiceProxy::stopEmulation() {
  m_emulationService->stopEmulation();
}
void QtEmulationServiceProxy::resetGame() {
  const auto instance = m_emulationService->getCurrentEmulatorInstance();
  if (instance) {
    instance->reset();
  }
}
} // namespace firelight::gui
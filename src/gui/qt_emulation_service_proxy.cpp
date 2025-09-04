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
            emit rewindEnabledChanged();
            emit pictureModeChanged();
            emit aspectRatioModeChanged();
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

  m_emulationSettingChangedConnection =
      EventDispatcher::instance()
          .subscribe<settings::EmulationSettingChangedEvent>(
              [this](const settings::EmulationSettingChangedEvent &e) {
                spdlog::info("Emulation setting changed: {} = {}", e.key,
                             e.contentHash);
                if (e.key == "rewind-enabled") {
                  emit rewindEnabledChanged();
                } else if (e.key == "picture-mode") {
                  emit pictureModeChanged();
                } else if (e.key == "aspect-ratio") {
                  emit aspectRatioModeChanged();
                }

                emit currentSettingsLevelChanged();
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
QString QtEmulationServiceProxy::getCurrentContentHash() const {
  const auto entry = m_emulationService->getCurrentEntry();
  if (entry.has_value()) {
    return entry->contentHash;
  }

  return "";
}
int QtEmulationServiceProxy::getCurrentSettingsLevel() const {
  const auto contentHash = getCurrentContentHash();

  return settings::SettingsService::instance()->getSettingsLevel(
      contentHash.toStdString());
}

int QtEmulationServiceProxy::getCurrentEntryId() const {
  const auto entry = m_emulationService->getCurrentEntry();
  if (entry.has_value()) {
    return entry->id;
  }

  return -1;
}
int QtEmulationServiceProxy::getCurrentPlatformId() const {
  const auto entry = m_emulationService->getCurrentEntry();
  if (entry.has_value()) {
    return entry->platformId;
  }

  return -1;
}
QString QtEmulationServiceProxy::getCurrentPlatformName() const {
  const auto platform = m_emulationService->getCurrentPlatform();
  if (platform.has_value()) {
    return QString::fromStdString(platform->name);
  }

  return {};
}
bool QtEmulationServiceProxy::isRewindEnabled() const {
  auto instance = m_emulationService->getCurrentEmulatorInstance();
  if (!instance) {
    return false;
  }

  return instance->isRewindEnabled();
}
QString QtEmulationServiceProxy::getPictureMode() const {
  const auto instance = m_emulationService->getCurrentEmulatorInstance();
  if (!instance) {
    return "";
  }

  return QString::fromStdString(instance->getPictureMode());
}
QString QtEmulationServiceProxy::getAspectRatioMode() const {
  const auto instance = m_emulationService->getCurrentEmulatorInstance();
  if (!instance) {
    return "";
  }

  return QString::fromStdString(instance->getAspectRatioMode());
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
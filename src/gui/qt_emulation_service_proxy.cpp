#include "qt_emulation_service_proxy.hpp"

#include "emulation/emulation_service.hpp"
#include "emulator_item_renderer.hpp"

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
            emit borderChanged();
            emit shaderChanged();
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
                } else if (e.key == "integer-scale") {
                  emit integerScaleChanged();
                } else if (e.key == "border") {
                  emit borderChanged();
                } else if (e.key == "shader") {
                  emit shaderChanged();
                }
              });

  m_controllerDevicesConnection =
      EventDispatcher::instance().subscribe<emulation::ControllerDevicesEvent>(
          [this](const emulation::ControllerDevicesEvent &) {
            emit controllerDevicesChanged();
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
    return QString::fromStdString(entry->contentHash);
  }

  return "";
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
int QtEmulationServiceProxy::getCurrentSaveSlotNumber() const {
  auto instance = m_emulationService->getCurrentEmulatorInstance();
  if (!instance) {
    return -1;
  }

  return instance->getSaveSlotNumber();
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
int QtEmulationServiceProxy::getIntegerScale() const {

  const auto instance = m_emulationService->getCurrentEmulatorInstance();
  if (!instance) {
    return 0;
  }

  return instance->getIntegerScale();
}
QString QtEmulationServiceProxy::getBorder() const {
  const auto instance = m_emulationService->getCurrentEmulatorInstance();
  if (!instance) {
    return "";
  }

  return QString::fromStdString(instance->getBorder());
}
QString QtEmulationServiceProxy::getShader() const {
  const auto instance = m_emulationService->getCurrentEmulatorInstance();
  if (!instance) {
    return "";
  }

  return QString::fromStdString(instance->getShader());
}

void QtEmulationServiceProxy::loadEntry(const int entryId) {
  // Fired immediately on the GUI thread so the UI can react to the launch (e.g.
  // fade to black) before the load runs on the pool.
  emit gameLoadStarted();
  QThreadPool::globalInstance()->start(
      [this, entryId] { m_emulationService->loadEntry(entryId); });
}
QString QtEmulationServiceProxy::captureCurrentFrame() const {
  if (!m_emulationService->isGameRunning()) {
    return {};
  }
  return EmulatorItemRenderer::captureCurrentFramePreviewUrl();
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

int QtEmulationServiceProxy::controllerPortCount() const {
  const auto instance = m_emulationService->getCurrentEmulatorInstance();
  return instance ? static_cast<int>(instance->getControllerPortCount()) : 0;
}

QVariantList
QtEmulationServiceProxy::controllerVariantsForPort(const int port) const {
  QVariantList result;
  const auto instance = m_emulationService->getCurrentEmulatorInstance();
  if (!instance || port < 0) {
    return result;
  }
  const auto p = static_cast<unsigned>(port);
  const auto selected = instance->getSelectedControllerVariant(p);
  for (const auto &variant : instance->getAvailableControllerVariants(p)) {
    QVariantMap entry;
    entry["coreDeviceId"] = static_cast<int>(variant.coreDeviceId);
    entry["name"] = QString::fromStdString(variant.friendlyName);
    entry["deviceClass"] = static_cast<int>(variant.deviceClass);
    entry["isCurrent"] = variant.coreDeviceId == selected;
    result.append(entry);
  }
  return result;
}

void QtEmulationServiceProxy::setControllerVariant(const int port,
                                                   const int coreDeviceId) {
  const auto instance = m_emulationService->getCurrentEmulatorInstance();
  if (!instance || port < 0) {
    return;
  }
  instance->setPortControllerVariant(static_cast<unsigned>(port),
                                     static_cast<unsigned>(coreDeviceId));
  emit controllerDevicesChanged();
}
} // namespace firelight::gui
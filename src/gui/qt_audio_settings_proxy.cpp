#include "qt_audio_settings_proxy.hpp"

#include <QAudioDevice>
#include <QSettings>

namespace firelight::gui {

namespace {
constexpr auto kSettingsKey = "audio/outputDevice";
const QString kDefaultLabel = QStringLiteral("System default");
} // namespace

QtAudioSettingsProxy::QtAudioSettingsProxy(QObject *parent) : QObject(parent) {
  m_mediaDevices = new QMediaDevices(this);
  connect(m_mediaDevices, &QMediaDevices::audioOutputsChanged, this,
          &QtAudioSettingsProxy::devicesChanged);
}

QStringList QtAudioSettingsProxy::availableDevices() const {
  QStringList devices;
  devices << kDefaultLabel;
  for (const auto &device : QMediaDevices::audioOutputs()) {
    devices << device.description();
  }
  return devices;
}

QString QtAudioSettingsProxy::outputDevice() const {
  const QString description = QSettings().value(kSettingsKey).toString();
  return description.isEmpty() ? kDefaultLabel : description;
}

void QtAudioSettingsProxy::setOutputDevice(const QString &description) {
  const QString stored =
      (description == kDefaultLabel) ? QString() : description;
  QSettings settings;
  if (settings.value(kSettingsKey).toString() == stored) {
    return;
  }
  settings.setValue(kSettingsKey, stored);
  emit outputDeviceChanged();
}

} // namespace firelight::gui

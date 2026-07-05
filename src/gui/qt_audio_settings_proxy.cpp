#include "qt_audio_settings_proxy.hpp"

#include <QAudioDevice>
#include <QSettings>

namespace firelight::gui {

namespace {
constexpr auto SETTINGS_KEY = "audio/outputDevice";
const QString DEFAULT_LABEL = QStringLiteral("System default");
} // namespace

QtAudioSettingsProxy::QtAudioSettingsProxy(QObject *parent) : QObject(parent) {
  m_mediaDevices = new QMediaDevices(this);
  connect(m_mediaDevices, &QMediaDevices::audioOutputsChanged, this,
          &QtAudioSettingsProxy::devicesChanged);
}

QStringList QtAudioSettingsProxy::availableDevices() const {
  QStringList devices;
  devices << DEFAULT_LABEL;
  for (const auto &device : QMediaDevices::audioOutputs()) {
    devices << device.description();
  }
  return devices;
}

QString QtAudioSettingsProxy::outputDevice() const {
  const QString description = QSettings().value(SETTINGS_KEY).toString();
  return description.isEmpty() ? DEFAULT_LABEL : description;
}

void QtAudioSettingsProxy::setOutputDevice(const QString &description) {
  const QString stored =
      (description == DEFAULT_LABEL) ? QString() : description;
  QSettings settings;
  if (settings.value(SETTINGS_KEY).toString() == stored) {
    return;
  }
  settings.setValue(SETTINGS_KEY, stored);
  emit outputDeviceChanged();
}

} // namespace firelight::gui

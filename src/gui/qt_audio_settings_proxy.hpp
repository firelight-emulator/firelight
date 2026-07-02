#pragma once

#include <QMediaDevices>
#include <QObject>
#include <QStringList>

namespace firelight::gui {

// App-level audio settings for QML: lists the available output devices and the
// user's chosen one (persisted via QSettings under "audio/outputDevice"; empty
// = system default). AudioManager reads the same setting when it (re)opens the
// output, so a change applies on the next game launch.
class QtAudioSettingsProxy : public QObject {
  Q_OBJECT
  Q_PROPERTY(QStringList availableDevices READ availableDevices NOTIFY
                 devicesChanged)
  Q_PROPERTY(QString outputDevice READ outputDevice WRITE setOutputDevice NOTIFY
                 outputDeviceChanged)
public:
  explicit QtAudioSettingsProxy(QObject *parent = nullptr);

  [[nodiscard]] QStringList availableDevices() const;
  [[nodiscard]] QString outputDevice() const;
  void setOutputDevice(const QString &description);

signals:
  void devicesChanged();
  void outputDeviceChanged();

private:
  QMediaDevices *m_mediaDevices;
};

} // namespace firelight::gui

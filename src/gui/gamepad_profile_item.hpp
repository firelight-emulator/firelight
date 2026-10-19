// TODO: NEEDS REVIEW
#pragma once
#include "service_accessor.hpp"

#include <firelight/event_dispatcher.hpp>
#include <firelight/input/analog_settings.hpp>
#include <firelight/input/gamepad_profile.hpp>

#include <QQuickItem>
#include <input/gui/analog_fields.hpp>

namespace firelight::gui {

// TODO
/** One controller profile for QML: its name, icon, and stick and trigger tuning */
class GamepadProfileItem : public QQuickItem, public ServiceAccessor {
  Q_OBJECT
  Q_PROPERTY(int profileId READ id WRITE setId NOTIFY profileIdChanged)
  Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
  Q_PROPERTY(QString icon READ icon NOTIFY profileIdChanged)
  Q_PROPERTY(bool isKeyboardProfile READ isKeyboardProfile NOTIFY profileIdChanged)

  Q_PROPERTY(
      double leftStickInnerDeadzone READ leftStickInnerDeadzone WRITE setLeftStickInnerDeadzone NOTIFY analogChanged)
  Q_PROPERTY(
      double leftStickOuterDeadzone READ leftStickOuterDeadzone WRITE setLeftStickOuterDeadzone NOTIFY analogChanged)
  Q_PROPERTY(double leftStickSensitivity READ leftStickSensitivity WRITE setLeftStickSensitivity NOTIFY analogChanged)
  Q_PROPERTY(
      double leftStickAntiDeadzone READ leftStickAntiDeadzone WRITE setLeftStickAntiDeadzone NOTIFY analogChanged)
  Q_PROPERTY(int leftStickCurve READ leftStickCurve WRITE setLeftStickCurve NOTIFY analogChanged)
  Q_PROPERTY(
      double leftStickCurveExponent READ leftStickCurveExponent WRITE setLeftStickCurveExponent NOTIFY analogChanged)

  Q_PROPERTY(
      double rightStickInnerDeadzone READ rightStickInnerDeadzone WRITE setRightStickInnerDeadzone NOTIFY analogChanged)
  Q_PROPERTY(
      double rightStickOuterDeadzone READ rightStickOuterDeadzone WRITE setRightStickOuterDeadzone NOTIFY analogChanged)
  Q_PROPERTY(
      double rightStickSensitivity READ rightStickSensitivity WRITE setRightStickSensitivity NOTIFY analogChanged)
  Q_PROPERTY(
      double rightStickAntiDeadzone READ rightStickAntiDeadzone WRITE setRightStickAntiDeadzone NOTIFY analogChanged)
  Q_PROPERTY(int rightStickCurve READ rightStickCurve WRITE setRightStickCurve NOTIFY analogChanged)
  Q_PROPERTY(
      double rightStickCurveExponent READ rightStickCurveExponent WRITE setRightStickCurveExponent NOTIFY analogChanged)

  Q_PROPERTY(double leftTriggerThreshold READ leftTriggerThreshold WRITE setLeftTriggerThreshold NOTIFY analogChanged)
  Q_PROPERTY(
      double rightTriggerThreshold READ rightTriggerThreshold WRITE setRightTriggerThreshold NOTIFY analogChanged)

public:
  explicit GamepadProfileItem(QQuickItem *parent = nullptr);

  int id() const;
  void setId(int id);
  QString name() const;
  void setName(const QString &name);
  QString icon() const;
  bool isKeyboardProfile() const;

  double leftStickInnerDeadzone() const;
  void setLeftStickInnerDeadzone(double value);
  double leftStickOuterDeadzone() const;
  void setLeftStickOuterDeadzone(double value);
  double leftStickSensitivity() const;
  void setLeftStickSensitivity(double value);
  double leftStickAntiDeadzone() const;
  void setLeftStickAntiDeadzone(double value);
  int leftStickCurve() const;
  void setLeftStickCurve(int value);
  double leftStickCurveExponent() const;
  void setLeftStickCurveExponent(double value);

  double rightStickInnerDeadzone() const;
  void setRightStickInnerDeadzone(double value);
  double rightStickOuterDeadzone() const;
  void setRightStickOuterDeadzone(double value);
  double rightStickSensitivity() const;
  void setRightStickSensitivity(double value);
  double rightStickAntiDeadzone() const;
  void setRightStickAntiDeadzone(double value);
  int rightStickCurve() const;
  void setRightStickCurve(int value);
  double rightStickCurveExponent() const;
  void setRightStickCurveExponent(double value);

  double leftTriggerThreshold() const;
  void setLeftTriggerThreshold(double value);
  double rightTriggerThreshold() const;
  void setRightTriggerThreshold(double value);

  // TODO
  /** Puts every stick and trigger value back to the library defaults */
  Q_INVOKABLE void resetAnalogToDefault();

signals:
  void profileIdChanged(int id);
  void nameChanged(const QString &name);
  void analogChanged();

private:
  // TODO
  /** Persists the given tuning, keeps it as the current one and announces the change */
  void writeAnalog(const input::AnalogSettings &updated);

  // TODO
  /** Re-reads the stored name and tuning and signals what differs */
  void reloadFromRepository();

  int m_id = -1;
  std::shared_ptr<input::GamepadProfile> m_profile;
  QString m_name;
  input::AnalogSettings m_analog;

  ScopedConnection m_settingsChangedHandler;
};

} // namespace firelight::gui

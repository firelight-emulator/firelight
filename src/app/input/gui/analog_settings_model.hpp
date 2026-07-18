#pragma once

#include "service_accessor.hpp"

#include <firelight/input/analog_settings.hpp>
#include <firelight/input/gamepad_profile.hpp>

#include <QObject>
#include <memory>
#include <qqmlintegration.h>

namespace firelight::input {

// TODO
// QML-facing editor for a profile's default analog tuning. Bind `profileId`,
// then read/write the per-axis properties; every change is persisted to the
// profile immediately. Curve is exposed as an int (0 = Linear, 1 = Exponential)
// for easy use in QML combo boxes
class AnalogSettingsModel : public QObject, public ServiceAccessor {
  Q_OBJECT
  Q_PROPERTY(int profileId READ profileId WRITE setProfileId NOTIFY profileIdChanged)

  Q_PROPERTY(double leftStickInnerDeadzone READ leftStickInnerDeadzone WRITE
                 setLeftStickInnerDeadzone NOTIFY changed)
  Q_PROPERTY(double leftStickOuterDeadzone READ leftStickOuterDeadzone WRITE
                 setLeftStickOuterDeadzone NOTIFY changed)
  Q_PROPERTY(double leftStickSensitivity READ leftStickSensitivity WRITE
                 setLeftStickSensitivity NOTIFY changed)
  Q_PROPERTY(double leftStickAntiDeadzone READ leftStickAntiDeadzone WRITE
                 setLeftStickAntiDeadzone NOTIFY changed)
  Q_PROPERTY(int leftStickCurve READ leftStickCurve WRITE setLeftStickCurve
                 NOTIFY changed)
  Q_PROPERTY(double leftStickCurveExponent READ leftStickCurveExponent WRITE
                 setLeftStickCurveExponent NOTIFY changed)

  Q_PROPERTY(double rightStickInnerDeadzone READ rightStickInnerDeadzone WRITE
                 setRightStickInnerDeadzone NOTIFY changed)
  Q_PROPERTY(double rightStickOuterDeadzone READ rightStickOuterDeadzone WRITE
                 setRightStickOuterDeadzone NOTIFY changed)
  Q_PROPERTY(double rightStickSensitivity READ rightStickSensitivity WRITE
                 setRightStickSensitivity NOTIFY changed)
  Q_PROPERTY(double rightStickAntiDeadzone READ rightStickAntiDeadzone WRITE
                 setRightStickAntiDeadzone NOTIFY changed)
  Q_PROPERTY(int rightStickCurve READ rightStickCurve WRITE setRightStickCurve
                 NOTIFY changed)
  Q_PROPERTY(double rightStickCurveExponent READ rightStickCurveExponent WRITE
                 setRightStickCurveExponent NOTIFY changed)

  Q_PROPERTY(double leftTriggerThreshold READ leftTriggerThreshold WRITE
                 setLeftTriggerThreshold NOTIFY changed)
  Q_PROPERTY(double rightTriggerThreshold READ rightTriggerThreshold WRITE
                 setRightTriggerThreshold NOTIFY changed)

public:
  explicit AnalogSettingsModel(QObject *parent = nullptr);

  int profileId() const;
  void setProfileId(int id);

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

  // Restores the profile's analog settings to the library defaults
  Q_INVOKABLE void resetToDefault();

signals:
  void profileIdChanged();
  void changed();

private:
  void load();
  void persist();

  int m_profileId = -1;
  std::shared_ptr<GamepadProfile> m_profile;
  AnalogSettings m_settings;
};

} // namespace firelight::input

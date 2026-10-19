// TODO: NEEDS REVIEW
#pragma once

#include <firelight/input/analog_settings.hpp>

#include <QVariant>
#include <array>

namespace firelight::input {

// TODO
/** One stick or trigger value of an AnalogSettings, named the way QML sees it */
enum class AnalogField {
  LeftStickInnerDeadzone,
  LeftStickOuterDeadzone,
  LeftStickSensitivity,
  LeftStickAntiDeadzone,
  LeftStickCurve,
  LeftStickCurveExponent,
  RightStickInnerDeadzone,
  RightStickOuterDeadzone,
  RightStickSensitivity,
  RightStickAntiDeadzone,
  RightStickCurve,
  RightStickCurveExponent,
  LeftTriggerThreshold,
  RightTriggerThreshold,
};

constexpr std::array ANALOG_FIELDS = {
    AnalogField::LeftStickInnerDeadzone,  AnalogField::LeftStickOuterDeadzone,  AnalogField::LeftStickSensitivity,
    AnalogField::LeftStickAntiDeadzone,   AnalogField::LeftStickCurve,          AnalogField::LeftStickCurveExponent,
    AnalogField::RightStickInnerDeadzone, AnalogField::RightStickOuterDeadzone, AnalogField::RightStickSensitivity,
    AnalogField::RightStickAntiDeadzone,  AnalogField::RightStickCurve,         AnalogField::RightStickCurveExponent,
    AnalogField::LeftTriggerThreshold,    AnalogField::RightTriggerThreshold,
};

// TODO
/** The QML name of a field, such as leftStickInnerDeadzone */
const char *getAnalogFieldName(AnalogField field);

// TODO
/** A field's value: a double, or an int for a curve */
QVariant getAnalogField(const AnalogSettings &settings, AnalogField field);

// TODO
/** Writes a field from a QML value, refusing one that does not convert or matches the stored one */
bool setAnalogField(AnalogSettings &settings, AnalogField field, const QVariant &value);

} // namespace firelight::input

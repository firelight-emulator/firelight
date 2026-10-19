// TODO: NEEDS REVIEW
#include "analog_fields.hpp"

namespace firelight::input {

namespace {
// TODO
/** The float a field reads and writes, or null for a curve */
const float *getFloatOf(const AnalogSettings &settings, const AnalogField field) {
  switch (field) {
  case AnalogField::LeftStickInnerDeadzone:
    return &settings.leftStick.innerDeadzone;
  case AnalogField::LeftStickOuterDeadzone:
    return &settings.leftStick.outerDeadzone;
  case AnalogField::LeftStickSensitivity:
    return &settings.leftStick.sensitivity;
  case AnalogField::LeftStickAntiDeadzone:
    return &settings.leftStick.antiDeadzone;
  case AnalogField::LeftStickCurveExponent:
    return &settings.leftStick.curveExponent;
  case AnalogField::RightStickInnerDeadzone:
    return &settings.rightStick.innerDeadzone;
  case AnalogField::RightStickOuterDeadzone:
    return &settings.rightStick.outerDeadzone;
  case AnalogField::RightStickSensitivity:
    return &settings.rightStick.sensitivity;
  case AnalogField::RightStickAntiDeadzone:
    return &settings.rightStick.antiDeadzone;
  case AnalogField::RightStickCurveExponent:
    return &settings.rightStick.curveExponent;
  case AnalogField::LeftTriggerThreshold:
    return &settings.leftTrigger.threshold;
  case AnalogField::RightTriggerThreshold:
    return &settings.rightTrigger.threshold;
  case AnalogField::LeftStickCurve:
  case AnalogField::RightStickCurve:
    return nullptr;
  }

  return nullptr;
}

// TODO
/** The curve a field reads and writes, or null for a float field */
const ResponseCurve *getCurveOf(const AnalogSettings &settings, const AnalogField field) {
  switch (field) {
  case AnalogField::LeftStickCurve:
    return &settings.leftStick.curve;
  case AnalogField::RightStickCurve:
    return &settings.rightStick.curve;
  default:
    return nullptr;
  }
}

// TODO
/** Whether an int names a ResponseCurve */
bool isCurveValue(const int value) {
  return value == static_cast<int>(ResponseCurve::Linear) || value == static_cast<int>(ResponseCurve::Exponential);
}
} // namespace

const char *getAnalogFieldName(const AnalogField field) {
  switch (field) {
  case AnalogField::LeftStickInnerDeadzone:
    return "leftStickInnerDeadzone";
  case AnalogField::LeftStickOuterDeadzone:
    return "leftStickOuterDeadzone";
  case AnalogField::LeftStickSensitivity:
    return "leftStickSensitivity";
  case AnalogField::LeftStickAntiDeadzone:
    return "leftStickAntiDeadzone";
  case AnalogField::LeftStickCurve:
    return "leftStickCurve";
  case AnalogField::LeftStickCurveExponent:
    return "leftStickCurveExponent";
  case AnalogField::RightStickInnerDeadzone:
    return "rightStickInnerDeadzone";
  case AnalogField::RightStickOuterDeadzone:
    return "rightStickOuterDeadzone";
  case AnalogField::RightStickSensitivity:
    return "rightStickSensitivity";
  case AnalogField::RightStickAntiDeadzone:
    return "rightStickAntiDeadzone";
  case AnalogField::RightStickCurve:
    return "rightStickCurve";
  case AnalogField::RightStickCurveExponent:
    return "rightStickCurveExponent";
  case AnalogField::LeftTriggerThreshold:
    return "leftTriggerThreshold";
  case AnalogField::RightTriggerThreshold:
    return "rightTriggerThreshold";
  }

  return "";
}

QVariant getAnalogField(const AnalogSettings &settings, const AnalogField field) {
  if (const auto curve = getCurveOf(settings, field)) {
    return static_cast<int>(*curve);
  }

  if (const auto value = getFloatOf(settings, field)) {
    return static_cast<double>(*value);
  }

  return {};
}

bool setAnalogField(AnalogSettings &settings, const AnalogField field, const QVariant &value) {
  if (const auto curve = const_cast<ResponseCurve *>(getCurveOf(settings, field))) {
    bool isInt = false;
    const auto number = value.toInt(&isInt);
    if (!isInt || !isCurveValue(number) || static_cast<ResponseCurve>(number) == *curve) {
      return false;
    }

    *curve = static_cast<ResponseCurve>(number);
    return true;
  }

  const auto target = const_cast<float *>(getFloatOf(settings, field));
  if (!target) {
    return false;
  }

  bool isNumber = false;
  const auto number = static_cast<float>(value.toDouble(&isNumber));
  if (!isNumber || number == *target) {
    return false;
  }

  *target = number;
  return true;
}

} // namespace firelight::input

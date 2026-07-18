#include "analog_settings_model.hpp"

#include <firelight/input/controller_repository.hpp>

namespace firelight::input {

AnalogSettingsModel::AnalogSettingsModel(QObject *parent) : QObject(parent) {}

int AnalogSettingsModel::profileId() const { return m_profileId; }

void AnalogSettingsModel::setProfileId(const int id) {
  if (id == m_profileId) {
    return;
  }
  m_profileId = id;
  load();
  emit profileIdChanged();
  emit changed();
}

void AnalogSettingsModel::load() {
  const auto service = getControllerProfileRepository();
  if (!service || m_profileId < 0) {
    m_profile = nullptr;
    m_settings = AnalogSettings{};
    return;
  }
  m_profile = service->getProfile(m_profileId);
  m_settings = m_profile ? m_profile->getDefaultAnalogSettings() : AnalogSettings{};
}

void AnalogSettingsModel::persist() {
  const auto service = getControllerProfileRepository();
  if (!service || m_profileId < 0) {
    return;
  }
  service->setProfileAnalogSettings(m_profileId, m_settings);
}

// Helper macro to cut down on near-identical getter/setter boilerplate. Each
// setter clamps reasonable ranges implicitly via the UI; here we only guard
// against no-op writes and persist + notify on real changes
#define FL_ANALOG_FLOAT_PROP(getterName, setterName, member)                                                           \
  double AnalogSettingsModel::getterName() const { return member; }                                                    \
  void AnalogSettingsModel::setterName(const double value) {                                                           \
    if (static_cast<float>(value) == member) {                                                                         \
      return;                                                                                                          \
    }                                                                                                                  \
    member = static_cast<float>(value);                                                                                \
    persist();                                                                                                         \
    emit changed();                                                                                                    \
  }

FL_ANALOG_FLOAT_PROP(leftStickInnerDeadzone, setLeftStickInnerDeadzone, m_settings.leftStick.innerDeadzone)
FL_ANALOG_FLOAT_PROP(leftStickOuterDeadzone, setLeftStickOuterDeadzone, m_settings.leftStick.outerDeadzone)
FL_ANALOG_FLOAT_PROP(leftStickSensitivity, setLeftStickSensitivity, m_settings.leftStick.sensitivity)
FL_ANALOG_FLOAT_PROP(leftStickAntiDeadzone, setLeftStickAntiDeadzone, m_settings.leftStick.antiDeadzone)
FL_ANALOG_FLOAT_PROP(leftStickCurveExponent, setLeftStickCurveExponent, m_settings.leftStick.curveExponent)

FL_ANALOG_FLOAT_PROP(rightStickInnerDeadzone, setRightStickInnerDeadzone, m_settings.rightStick.innerDeadzone)
FL_ANALOG_FLOAT_PROP(rightStickOuterDeadzone, setRightStickOuterDeadzone, m_settings.rightStick.outerDeadzone)
FL_ANALOG_FLOAT_PROP(rightStickSensitivity, setRightStickSensitivity, m_settings.rightStick.sensitivity)
FL_ANALOG_FLOAT_PROP(rightStickAntiDeadzone, setRightStickAntiDeadzone, m_settings.rightStick.antiDeadzone)
FL_ANALOG_FLOAT_PROP(rightStickCurveExponent, setRightStickCurveExponent, m_settings.rightStick.curveExponent)

FL_ANALOG_FLOAT_PROP(leftTriggerThreshold, setLeftTriggerThreshold, m_settings.leftTrigger.threshold)
FL_ANALOG_FLOAT_PROP(rightTriggerThreshold, setRightTriggerThreshold, m_settings.rightTrigger.threshold)

#undef FL_ANALOG_FLOAT_PROP

int AnalogSettingsModel::leftStickCurve() const { return static_cast<int>(m_settings.leftStick.curve); }

void AnalogSettingsModel::setLeftStickCurve(const int value) {
  const auto curve = static_cast<ResponseCurve>(value);
  if (curve == m_settings.leftStick.curve) {
    return;
  }
  m_settings.leftStick.curve = curve;
  persist();
  emit changed();
}

int AnalogSettingsModel::rightStickCurve() const { return static_cast<int>(m_settings.rightStick.curve); }

void AnalogSettingsModel::setRightStickCurve(const int value) {
  const auto curve = static_cast<ResponseCurve>(value);
  if (curve == m_settings.rightStick.curve) {
    return;
  }
  m_settings.rightStick.curve = curve;
  persist();
  emit changed();
}

void AnalogSettingsModel::resetToDefault() {
  m_settings = AnalogSettings{};
  persist();
  emit changed();
}

} // namespace firelight::input

// TODO: NEEDS REVIEW
#include "gamepad_profile_item.hpp"

#include <firelight/input/controller_repository.hpp>

#include <input/profile_events.hpp>

namespace firelight::gui {

GamepadProfileItem::GamepadProfileItem(QQuickItem *parent) : QQuickItem(parent) {
  m_settingsChangedHandler = EventDispatcher::instance().subscribe<input::ProfileSettingsChangedEvent>(
      [this](const input::ProfileSettingsChangedEvent &event) {
        const auto profileId = event.profileId;
        QMetaObject::invokeMethod(
            this,
            [this, profileId] {
              if (profileId == m_id) {
                reloadFromRepository();
              }
            },
            Qt::QueuedConnection);
      });
}

int GamepadProfileItem::id() const { return m_id; }

void GamepadProfileItem::setId(const int id) {
  if (m_id == id) {
    return;
  }
  m_id = id;

  const auto repository = getControllerProfileRepository();
  m_profile = repository ? repository->getProfile(m_id) : nullptr;
  m_name = m_profile ? QString::fromStdString(m_profile->getName()) : QString{};
  m_analog = m_profile ? m_profile->getDefaultAnalogSettings() : input::AnalogSettings{};
  emit profileIdChanged(m_id);
  emit nameChanged(name());
  emit analogChanged();
}

QString GamepadProfileItem::name() const { return m_name; }

void GamepadProfileItem::setName(const QString &name) {
  const auto repository = getControllerProfileRepository();
  if (!m_profile || !repository || name.isEmpty() || name == m_name) {
    return;
  }

  if (!repository->renameProfile(m_id, name.toStdString())) {
    return;
  }

  m_name = name;
  emit nameChanged(m_name);
  EventDispatcher::instance().publish(input::ProfileSettingsChangedEvent{m_id});
}

QString GamepadProfileItem::icon() const {
  if (!m_profile) {
    return {};
  }

  return QString::fromStdString(m_profile->getIcon());
}

bool GamepadProfileItem::isKeyboardProfile() const {
  if (!m_profile) {
    return false;
  }

  return m_profile->isKeyboardProfile();
}

// TODO
// Defines the getter and setter of one analog field
#define FL_PROFILE_ANALOG_PROPERTY(valueType, getterName, setterName, field)                                           \
  valueType GamepadProfileItem::getterName() const {                                                                   \
    return input::getAnalogField(m_analog, input::AnalogField::field).value<valueType>();                              \
  }                                                                                                                    \
  void GamepadProfileItem::setterName(const valueType value) {                                                         \
    auto updated = m_analog;                                                                                           \
    if (input::setAnalogField(updated, input::AnalogField::field, value)) {                                            \
      writeAnalog(updated);                                                                                            \
    }                                                                                                                  \
  }

FL_PROFILE_ANALOG_PROPERTY(double, leftStickInnerDeadzone, setLeftStickInnerDeadzone, LeftStickInnerDeadzone)
FL_PROFILE_ANALOG_PROPERTY(double, leftStickOuterDeadzone, setLeftStickOuterDeadzone, LeftStickOuterDeadzone)
FL_PROFILE_ANALOG_PROPERTY(double, leftStickSensitivity, setLeftStickSensitivity, LeftStickSensitivity)
FL_PROFILE_ANALOG_PROPERTY(double, leftStickAntiDeadzone, setLeftStickAntiDeadzone, LeftStickAntiDeadzone)
FL_PROFILE_ANALOG_PROPERTY(int, leftStickCurve, setLeftStickCurve, LeftStickCurve)
FL_PROFILE_ANALOG_PROPERTY(double, leftStickCurveExponent, setLeftStickCurveExponent, LeftStickCurveExponent)

FL_PROFILE_ANALOG_PROPERTY(double, rightStickInnerDeadzone, setRightStickInnerDeadzone, RightStickInnerDeadzone)
FL_PROFILE_ANALOG_PROPERTY(double, rightStickOuterDeadzone, setRightStickOuterDeadzone, RightStickOuterDeadzone)
FL_PROFILE_ANALOG_PROPERTY(double, rightStickSensitivity, setRightStickSensitivity, RightStickSensitivity)
FL_PROFILE_ANALOG_PROPERTY(double, rightStickAntiDeadzone, setRightStickAntiDeadzone, RightStickAntiDeadzone)
FL_PROFILE_ANALOG_PROPERTY(int, rightStickCurve, setRightStickCurve, RightStickCurve)
FL_PROFILE_ANALOG_PROPERTY(double, rightStickCurveExponent, setRightStickCurveExponent, RightStickCurveExponent)

FL_PROFILE_ANALOG_PROPERTY(double, leftTriggerThreshold, setLeftTriggerThreshold, LeftTriggerThreshold)
FL_PROFILE_ANALOG_PROPERTY(double, rightTriggerThreshold, setRightTriggerThreshold, RightTriggerThreshold)

#undef FL_PROFILE_ANALOG_PROPERTY

void GamepadProfileItem::resetAnalogToDefault() {
  const auto defaults = input::AnalogSettings{};
  if (m_analog == defaults) {
    return;
  }

  writeAnalog(defaults);
}

void GamepadProfileItem::writeAnalog(const input::AnalogSettings &updated) {
  const auto repository = getControllerProfileRepository();
  if (!m_profile || !repository) {
    return;
  }

  repository->setProfileAnalogSettings(m_id, updated);
  m_analog = updated;
  emit analogChanged();
  EventDispatcher::instance().publish(input::ProfileSettingsChangedEvent{m_id});
}

void GamepadProfileItem::reloadFromRepository() {
  const auto repository = getControllerProfileRepository();
  if (!repository) {
    return;
  }

  const auto profile = repository->getProfile(m_id);
  if (!profile) {
    if (!m_profile) {
      return;
    }

    m_profile = nullptr;
    m_name.clear();
    m_analog = input::AnalogSettings{};
    emit profileIdChanged(m_id);
    emit nameChanged(m_name);
    emit analogChanged();
    return;
  }
  m_profile = profile;

  const auto storedName = QString::fromStdString(m_profile->getName());
  if (storedName != m_name) {
    m_name = storedName;
    emit nameChanged(m_name);
  }

  const auto &stored = m_profile->getDefaultAnalogSettings();
  if (!(stored == m_analog)) {
    m_analog = stored;
    emit analogChanged();
  }
}

} // namespace firelight::gui

#include "firelight/event_dispatcher.hpp"

#include <firelight/input/gamepad_key_event.hpp>
#include <firelight/input/input_service.hpp>
#include <firelight/input/keyboard_input_handler.hpp>

#include <QKeyEvent>
#include <spdlog/spdlog.h>

namespace firelight::input {
KeyboardInputHandler::KeyboardInputHandler() = default;

const QMap<GamepadInput, Qt::Key> &KeyboardInputHandler::defaultKeyMap() {
  static const QMap<GamepadInput, Qt::Key> keys = {
      {GamepadInput::DpadUp, Qt::Key_Up},
      {GamepadInput::DpadDown, Qt::Key_Down},
      {GamepadInput::DpadLeft, Qt::Key_Left},
      {GamepadInput::DpadRight, Qt::Key_Right},
      {GamepadInput::LeftStickUp, Qt::Key_I},
      {GamepadInput::LeftStickDown, Qt::Key_K},
      {GamepadInput::LeftStickLeft, Qt::Key_J},
      {GamepadInput::LeftStickRight, Qt::Key_L},
      {GamepadInput::RightStickUp, Qt::Key_T},
      {GamepadInput::RightStickDown, Qt::Key_G},
      {GamepadInput::RightStickLeft, Qt::Key_F},
      {GamepadInput::RightStickRight, Qt::Key_H},
      {GamepadInput::L3, Qt::Key_M},
      {GamepadInput::R3, Qt::Key_B},
      {GamepadInput::WestFace, Qt::Key_A},
      {GamepadInput::SouthFace, Qt::Key_Z},
      {GamepadInput::EastFace, Qt::Key_X},
      {GamepadInput::NorthFace, Qt::Key_S},
      {GamepadInput::LeftBumper, Qt::Key_Q},
      {GamepadInput::RightBumper, Qt::Key_W},
      {GamepadInput::LeftTrigger, Qt::Key_E},
      {GamepadInput::RightTrigger, Qt::Key_R},
      {GamepadInput::Start, Qt::Key_Return},
      {GamepadInput::Select, Qt::Key_Shift},
  };
  return keys;
}

Qt::Key KeyboardInputHandler::getDefaultKey(const GamepadInput input) {
  return defaultKeyMap().value(input, Qt::Key_unknown);
}

QString KeyboardInputHandler::getKeyLabel(const Qt::Key key) {
  return QKeySequence(key).toString(QKeySequence::NativeText);
}

bool KeyboardInputHandler::isButtonPressed(int platformId, int controllerTypeId, Input t_button) {
  std::lock_guard lock(m_keyStatesMutex);
  const auto input = static_cast<GamepadInput>(t_button);
  if (m_profile) {
    auto mapping = m_profile->getMappingForPlatformAndController(platformId, controllerTypeId);
    if (mapping) {
      const auto &bindings = mapping->getBindings(input);
      if (!bindings.empty()) {
        // Multiple key bindings for one emulated input are OR'd together
        for (const auto &binding : bindings) {
          bool modifiersHeld = true;
          for (const auto mod : binding.source.modifiers) {
            if (!m_keyStates[static_cast<Qt::Key>(mod)]) {
              modifiersHeld = false;
              break;
            }
          }
          if (modifiersHeld && m_keyStates[static_cast<Qt::Key>(binding.source.code)]) {
            return true;
          }
        }
        return false;
      }
    }
  }

  const auto defaultKey = getDefaultKey(static_cast<GamepadInput>(t_button));
  if (defaultKey == Qt::Key_unknown) {
    return false;
  }

  // A shortcut is using this key, so the game doesn't see it
  if (isSuppressed(defaultKey)) {
    return false;
  }
  return m_keyStates[defaultKey];
}

int16_t KeyboardInputHandler::getLeftStickXPosition(int platformId, int controllerTypeId) {
  std::lock_guard lock(m_keyStatesMutex);
  if (!m_profile) {
    if (m_keyStates[Qt::Key_Left]) {
      return -32767;
    }
    if (m_keyStates[Qt::Key_Right]) {
      return 32767;
    }
  }

  auto mapping = m_profile->getMappingForPlatformAndController(platformId, controllerTypeId);
  if (!mapping) {
    if (m_keyStates[Qt::Key_Left]) {
      return -32767;
    }
    if (m_keyStates[Qt::Key_Right]) {
      return 32767;
    }
  }

  const auto mappedLeft = mapping->getMappedInput(GamepadInput::LeftStickLeft);
  const auto mappedRight = mapping->getMappedInput(GamepadInput::LeftStickRight);

  if (mappedLeft.has_value() && mappedRight.has_value()) {
    const auto valLeft = m_keyStates[static_cast<Qt::Key>(mappedLeft.value())];
    const auto valRight = m_keyStates[static_cast<Qt::Key>(mappedRight.value())];

    if (valLeft && valRight) {
      return 0;
    }

    if (valLeft) {
      return -32767;
    }

    if (valRight) {
      return 32767;
    }

    return 0;
  }

  if (mappedLeft.has_value() && m_keyStates[static_cast<Qt::Key>(mappedLeft.value())]) {
    return -32767;
  }

  if (mappedRight.has_value() && m_keyStates[static_cast<Qt::Key>(mappedRight.value())]) {
    return 32767;
  }

  if (m_keyStates[Qt::Key_Left]) {
    return -32767;
  }
  if (m_keyStates[Qt::Key_Right]) {
    return 32767;
  }

  return 0;
}

int16_t KeyboardInputHandler::getLeftStickYPosition(int platformId, int controllerTypeId) {
  std::lock_guard lock(m_keyStatesMutex);
  if (!m_profile) {
    if (m_keyStates[Qt::Key_Left]) {
      return -32767;
    }
    if (m_keyStates[Qt::Key_Right]) {
      return 32767;
    }
  }

  auto mapping = m_profile->getMappingForPlatformAndController(platformId, controllerTypeId);
  if (!mapping) {
    if (m_keyStates[Qt::Key_Left]) {
      return -32767;
    }
    if (m_keyStates[Qt::Key_Right]) {
      return 32767;
    }
  }

  const auto mappedUp = mapping->getMappedInput(GamepadInput::LeftStickUp);
  const auto mappedDown = mapping->getMappedInput(GamepadInput::LeftStickDown);

  if (mappedUp.has_value() && mappedDown.has_value()) {
    const auto valLeft = m_keyStates[static_cast<Qt::Key>(mappedUp.value())];
    const auto valRight = m_keyStates[static_cast<Qt::Key>(mappedDown.value())];

    if (valLeft && valRight) {
      return 0;
    }

    if (valLeft) {
      return -32767;
    }

    if (valRight) {
      return 32767;
    }

    return 0;
  }

  if (mappedUp.has_value() && m_keyStates[static_cast<Qt::Key>(mappedUp.value())]) {
    return -32767;
  }

  if (mappedDown.has_value() && m_keyStates[static_cast<Qt::Key>(mappedDown.value())]) {
    return 32767;
  }

  if (m_keyStates[Qt::Key_Up]) {
    return -32767;
  }
  if (m_keyStates[Qt::Key_Down]) {
    return 32767;
  }

  return 0;
}

int16_t KeyboardInputHandler::getRightStickXPosition(int platformId, int controllerTypeId) { return 0; }

int16_t KeyboardInputHandler::getRightStickYPosition(int platformId, int controllerTypeId) { return 0; }

void KeyboardInputHandler::setStrongRumble(int platformId, uint16_t t_strength) {}

void KeyboardInputHandler::setWeakRumble(int platformId, uint16_t t_strength) {}

std::string KeyboardInputHandler::getName() const { return "Keyboard"; }

int KeyboardInputHandler::getPlayerIndex() const { return m_playerIndex; }

void KeyboardInputHandler::setPlayerIndex(const int playerIndex) { m_playerIndex = playerIndex; }

bool KeyboardInputHandler::isWired() const { return true; }

GamepadType KeyboardInputHandler::getType() const { return KEYBOARD; }

DeviceType KeyboardInputHandler::getDeviceType() const { return DeviceType::Keyboard; }

int KeyboardInputHandler::getInstanceId() const { return -2; }

bool KeyboardInputHandler::disconnect() { return true; }

DeviceIdentifier KeyboardInputHandler::getDeviceIdentifier() const {
  return DeviceIdentifier{
      .deviceName = "Keyboard",
      .type = DeviceType::Keyboard,
      .vendorId = -1,
      .productId = -1,
      .productVersion = -1,
  };
}

bool KeyboardInputHandler::eventFilter(QObject *obj, QEvent *event) {
  if (event->type() == QEvent::KeyPress) {
    if (GamepadKeyEvent::isGamepad(event)) {
      return false;
    }

    const auto keyEvent = dynamic_cast<QKeyEvent *>(event);
    const auto key = static_cast<Qt::Key>(keyEvent->key());
    bool rising = false;
    {
      std::lock_guard lock(m_keyStatesMutex);
      // Ignore auto-repeat: only report the rising edge to the shortcut engine
      if (!m_keyStates.value(key, false)) {
        m_keyStates[key] = true;
        rising = true;
      }
    }
    if (rising) {
      EventDispatcher::instance().publish(
          KeyboardKeyEvent{.playerIndex = m_playerIndex, .key = keyEvent->key(), .pressed = true});
    }
  } else if (event->type() == QEvent::KeyRelease) {
    if (GamepadKeyEvent::isGamepad(event)) {
      return false;
    }

    const auto keyEvent = dynamic_cast<QKeyEvent *>(event);
    const auto key = static_cast<Qt::Key>(keyEvent->key());
    {
      std::lock_guard lock(m_keyStatesMutex);
      m_keyStates[key] = false;
    }
    EventDispatcher::instance().publish(
        KeyboardKeyEvent{.playerIndex = m_playerIndex, .key = keyEvent->key(), .pressed = false});
  }

  return QObject::eventFilter(obj, event);
}

std::shared_ptr<GamepadProfile> KeyboardInputHandler::getProfile() const { return m_profile; }

void KeyboardInputHandler::setProfile(const std::shared_ptr<GamepadProfile> &profile) { m_profile = profile; }

int16_t KeyboardInputHandler::evaluateRawInput(const GamepadInput input) const { return 0; }
} // namespace firelight::input

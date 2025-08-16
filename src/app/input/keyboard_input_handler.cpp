#include "keyboard_input_handler.hpp"

#include "event_dispatcher.hpp"
#include "input2/input_service.hpp"

#include <qevent.h>
#include <spdlog/spdlog.h>

namespace firelight::input {
QMap<GamepadInput, Qt::Key> KeyboardInputHandler::defaultKeys;

KeyboardInputHandler::KeyboardInputHandler() {
  defaultKeys[GamepadInput::DpadUp] = Qt::Key_Up;
  defaultKeys[GamepadInput::DpadDown] = Qt::Key_Down;
  defaultKeys[GamepadInput::DpadLeft] = Qt::Key_Left;
  defaultKeys[GamepadInput::DpadRight] = Qt::Key_Right;
  defaultKeys[GamepadInput::LeftStickUp] = Qt::Key_I;
  defaultKeys[GamepadInput::LeftStickDown] = Qt::Key_K;
  defaultKeys[GamepadInput::LeftStickLeft] = Qt::Key_J;
  defaultKeys[GamepadInput::LeftStickRight] = Qt::Key_L;
  defaultKeys[GamepadInput::RightStickUp] = Qt::Key_T;
  defaultKeys[GamepadInput::RightStickDown] = Qt::Key_G;
  defaultKeys[GamepadInput::RightStickLeft] = Qt::Key_F;
  defaultKeys[GamepadInput::RightStickRight] = Qt::Key_H;
  defaultKeys[GamepadInput::L3] = Qt::Key_M;
  defaultKeys[GamepadInput::R3] = Qt::Key_B;
  defaultKeys[GamepadInput::WestFace] = Qt::Key_A;
  defaultKeys[GamepadInput::SouthFace] = Qt::Key_Z;
  defaultKeys[GamepadInput::EastFace] = Qt::Key_X;
  defaultKeys[GamepadInput::NorthFace] = Qt::Key_S;
  defaultKeys[GamepadInput::LeftBumper] = Qt::Key_Q;
  defaultKeys[GamepadInput::RightBumper] = Qt::Key_W;
  defaultKeys[GamepadInput::LeftTrigger] = Qt::Key_E;
  defaultKeys[GamepadInput::RightTrigger] = Qt::Key_R;
  defaultKeys[GamepadInput::Start] = Qt::Key_Return;
  defaultKeys[GamepadInput::Select] = Qt::Key_Shift;
}

Qt::Key KeyboardInputHandler::getDefaultKey(const GamepadInput input) {
  return defaultKeys.contains(input) ? defaultKeys[input] : Qt::Key_unknown;
}
QString KeyboardInputHandler::getKeyLabel(const Qt::Key key) {
  return QKeySequence(key).toString(QKeySequence::NativeText);
}

bool KeyboardInputHandler::isButtonPressed(int platformId, int controllerTypeId,
                                           Input t_button) {
  const auto input = static_cast<GamepadInput>(t_button);
  if (m_profile) {
    auto mapping = m_profile->getMappingForPlatformAndController(
        platformId, controllerTypeId);
    if (mapping) {
      const auto mappedKey = mapping->getMappedInput(input);
      if (mappedKey.has_value()) {
        return m_keyStates[static_cast<Qt::Key>(mappedKey.value())];
      }
    }
  }

  if (!defaultKeys.contains(static_cast<GamepadInput>(t_button))) {
    return false;
  }

  return m_keyStates[defaultKeys[static_cast<GamepadInput>(t_button)]];
}

int16_t KeyboardInputHandler::getLeftStickXPosition(int platformId,
                                                    int controllerTypeId) {
  if (!m_profile) {
    if (m_keyStates[Qt::Key_Left]) {
      return -32767;
    }
    if (m_keyStates[Qt::Key_Right]) {
      return 32767;
    }
  }

  auto mapping = m_profile->getMappingForPlatformAndController(
      platformId, controllerTypeId);
  if (!mapping) {
    if (m_keyStates[Qt::Key_Left]) {
      return -32767;
    }
    if (m_keyStates[Qt::Key_Right]) {
      return 32767;
    }
  }

  const auto mappedLeft = mapping->getMappedInput(GamepadInput::LeftStickLeft);
  const auto mappedRight =
      mapping->getMappedInput(GamepadInput::LeftStickRight);

  if (mappedLeft.has_value() && mappedRight.has_value()) {
    const auto valLeft = m_keyStates[static_cast<Qt::Key>(mappedLeft.value())];
    const auto valRight =
        m_keyStates[static_cast<Qt::Key>(mappedRight.value())];

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

  if (mappedLeft.has_value() &&
      m_keyStates[static_cast<Qt::Key>(mappedLeft.value())]) {
    return -32767;
  }

  if (mappedRight.has_value() &&
      m_keyStates[static_cast<Qt::Key>(mappedRight.value())]) {
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

int16_t KeyboardInputHandler::getLeftStickYPosition(int platformId,
                                                    int controllerTypeId) {
  if (!m_profile) {
    if (m_keyStates[Qt::Key_Left]) {
      return -32767;
    }
    if (m_keyStates[Qt::Key_Right]) {
      return 32767;
    }
  }

  auto mapping = m_profile->getMappingForPlatformAndController(
      platformId, controllerTypeId);
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

  if (mappedUp.has_value() &&
      m_keyStates[static_cast<Qt::Key>(mappedUp.value())]) {
    return -32767;
  }

  if (mappedDown.has_value() &&
      m_keyStates[static_cast<Qt::Key>(mappedDown.value())]) {
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

int16_t KeyboardInputHandler::getRightStickXPosition(int platformId,
                                                     int controllerTypeId) {
  return 0;
}

int16_t KeyboardInputHandler::getRightStickYPosition(int platformId,
                                                     int controllerTypeId) {
  return 0;
}

void KeyboardInputHandler::setStrongRumble(int platformId,
                                           uint16_t t_strength) {
  // Intentionally do nothing.
}

void KeyboardInputHandler::setWeakRumble(int platformId, uint16_t t_strength) {
  // Intentionally do nothing.
}

std::string KeyboardInputHandler::getName() const { return "Keyboard"; }

int KeyboardInputHandler::getPlayerIndex() const { return m_playerIndex; }

void KeyboardInputHandler::setPlayerIndex(const int playerIndex) {
  m_playerIndex = playerIndex;
}

bool KeyboardInputHandler::isWired() const { return true; }

GamepadType KeyboardInputHandler::getType() const { return KEYBOARD; }

int KeyboardInputHandler::getInstanceId() const { return -2; }

bool KeyboardInputHandler::disconnect() {
  // Intentionally do nothing.
  return true;
}

DeviceIdentifier KeyboardInputHandler::getDeviceIdentifier() const {
  return DeviceIdentifier{
      .deviceName = "Keyboard",
      .vendorId = -1,
      .productId = -1,
      .productVersion = -1,
  };
}

bool KeyboardInputHandler::eventFilter(QObject *obj, QEvent *event) {
  if (event->type() == QEvent::KeyPress) {
    const auto keyEvent = dynamic_cast<QKeyEvent *>(event);
    if (keyEvent->modifiers() == Qt::KeyboardModifier::KeyboardModifierMask) {
      return false;
    }
    for (auto &thing : m_profile->getShortcutMapping()->getMappings()) {
      auto pressed = true;
      auto ctrlPressed = keyEvent->modifiers() & Qt::ControlModifier;
      auto altPressed = keyEvent->modifiers() & Qt::AltModifier;
      auto shiftPressed = keyEvent->modifiers() & Qt::ShiftModifier;

      for (auto mod : thing.second.modifiers) {
        if (mod == Qt::Key_Control && !ctrlPressed) {
          pressed = false;
          break;
        }

        if (mod == Qt::Key_Alt && !altPressed) {
          pressed = false;
          break;
        }

        if (mod == Qt::Key_Shift && !shiftPressed) {
          pressed = false;
          break;
        }
      }

      if (!pressed) {
        continue;
      }

      if (thing.second.input == keyEvent->key()) {
        spdlog::info("Shortcut {} triggered", static_cast<int>(thing.first));
        EventDispatcher::instance().publish(
            ShortcutToggledEvent{.shortcut = thing.first});
        return true;
      }
    }
    m_keyStates[static_cast<Qt::Key>(keyEvent->key())] = true;
  } else if (event->type() == QEvent::KeyRelease) {
    const auto keyEvent = dynamic_cast<QKeyEvent *>(event);
    if (keyEvent->modifiers() == Qt::KeyboardModifier::KeyboardModifierMask) {
      return false;
    }
    m_keyStates[static_cast<Qt::Key>(keyEvent->key())] = false;
  }

  return QObject::eventFilter(obj, event);
}
std::shared_ptr<GamepadProfile> KeyboardInputHandler::getProfile() const {
  return m_profile;
}
void KeyboardInputHandler::setProfile(
    const std::shared_ptr<GamepadProfile> &profile) {
  m_profile = profile;
}

std::vector<Shortcut>
KeyboardInputHandler::getToggledShortcuts(GamepadInput input) {
  return {};
}

int16_t KeyboardInputHandler::evaluateRawInput(const GamepadInput input) const {
  return 0;
}
} // namespace firelight::input

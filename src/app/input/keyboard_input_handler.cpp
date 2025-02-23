#include "keyboard_input_handler.hpp"

#include <qevent.h>
#include <spdlog/spdlog.h>

namespace firelight::input {
  KeyboardInputHandler::KeyboardInputHandler() = default;
void KeyboardInputHandler::setActiveMapping(
    const std::shared_ptr<InputMapping> &mapping) {
  m_keyboardMapping = std::static_pointer_cast<KeyboardMapping>(mapping);
  SdlController::setActiveMapping(mapping);
}

bool KeyboardInputHandler::isButtonPressed(int platformId, Input t_button) {
    if (m_keyboardMapping) {
      const auto mappedKey = m_keyboardMapping->getMappedKeyboardInput(t_button);
      if (mappedKey.has_value()) {
        return m_keyStates[static_cast<Qt::Key>(mappedKey.value())];
      }
    }

    switch (t_button) {
      case RightBumper: return m_keyStates[Qt::Key_W];
      case LeftBumper: return m_keyStates[Qt::Key_Q];
      case LeftTrigger: return m_keyStates[Qt::Key_E];
      case RightTrigger: return m_keyStates[Qt::Key_R];
      case NorthFace: return m_keyStates[Qt::Key_S];
      case EastFace: return m_keyStates[Qt::Key_X];
      case WestFace: return m_keyStates[Qt::Key_A];
      case SouthFace: return m_keyStates[Qt::Key_Z];
      case DpadUp: return m_keyStates[Qt::Key_I];
      case DpadDown: return m_keyStates[Qt::Key_K];
      case DpadLeft: return m_keyStates[Qt::Key_J];
      case DpadRight: return m_keyStates[Qt::Key_L];
      case Start: return m_keyStates[Qt::Key_Return];
      case Select: return m_keyStates[Qt::Key_Shift];
      default: return false;
    }
  }

  int16_t KeyboardInputHandler::getLeftStickXPosition(int platformId) {
    if (!m_keyboardMapping) {
      if (m_keyStates[Qt::Key_Left]) {
        return -32767;
      }
      if (m_keyStates[Qt::Key_Right]) {
        return 32767;
      }
    }

    const auto mappedLeft = m_keyboardMapping->getMappedKeyboardInput(LeftStickLeft);
    const auto mappedRight = m_keyboardMapping->getMappedKeyboardInput(LeftStickRight);

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

  int16_t KeyboardInputHandler::getLeftStickYPosition(int platformId) {
    if (!m_keyboardMapping) {
      if (m_keyStates[Qt::Key_Up]) {
        return -32767;
      }
      if (m_keyStates[Qt::Key_Down]) {
        return 32767;
      }
    }

    const auto mappedUp = m_keyboardMapping->getMappedKeyboardInput(LeftStickUp);
    const auto mappedDown = m_keyboardMapping->getMappedKeyboardInput(LeftStickDown);

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

  int16_t KeyboardInputHandler::getRightStickXPosition(int platformId) {
    return 0;
  }

  int16_t KeyboardInputHandler::getRightStickYPosition(int platformId) {
    return 0;
  }

  void KeyboardInputHandler::setStrongRumble(int platformId, uint16_t t_strength) {
    // Intentionally do nothing.
  }

  void KeyboardInputHandler::setWeakRumble(int platformId, uint16_t t_strength) {
    // Intentionally do nothing.
  }

  std::string KeyboardInputHandler::getName() const {
    return "Keyboard";
  }

  int KeyboardInputHandler::getPlayerIndex() const {
    return m_playerIndex;
  }

  void KeyboardInputHandler::setPlayerIndex(int playerIndex) {
    m_playerIndex = playerIndex;
  }

  bool KeyboardInputHandler::isWired() const {
    return true;
  }

  GamepadType KeyboardInputHandler::getType() const {
    return KEYBOARD;
  }

  bool KeyboardInputHandler::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::KeyPress) {
      const auto keyEvent = dynamic_cast<QKeyEvent *>(event);
      m_keyStates[static_cast<Qt::Key>(keyEvent->key())] = true;
    } else if (event->type() == QEvent::KeyRelease) {
      const auto keyEvent = dynamic_cast<QKeyEvent *>(event);
      m_keyStates[static_cast<Qt::Key>(keyEvent->key())] = false;
    }

    return QObject::eventFilter(obj, event);
  }
} // namespace firelight::gu

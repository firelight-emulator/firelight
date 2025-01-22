#include "keyboard_input_handler.hpp"

#include <qevent.h>
#include <spdlog/spdlog.h>

namespace firelight::input {
  KeyboardInputHandler::KeyboardInputHandler() = default;

  bool KeyboardInputHandler::isButtonPressed(int platformId, Input t_button) {
    if (m_activeMapping) {
      printf("Getting active mapping\n");

      // const auto mapping = m_activeMapping.get();
      // const auto actualMapping = dynamic_cast<KeyboardMapping *>(mapping);
      //
      // const auto mappedKey = actualMapping->getMappedKeyboardInput(t_button);
      // if (mappedKey.has_value()) {
      //   return m_keyStates[static_cast<Qt::Key>(mappedKey.value())];
      // }
    }

    switch (t_button) {
      case RightBumper: return m_keyStates[Qt::Key_W];
      case LeftBumper: return m_keyStates[Qt::Key_Q];
      case NorthFace: return m_keyStates[Qt::Key_S];
      case EastFace: return m_keyStates[Qt::Key_X];
      case WestFace: return m_keyStates[Qt::Key_A];
      case SouthFace: return m_keyStates[Qt::Key_Z];
      case DpadUp: return m_keyStates[Qt::Key_Up];
      case DpadDown: return m_keyStates[Qt::Key_Down];
      case DpadLeft: return m_keyStates[Qt::Key_Left];
      case DpadRight: return m_keyStates[Qt::Key_Right];
      case Start: return m_keyStates[Qt::Key_Return];
      case Select: return m_keyStates[Qt::Key_Shift];
      default: return false;
    }
  }

  int16_t KeyboardInputHandler::getLeftStickXPosition(int platformId) {
    return 0;
  }

  int16_t KeyboardInputHandler::getLeftStickYPosition(int platformId) {
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

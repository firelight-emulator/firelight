#include "keyboard_input_handler.hpp"

#include <qevent.h>
#include <spdlog/spdlog.h>

namespace firelight::input {
  bool KeyboardInputHandler::isButtonPressed(Button t_button) {
    return m_buttonStates[t_button];
  }

  int16_t KeyboardInputHandler::getLeftStickXPosition() {
    return 0;
  }

  int16_t KeyboardInputHandler::getLeftStickYPosition() {
    return 0;
  }

  int16_t KeyboardInputHandler::getRightStickXPosition() {
    return 0;
  }

  int16_t KeyboardInputHandler::getRightStickYPosition() {
    return 0;
  }

  void KeyboardInputHandler::setStrongRumble(uint16_t t_strength) {
  }

  void KeyboardInputHandler::setWeakRumble(uint16_t t_strength) {
  }

  QPointF KeyboardInputHandler::getMousePosition() const {
    return m_mousePosition;
  }

  bool KeyboardInputHandler::isLeftMouseButtonDown() const {
    return m_leftMouseButtonDown;
  }

  bool KeyboardInputHandler::isRightMouseButtonDown() const {
    return m_rightMouseButtonDown;
  }

  bool KeyboardInputHandler::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::MouseMove) {
      auto mouseEvent = dynamic_cast<QMouseEvent *>(event);
      m_mousePosition = mouseEvent->position();
    } else if (event->type() == QEvent::MouseButtonPress) {
      auto mouseEvent = dynamic_cast<QMouseEvent *>(event);
      if (mouseEvent->button() == Qt::LeftButton) {
        m_leftMouseButtonDown = true;
      } else if (mouseEvent->button() == Qt::RightButton) {
        m_rightMouseButtonDown = true;
      }
    } else if (event->type() == QEvent::MouseButtonRelease) {
      auto mouseEvent = dynamic_cast<QMouseEvent *>(event);
      if (mouseEvent->button() == Qt::LeftButton) {
        m_leftMouseButtonDown = false;
      } else if (mouseEvent->button() == Qt::RightButton) {
        m_rightMouseButtonDown = false;
      }
    } else if (event->type() == QEvent::KeyPress) {
      auto keyEvent = dynamic_cast<QKeyEvent *>(event);
      switch (keyEvent->key()) {
        case Qt::Key_W:
          m_buttonStates[RightBumper] = true;
          break;
        case Qt::Key_Q:
          m_buttonStates[LeftBumper] = true;
          break;
        case Qt::Key_S:
          m_buttonStates[NorthFace] = true;
          break;
        case Qt::Key_A:
          m_buttonStates[WestFace] = true;
          break;
        case Qt::Key_X:
          m_buttonStates[EastFace] = true;
          break;
        case Qt::Key_Z:
          m_buttonStates[SouthFace] = true;
          break;
        case Qt::Key_Up:
          m_buttonStates[DpadUp] = true;
          break;
        case Qt::Key_Down:
          m_buttonStates[DpadDown] = true;
          break;
        case Qt::Key_Left:
          m_buttonStates[DpadLeft] = true;
          break;
        case Qt::Key_Right:
          m_buttonStates[DpadRight] = true;
          break;
        case Qt::Key_Return:
        case Qt::Key_Enter:
          m_buttonStates[Start] = true;
          break;
        case Qt::Key_Shift:
          m_buttonStates[Select] = true;
          break;
      }
    } else if (event->type() == QEvent::KeyRelease) {
      auto keyEvent = dynamic_cast<QKeyEvent *>(event);
      switch (keyEvent->key()) {
        case Qt::Key_W:
          m_buttonStates[RightBumper] = false;
          break;
        case Qt::Key_Q:
          m_buttonStates[LeftBumper] = false;
          break;
        case Qt::Key_S:
          m_buttonStates[NorthFace] = false;
          break;
        case Qt::Key_A:
          m_buttonStates[WestFace] = false;
          break;
        case Qt::Key_X:
          m_buttonStates[EastFace] = false;
          break;
        case Qt::Key_Z:
          m_buttonStates[SouthFace] = false;
          break;
        case Qt::Key_Up:
          m_buttonStates[DpadUp] = false;
          break;
        case Qt::Key_Down:
          m_buttonStates[DpadDown] = false;
          break;
        case Qt::Key_Left:
          m_buttonStates[DpadLeft] = false;
          break;
        case Qt::Key_Right:
          m_buttonStates[DpadRight] = false;
          break;
        case Qt::Key_Return:
        case Qt::Key_Enter:
          m_buttonStates[Start] = false;
          break;
        case Qt::Key_Shift:
          m_buttonStates[Select] = false;
          break;
      }
    }

    return QObject::eventFilter(obj, event);
  }
} // namespace firelight::gu

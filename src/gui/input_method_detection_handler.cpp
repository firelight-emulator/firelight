#include "input_method_detection_handler.hpp"

namespace firelight::gui {
  bool InputMethodDetectionHandler::usingMouse() const {
    return m_mouseUsedLast;
  }

  bool InputMethodDetectionHandler::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::MouseMove || event->type() == QEvent::TouchUpdate) {
      if (!m_mouseUsedLast) {
        m_mouseUsedLast = true;
        emit inputMethodChanged();
      }
    } else if (event->type() == QEvent::KeyPress) {
      if (m_mouseUsedLast) {
        m_mouseUsedLast = false;
        emit inputMethodChanged();
      }
    }

    return QObject::eventFilter(obj, event);
  }
} // firelight::gui

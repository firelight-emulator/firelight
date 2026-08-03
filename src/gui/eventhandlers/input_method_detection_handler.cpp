#include "input_method_detection_handler.hpp"

#include <QKeyEvent>

namespace firelight::gui {
bool InputMethodDetectionHandler::usingMouse() const { return m_mouseUsedLast; }

bool InputMethodDetectionHandler::isKeyRepeating() const { return m_keyRepeating; }

void InputMethodDetectionHandler::setKeyRepeating(const bool repeating) {
  if (m_keyRepeating == repeating) {
    return;
  }

  m_keyRepeating = repeating;
  emit keyRepeatingChanged();
}

bool InputMethodDetectionHandler::eventFilter(QObject *obj, QEvent *event) {
  if (event->type() == QEvent::MouseMove || event->type() == QEvent::TouchUpdate) {
    if (!m_mouseUsedLast) {
      m_mouseUsedLast = true;
      emit inputMethodChanged();
    }
  } else if (event->type() == QEvent::KeyPress) {
    setKeyRepeating(static_cast<QKeyEvent *>(event)->isAutoRepeat());

    if (m_mouseUsedLast) {
      m_mouseUsedLast = false;
      emit inputMethodChanged();
    }
  } else if (event->type() == QEvent::KeyRelease) {
    // A repeat delivers its own releases, so only the one ending the hold counts
    if (!static_cast<QKeyEvent *>(event)->isAutoRepeat()) {
      setKeyRepeating(false);
    }
  }

  return QObject::eventFilter(obj, event);
}
} // namespace firelight::gui

#include "window_resize_handler.hpp"

namespace firelight::gui {

WindowResizeHandler::WindowResizeHandler() {
  resizeTimer = new QTimer(this);
  resizeTimer->setSingleShot(true);
  resizeTimer->setInterval(100);
  connect(resizeTimer, &QTimer::timeout, this,
          [this]() { emit windowResizeFinished(); });
}

bool WindowResizeHandler::eventFilter(QObject *obj, QEvent *event) {
  if (event->type() == QEvent::Resize) {
    if (!resizeTimer->isActive()) {
      emit windowResizeStarted();
    }
    resizeTimer->start();
  }

  return QObject::eventFilter(obj, event);
}
} // namespace firelight::gui
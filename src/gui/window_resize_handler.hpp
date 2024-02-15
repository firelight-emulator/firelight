//
// Created by alexs on 2/15/2024.
//

#ifndef WINDOW_RESIZE_HANDLER_HPP
#define WINDOW_RESIZE_HANDLER_HPP
#include <QTimer>
#include <qevent.h>
#include <qtmetamacros.h>

namespace Firelight {

class WindowResizeHandler : public QObject {
  Q_OBJECT

  QTimer *resizeTimer;

signals:
  void windowResizeStarted();
  void windowResizeFinished();

public:
  WindowResizeHandler() {
    resizeTimer = new QTimer(this);
    resizeTimer->setSingleShot(true);
    resizeTimer->setInterval(300);
    connect(resizeTimer, &QTimer::timeout, this,
            [this]() { emit windowResizeFinished(); });
  }

protected:
  bool eventFilter(QObject *obj, QEvent *event) override;
};

inline bool WindowResizeHandler::eventFilter(QObject *obj, QEvent *event) {
  if (event->type() == QEvent::Resize) {
    if (!resizeTimer->isActive()) {
      emit windowResizeStarted();
    }
    resizeTimer->start();
  }

  return QObject::eventFilter(obj, event);
}

} // namespace Firelight

#endif // WINDOW_RESIZE_HANDLER_HPP

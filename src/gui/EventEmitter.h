#pragma once
#include <QApplication>
#include <QObject>
#include <QWindow>
#include <qevent.h>
namespace firelight::gui {

class EventEmitter : public QObject {
  Q_OBJECT
public:
  Q_INVOKABLE void emitKeyEvent(int key) {
    QMetaObject::invokeMethod(QApplication::instance(), [key]() {
                QApplication::postEvent(QGuiApplication::focusWindow(),
                  new QKeyEvent(QEvent::KeyPress, key, Qt::KeyboardModifier::NoModifier));
                QApplication::postEvent(QGuiApplication::focusWindow(),
                  new QKeyEvent(QEvent::KeyRelease, key, Qt::KeyboardModifier::NoModifier));
              }, Qt::QueuedConnection);
  }
};

} // namespace firelight::gui
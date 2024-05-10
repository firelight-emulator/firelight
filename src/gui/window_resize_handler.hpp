#pragma once

#include <QEvent>
#include <QTimer>

namespace firelight::gui {

/**
 * @class WindowResizeHandler
 * @brief Emits signals when the window is resized.
 *
 * This class emits signals when the window starts resizing and when it finishes
 * (after a short delay). It is intended to be used as an event filter for the
 * main window.
 */
class WindowResizeHandler final : public QObject {
  Q_OBJECT

  QTimer *resizeTimer;

signals:
  void windowResizeStarted();
  void windowResizeFinished();

public:
  WindowResizeHandler();

protected:
  bool eventFilter(QObject *obj, QEvent *event) override;
};

} // namespace firelight::gui

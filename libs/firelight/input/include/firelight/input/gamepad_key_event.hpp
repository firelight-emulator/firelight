// TODO: NEEDS REVIEW
#pragma once

#include <QEvent>
#include <QKeyEvent>

namespace firelight::input {

/**
 * A key event standing in for a controller button.
 *
 * Gamepad input reaches the interface as key events, so the focus layer answers a controller and a
 * keyboard through one path. The event type is what marks one as synthesized: the handler that reads
 * real keyboard input has to let these through untouched, and a modifier bit used as the mark would
 * be indistinguishable from the user holding that modifier.
 */
class GamepadKeyEvent : public QKeyEvent {
public:
  /**
   * Creates a press or release of key, holding no modifiers
   */
  GamepadKeyEvent(const QEvent::Type type, const int key, const bool isAutoRepeat)
      : QKeyEvent(type, key, Qt::NoModifier, QString(), isAutoRepeat) {}

  /**
   * @return Whether event was synthesized from a controller button
   */
  static bool isGamepad(const QEvent *event) { return dynamic_cast<const GamepadKeyEvent *>(event) != nullptr; }
};

} // namespace firelight::input

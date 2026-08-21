#pragma once

#include <QEvent>
#include <QObject>

namespace firelight::gui {
class InputMethodDetectionHandler final : public QObject {
  Q_OBJECT
  Q_PROPERTY(bool usingMouse READ usingMouse NOTIFY inputMethodChanged)
  Q_PROPERTY(bool keyRepeating READ isKeyRepeating NOTIFY keyRepeatingChanged)

public:
  InputMethodDetectionHandler() = default;

  bool usingMouse() const;

  /**
   * Whether the key press being delivered right now is a hold repeating rather than a fresh press.
   *
   * Filtering the window sees every key before the focused item does, so anything reacting to a
   * press — including code with no key event of its own, like the focus ring reacting to focus
   * moving — can tell a held direction from a deliberate one.
   */
  [[nodiscard]] bool isKeyRepeating() const;

signals:
  void inputMethodChanged();

  void keyRepeatingChanged();

protected:
  bool eventFilter(QObject *obj, QEvent *event) override;

private:
  void setKeyRepeating(bool repeating);

  bool m_mouseUsedLast = false;
  bool m_keyRepeating = false;
};
} // namespace firelight::gui

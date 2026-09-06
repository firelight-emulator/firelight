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

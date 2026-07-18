#pragma once

#include <QEvent>
#include <QObject>

namespace firelight::gui {
class InputMethodDetectionHandler final : public QObject {
  Q_OBJECT
  Q_PROPERTY(bool usingMouse READ usingMouse NOTIFY inputMethodChanged)

public:
  InputMethodDetectionHandler() = default;

  bool usingMouse() const;

signals:
  void inputMethodChanged();

protected:
  bool eventFilter(QObject *obj, QEvent *event) override;

private:
  bool m_mouseUsedLast = false;
};
} // namespace firelight::gui

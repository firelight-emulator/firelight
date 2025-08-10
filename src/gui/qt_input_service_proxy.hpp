#pragma once
#include "event_dispatcher.hpp"

#include <QObject>

namespace firelight::gui {

class QtInputServiceProxy final : public QObject {
  Q_OBJECT
public:
  QtInputServiceProxy();
signals:
  void shortcutToggled(int playerIndex, int shortcut);

private:
  ScopedConnection shortcutToggledConnection;
  ScopedConnection gamepadInputConnection;
};

} // namespace firelight::gui
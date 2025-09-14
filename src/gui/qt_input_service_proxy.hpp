#pragma once
#include "event_dispatcher.hpp"
#include "input2/input_service.hpp"
#include "service_accessor.hpp"

#include <QObject>
#include <qsettings.h>

namespace firelight::gui {

class QtInputServiceProxy final : public QObject, public ServiceAccessor {
  Q_OBJECT
  Q_PROPERTY(bool prioritizeControllerOverKeyboard READ
                 prioritizeControllerOverKeyboard WRITE
                     setPrioritizeControllerOverKeyboard NOTIFY
                         prioritizeControllerOverKeyboardChanged)
  Q_PROPERTY(
      bool onlyPlayerOneCanNavigateMenus READ getOnlyPlayerOneCanNavigateMenus
          WRITE setOnlyPlayerOneCanNavigateMenus NOTIFY
              onlyPlayerOneCanNavigateMenusChanged)
public:
  QtInputServiceProxy();

  void
  setPrioritizeControllerOverKeyboard(bool prioritizeControllerOverKeyboard);

  [[nodiscard]] bool prioritizeControllerOverKeyboard() const;

  void setOnlyPlayerOneCanNavigateMenus(bool onlyPlayerOneCanNavigateMenus);

  [[nodiscard]] bool getOnlyPlayerOneCanNavigateMenus() const;
signals:
  void shortcutToggled(int playerIndex, int shortcut);
  void prioritizeControllerOverKeyboardChanged();
  void onlyPlayerOneCanNavigateMenusChanged();

private:
  input::InputService *m_inputService;

  QSettings m_settings;

  bool m_onlyPlayerOneCanNavigateMenus = true;

  ScopedConnection shortcutToggledConnection;
  ScopedConnection gamepadInputConnection;
};

} // namespace firelight::gui
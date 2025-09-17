#pragma once
#include "event_dispatcher.hpp"
#include "input2/input_service.hpp"
#include "service_accessor.hpp"

#include <QObject>
#include <QTimer>
#include <chrono>
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

  // Auto-repeat functionality
  struct AutoRepeatState {
    std::chrono::steady_clock::time_point pressTime;
    std::chrono::steady_clock::time_point lastRepeatTime;
    bool isRepeating = false;
    int playerIndex;
    input::GamepadInput input;
  };
  std::map<std::pair<int, input::GamepadInput>, AutoRepeatState> m_autoRepeatStates;

  QTimer *m_autoRepeatTimer;

  // Auto-repeat configuration (in milliseconds)
  static constexpr std::chrono::milliseconds AUTO_REPEAT_INITIAL_DELAY{500};
  static constexpr std::chrono::milliseconds AUTO_REPEAT_INTERVAL{30};

  // Auto-repeat helper methods
  void startAutoRepeat(int playerIndex, input::GamepadInput input);
  void stopAutoRepeat(int playerIndex, input::GamepadInput input);
  void processAutoRepeat();
};

} // namespace firelight::gui
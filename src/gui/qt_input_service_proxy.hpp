#pragma once
#include "firelight/event_dispatcher.hpp"

#include <firelight/input/input_service.hpp>
#include <firelight/settings/setting_watcher.hpp>
#include <firelight/settings/settings_service.hpp>

#include <QObject>
#include <QTimer>
#include <QVariant>
#include <atomic>
#include <chrono>

namespace firelight::gui {

class QtInputServiceProxy final : public QObject {
  Q_OBJECT
  Q_PROPERTY(bool onlyPlayerOneCanNavigateMenus READ getOnlyPlayerOneCanNavigateMenus NOTIFY
                 onlyPlayerOneCanNavigateMenusChanged)
  Q_PROPERTY(QVariantMap currentGamepadButtonIcons READ getCurrentGamepadButtonIcons NOTIFY currentGamepadTypeChanged)
  Q_PROPERTY(int currentGamepadType READ getCurrentGamepadType NOTIFY currentGamepadTypeChanged)
public:
  QtInputServiceProxy(input::InputService &inputService, settings::SettingsService &settingsService);

  [[nodiscard]] bool getOnlyPlayerOneCanNavigateMenus() const;

  // TODO
  /**
   * Whether gamepad input from the given player slot is forwarded to the window as navigation keys. Player 1 and
   * input with no slot behind it always are
   */
  [[nodiscard]] bool isAllowedToNavigateMenus(int playerIndex) const;

  QVariantMap getCurrentGamepadButtonIcons() const;

  [[nodiscard]] int getCurrentGamepadType() const;

  // Switches shortcut scope between in-game and menu contexts (driven by the UI)
  Q_INVOKABLE void setShortcutsInGame(bool inGame);

  // Whether the Steam client is running. Steam reads gamepads globally through
  // its own filter driver, and its "Guide button focuses Steam" option fires
  // even for games it didn't launch — nothing in this process can pre-empt it,
  // so the Controllers page warns instead. Asked on demand rather than exposed
  // as a property: Steam can come and go, and a cached answer would be a lie
  [[nodiscard]] Q_INVOKABLE bool isSteamRunning() const;
signals:
  void onlyPlayerOneCanNavigateMenusChanged();
  void currentGamepadTypeChanged();

protected:
  bool eventFilter(QObject *obj, QEvent *event) override;

private:
  void markGamepadTypeChanged();

  input::InputService *m_inputService;

  std::atomic<bool> m_onlyPlayerOneCanNavigateMenus{false};

  settings::SettingWatcher m_onlyPlayerOneCanNavigateMenusWatcher;

  ScopedConnection gamepadInputConnection;

  // Auto-repeat functionality
  struct AutoRepeatState {
    std::chrono::steady_clock::time_point pressTime;
    std::chrono::steady_clock::time_point lastRepeatTime;
    bool isRepeating = false;
    int playerIndex;
    input::GamepadInput input;
  };

  GamepadType m_currentGamepadType = KEYBOARD;
  QVariantMap m_currentGamepadButtonIcons;

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

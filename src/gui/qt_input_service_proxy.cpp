// TODO: NEEDS REVIEW
//
// Created by alexs on 8/9/2025
//

#include "qt_input_service_proxy.hpp"

#include "EventEmitter.h"

#include <firelight/input/gamepad_key_event.hpp>
#include <firelight/input/input_service.hpp>
#include <firelight/input/shortcut_action.hpp>
#include <firelight/platforms/controller_type.hpp>

#include <QApplication>
#include <spdlog/spdlog.h>

#ifdef _WIN32
// clang-format off
#include <windows.h>
#include <tlhelp32.h>
// clang-format on
#endif

namespace firelight::gui {

namespace {
// Looks for the Steam client by process name. Deliberately not the registry's
// ActiveProcess/pid: that keeps a stale pid after a crash, and this warning is
// only worth showing when Steam is actually there to interfere
bool steamClientRunning() {
#ifdef _WIN32
  const auto snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (snapshot == INVALID_HANDLE_VALUE) {
    return false;
  }

  PROCESSENTRY32W entry{};
  entry.dwSize = sizeof(entry);
  bool found = false;
  if (Process32FirstW(snapshot, &entry)) {
    do {
      if (_wcsicmp(entry.szExeFile, L"steam.exe") == 0) {
        found = true;
        break;
      }
    } while (Process32NextW(snapshot, &entry));
  }
  CloseHandle(snapshot);
  return found;
#else
  // Only Windows has the filter driver this warning is about
  return false;
#endif
}
} // namespace

static QMap<input::GamepadInput, Qt::Key> gamepadToQtKeyMap = {
    {input::DpadRight, Qt::Key_Right},  {input::DpadLeft, Qt::Key_Left},         {input::DpadUp, Qt::Key_Up},
    {input::DpadDown, Qt::Key_Down},    {input::LeftStickRight, Qt::Key_Right},  {input::LeftStickLeft, Qt::Key_Left},
    {input::LeftStickUp, Qt::Key_Up},   {input::LeftStickDown, Qt::Key_Down},    {input::WestFace, Qt::Key_Menu},
    {input::SouthFace, Qt::Key_Enter},  {input::EastFace, Qt::Key_Back},         {input::LeftBumper, Qt::Key_Minus},
    {input::RightBumper, Qt::Key_Plus}, {input::RightTrigger, Qt::Key_PageDown}, {input::LeftTrigger, Qt::Key_PageUp},
    {input::Start, Qt::Key_Menu},       {input::Select, Qt::Key_Home},           {input::Home, Qt::Key_Home}};

static QMap<GamepadType, QMap<int, QString>> gamepadButtonIcons = {
    {KEYBOARD,
     {{Qt::Key_A, "qrc:/images/gamepad-buttons/keyboard/a"},
      {Qt::Key_B, "qrc:/images/gamepad-buttons/keyboard/b"},
      {Qt::Key_C, "qrc:/images/gamepad-buttons/keyboard/c"},
      {Qt::Key_D, "qrc:/images/gamepad-buttons/keyboard/d"},
      {Qt::Key_E, "qrc:/images/gamepad-buttons/keyboard/e"},
      {Qt::Key_F, "qrc:/images/gamepad-buttons/keyboard/f"},
      {Qt::Key_G, "qrc:/images/gamepad-buttons/keyboard/g"},
      {Qt::Key_H, "qrc:/images/gamepad-buttons/keyboard/h"},
      {Qt::Key_I, "qrc:/images/gamepad-buttons/keyboard/i"},
      {Qt::Key_J, "qrc:/images/gamepad-buttons/keyboard/j"},
      {Qt::Key_K, "qrc:/images/gamepad-buttons/keyboard/k"},
      {Qt::Key_L, "qrc:/images/gamepad-buttons/keyboard/l"},
      {Qt::Key_M, "qrc:/images/gamepad-buttons/keyboard/m"},
      {Qt::Key_N, "qrc:/images/gamepad-buttons/keyboard/n"},
      {Qt::Key_O, "qrc:/images/gamepad-buttons/keyboard/o"},
      {Qt::Key_P, "qrc:/images/gamepad-buttons/keyboard/p"},
      {Qt::Key_Q, "qrc:/images/gamepad-buttons/keyboard/q"},
      {Qt::Key_R, "qrc:/images/gamepad-buttons/keyboard/r"},
      {Qt::Key_S, "qrc:/images/gamepad-buttons/keyboard/s"},
      {Qt::Key_T, "qrc:/images/gamepad-buttons/keyboard/t"},
      {Qt::Key_U, "qrc:/images/gamepad-buttons/keyboard/u"},
      {Qt::Key_V, "qrc:/images/gamepad-buttons/keyboard/v"},
      {Qt::Key_W, "qrc:/images/gamepad-buttons/keyboard/w"},
      {Qt::Key_X, "qrc:/images/gamepad-buttons/keyboard/x"},
      {Qt::Key_Y, "qrc:/images/gamepad-buttons/keyboard/y"},
      {Qt::Key_Z, "qrc:/images/gamepad-buttons/keyboard/z"},
      {Qt::Key_0, "qrc:/images/gamepad-buttons/keyboard/0"},
      {Qt::Key_1, "qrc:/images/gamepad-buttons/keyboard/1"},
      {Qt::Key_2, "qrc:/images/gamepad-buttons/keyboard/2"},
      {Qt::Key_3, "qrc:/images/gamepad-buttons/keyboard/3"},
      {Qt::Key_4, "qrc:/images/gamepad-buttons/keyboard/4"},
      {Qt::Key_5, "qrc:/images/gamepad-buttons/keyboard/5"},
      {Qt::Key_6, "qrc:/images/gamepad-buttons/keyboard/6"},
      {Qt::Key_7, "qrc:/images/gamepad-buttons/keyboard/7"},
      {Qt::Key_8, "qrc:/images/gamepad-buttons/keyboard/8"},
      {Qt::Key_9, "qrc:/images/gamepad-buttons/keyboard/9"},
      {Qt::Key_F1, "qrc:/images/gamepad-buttons/keyboard/f1"},
      {Qt::Key_F2, "qrc:/images/gamepad-buttons/keyboard/f2"},
      {Qt::Key_F3, "qrc:/images/gamepad-buttons/keyboard/f3"},
      {Qt::Key_F4, "qrc:/images/gamepad-buttons/keyboard/f4"},
      {Qt::Key_F5, "qrc:/images/gamepad-buttons/keyboard/f5"},
      {Qt::Key_F6, "qrc:/images/gamepad-buttons/keyboard/f6"},
      {Qt::Key_F7, "qrc:/images/gamepad-buttons/keyboard/f7"},
      {Qt::Key_F8, "qrc:/images/gamepad-buttons/keyboard/f8"},
      {Qt::Key_F9, "qrc:/images/gamepad-buttons/keyboard/f9"},
      {Qt::Key_F10, "qrc:/images/gamepad-buttons/keyboard/f10"},
      {Qt::Key_F11, "qrc:/images/gamepad-buttons/keyboard/f11"},
      {Qt::Key_F12, "qrc:/images/gamepad-buttons/keyboard/f12"},
      {Qt::Key_Up, "qrc:/images/gamepad-buttons/keyboard/arrow_up"},
      {Qt::Key_Down, "qrc:/images/gamepad-buttons/keyboard/arrow_down"},
      {Qt::Key_Left, "qrc:/images/gamepad-buttons/keyboard/arrow_left"},
      {Qt::Key_Right, "qrc:/images/gamepad-buttons/keyboard/arrow_right"},
      {Qt::Key_Shift, "qrc:/images/gamepad-buttons/keyboard/shift"},
      {Qt::Key_Control, "qrc:/images/gamepad-buttons/keyboard/ctrl"},
      {Qt::Key_Alt, "qrc:/images/gamepad-buttons/keyboard/alt"},
      {Qt::Key_Meta, "qrc:/images/gamepad-buttons/keyboard/win"},
      {Qt::Key_CapsLock, "qrc:/images/gamepad-buttons/keyboard/capslock"},
      {Qt::Key_NumLock, "qrc:/images/gamepad-buttons/keyboard/numlock"},
      {Qt::Key_ScrollLock, "qrc:/images/gamepad-buttons/keyboard/scroll_lock"},
      {Qt::Key_Space, "qrc:/images/gamepad-buttons/keyboard/space"},
      {Qt::Key_Tab, "qrc:/images/gamepad-buttons/keyboard/tab"},
      {Qt::Key_Backspace, "qrc:/images/gamepad-buttons/keyboard/backspace"},
      {Qt::Key_Return, "qrc:/images/gamepad-buttons/keyboard/return"},
      {Qt::Key_Enter, "qrc:/images/gamepad-buttons/keyboard/enter"},
      {Qt::Key_Escape, "qrc:/images/gamepad-buttons/keyboard/escape"},
      {Qt::Key_Delete, "qrc:/images/gamepad-buttons/keyboard/delete"},
      {Qt::Key_Insert, "qrc:/images/gamepad-buttons/keyboard/insert"},
      {Qt::Key_Home, "qrc:/images/gamepad-buttons/keyboard/home"},
      {Qt::Key_End, "qrc:/images/gamepad-buttons/keyboard/end"},
      {Qt::Key_PageUp, "qrc:/images/gamepad-buttons/keyboard/page_up"},
      {Qt::Key_PageDown, "qrc:/images/gamepad-buttons/keyboard/page_down"},
      {Qt::Key_Menu, "qrc:/images/gamepad-buttons/keyboard/menu"},
      {Qt::Key_Print, "qrc:/images/gamepad-buttons/keyboard/printscreen"},
      {Qt::Key_Pause, "qrc:/images/gamepad-buttons/keyboard/pause"},
      {Qt::Key_Apostrophe, "qrc:/images/gamepad-buttons/keyboard/apostrophe"},
      {Qt::Key_QuoteDbl, "qrc:/images/gamepad-buttons/keyboard/quote"},
      {Qt::Key_QuoteLeft, "qrc:/images/gamepad-buttons/keyboard/tilde"},
      {Qt::Key_AsciiTilde, "qrc:/images/gamepad-buttons/keyboard/tilde"},
      {Qt::Key_Comma, "qrc:/images/gamepad-buttons/keyboard/comma"},
      {Qt::Key_Period, "qrc:/images/gamepad-buttons/keyboard/period"},
      {Qt::Key_Semicolon, "qrc:/images/gamepad-buttons/keyboard/semicolon"},
      {Qt::Key_Colon, "qrc:/images/gamepad-buttons/keyboard/colon"},
      {Qt::Key_Minus, "qrc:/images/gamepad-buttons/keyboard/minus"},
      {Qt::Key_Plus, "qrc:/images/gamepad-buttons/keyboard/plus"},
      {Qt::Key_Equal, "qrc:/images/gamepad-buttons/keyboard/equals"},
      {Qt::Key_Underscore, "qrc:/images/gamepad-buttons/keyboard/underscore"},
      {Qt::Key_Slash, "qrc:/images/gamepad-buttons/keyboard/slash_forward"},
      {Qt::Key_Backslash, "qrc:/images/gamepad-buttons/keyboard/slash_back"},
      {Qt::Key_BracketLeft, "qrc:/images/gamepad-buttons/keyboard/bracket_open"},
      {Qt::Key_BracketRight, "qrc:/images/gamepad-buttons/keyboard/bracket_close"},
      {Qt::Key_Less, "qrc:/images/gamepad-buttons/keyboard/bracket_less"},
      {Qt::Key_Greater, "qrc:/images/gamepad-buttons/keyboard/bracket_greater"},
      {Qt::Key_Asterisk, "qrc:/images/gamepad-buttons/keyboard/asterisk"},
      {Qt::Key_AsciiCircum, "qrc:/images/gamepad-buttons/keyboard/caret"},
      {Qt::Key_Exclam, "qrc:/images/gamepad-buttons/keyboard/exclamation"},
      {Qt::Key_Question, "qrc:/images/gamepad-buttons/keyboard/question"}}},
    {MICROSOFT_XBOX_ONE,
     {{Qt::Key_Enter, "qrc:/images/gamepad-buttons/xbox-series/button_color_a"},
      {Qt::Key_Back, "qrc:/images/gamepad-buttons/xbox-series/button_color_b"},
      {Qt::Key_Menu, "qrc:/images/gamepad-buttons/xbox-series/button_menu"},
      {Qt::Key_Minus, "qrc:/images/gamepad-buttons/xbox-series/lb"},
      {Qt::Key_Plus, "qrc:/images/gamepad-buttons/xbox-series/rb"},
      {Qt::Key_PageDown, "qrc:/images/gamepad-buttons/xbox-series/rt"},
      {Qt::Key_PageUp, "qrc:/images/gamepad-buttons/xbox-series/lt"},
      {Qt::Key_Up, "qrc:/images/gamepad-buttons/xbox-series/dpad_up"},
      {Qt::Key_Down, "qrc:/images/gamepad-buttons/xbox-series/dpad_down"},
      {Qt::Key_Left, "qrc:/images/gamepad-buttons/xbox-series/dpad_left"},
      {Qt::Key_Right, "qrc:/images/gamepad-buttons/xbox-series/dpad_right"},
      {Qt::Key_Home, "qrc:/images/gamepad-buttons/xbox-series/guide"}}}};

QtInputServiceProxy::QtInputServiceProxy(input::InputService &inputService) {
  m_inputService = &inputService;

  m_inputService->setPreferGamepadOverKeyboard(
      m_settings.value("controllers/prioritizeControllerOverKeyboard", true).toBool());

  m_onlyPlayerOneCanNavigateMenus = m_settings.value("controllers/onlyPlayerOneCanNavigateMenus", true).toBool();

  // Initialize auto-repeat timer
  m_autoRepeatTimer = new QTimer(this);
  connect(m_autoRepeatTimer, &QTimer::timeout, this, &QtInputServiceProxy::processAutoRepeat);
  m_autoRepeatTimer->start(16); // ~60 FPS processing

  gamepadInputConnection =
      EventDispatcher::instance().subscribe<input::GamepadInputEvent>([this](const input::GamepadInputEvent &event) {
        // Handle auto-repeat logic
        if (event.pressed && !event.autoRepeat) {
          // Button press - start auto-repeat
          startAutoRepeat(event.playerIndex, event.input);
        } else if (!event.pressed) {
          // Button release - stop auto-repeat
          stopAutoRepeat(event.playerIndex, event.input);
        }

        if (QApplication::focusWindow()) {
          if (!gamepadToQtKeyMap.contains(event.input)) {
            return;
          }

          Qt::Key key = gamepadToQtKeyMap[event.input];

          QMetaObject::invokeMethod(
              QApplication::instance(),
              [key, event]() {
                QApplication::postEvent(
                    QApplication::focusWindow(),
                    new input::GamepadKeyEvent(event.pressed ? QEvent::KeyPress : QEvent::KeyRelease, key,
                                               event.autoRepeat));
              },
              Qt::QueuedConnection);
        }
      });
}

void QtInputServiceProxy::setPrioritizeControllerOverKeyboard(bool prioritizeControllerOverKeyboard) {
  if (m_inputService->preferGamepadOverKeyboard() == prioritizeControllerOverKeyboard) {
    return;
  }

  m_inputService->setPreferGamepadOverKeyboard(prioritizeControllerOverKeyboard);
  m_settings.setValue("controllers/prioritizeControllerOverKeyboard", prioritizeControllerOverKeyboard);
  emit prioritizeControllerOverKeyboardChanged();
}

bool QtInputServiceProxy::prioritizeControllerOverKeyboard() const {
  return m_inputService->preferGamepadOverKeyboard();
}

void QtInputServiceProxy::setOnlyPlayerOneCanNavigateMenus(bool onlyPlayerOneCanNavigateMenus) {
  if (m_onlyPlayerOneCanNavigateMenus == onlyPlayerOneCanNavigateMenus) {
    return;
  }

  m_onlyPlayerOneCanNavigateMenus = onlyPlayerOneCanNavigateMenus;
  m_settings.setValue("controllers/onlyPlayerOneCanNavigateMenus", onlyPlayerOneCanNavigateMenus);
  emit onlyPlayerOneCanNavigateMenusChanged();
}

bool QtInputServiceProxy::getOnlyPlayerOneCanNavigateMenus() const { return m_onlyPlayerOneCanNavigateMenus; }

QVariantMap QtInputServiceProxy::getCurrentGamepadButtonIcons() const {
  QVariantMap result;

  for (auto it = gamepadButtonIcons[MICROSOFT_XBOX_ONE].cbegin(); it != gamepadButtonIcons[MICROSOFT_XBOX_ONE].cend();
       ++it) {
    result.insert(QString::number(it.key()), it.value());
  }

  return result;
}

void QtInputServiceProxy::setShortcutsInGame(const bool inGame) {
  m_inputService->setShortcutContext(inGame ? input::ScopeInGame : input::ScopeInMenu);
}

bool QtInputServiceProxy::isSteamRunning() const { return steamClientRunning(); }

void QtInputServiceProxy::startAutoRepeat(int playerIndex, input::GamepadInput input) {
  if (input == input::None) {
    return;
  }

  auto key = std::make_pair(playerIndex, input);
  auto now = std::chrono::steady_clock::now();
  m_autoRepeatStates[key] = AutoRepeatState{
      .pressTime = now, .lastRepeatTime = now, .isRepeating = false, .playerIndex = playerIndex, .input = input};
}

void QtInputServiceProxy::stopAutoRepeat(int playerIndex, input::GamepadInput input) {
  if (input == input::None) {
    return;
  }

  auto key = std::make_pair(playerIndex, input);
  m_autoRepeatStates.erase(key);
}

void QtInputServiceProxy::processAutoRepeat() {
  auto now = std::chrono::steady_clock::now();

  for (auto &[key, state] : m_autoRepeatStates) {
    auto timeSincePress = now - state.pressTime;
    auto timeSinceLastRepeat = now - state.lastRepeatTime;

    // Check if we should start repeating (after initial delay)
    if (!state.isRepeating && timeSincePress >= AUTO_REPEAT_INITIAL_DELAY) {
      state.isRepeating = true;
      state.lastRepeatTime = now;

      // Send repeat event
      EventDispatcher::instance().publish(input::GamepadInputEvent{
          .playerIndex = state.playerIndex, .input = state.input, .pressed = true, .autoRepeat = true});
    }
    // Check if we should send another repeat event
    else if (state.isRepeating && timeSinceLastRepeat >= AUTO_REPEAT_INTERVAL) {
      state.lastRepeatTime = now;

      // Send repeat event
      EventDispatcher::instance().publish(input::GamepadInputEvent{
          .playerIndex = state.playerIndex, .input = state.input, .pressed = true, .autoRepeat = true});
    }
  }
}

} // namespace firelight::gui

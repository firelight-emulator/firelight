//
// Created by alexs on 8/9/2025
//

#include "qt_input_service_proxy.hpp"

#include "EventEmitter.h"

#include <firelight/input/input_service.hpp>
#include <firelight/input/shortcut_action.hpp>

#include <QApplication>

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

static QMap<input::GamepadInput, Qt::Key> gamepadToQtKeyMap = {{input::DpadRight, Qt::Key_Right},
                                                               {input::DpadLeft, Qt::Key_Left},
                                                               {input::DpadUp, Qt::Key_Up},
                                                               {input::DpadDown, Qt::Key_Down},
                                                               {input::LeftStickRight, Qt::Key_Right},
                                                               {input::LeftStickLeft, Qt::Key_Left},
                                                               {input::LeftStickUp, Qt::Key_Up},
                                                               {input::LeftStickDown, Qt::Key_Down},
                                                               {input::WestFace, Qt::Key_Menu},
                                                               {input::SouthFace, Qt::Key_Select},
                                                               {input::EastFace, Qt::Key_Back},
                                                               {input::LeftBumper, Qt::Key_PageDown},
                                                               {input::RightBumper, Qt::Key_PageUp},
                                                               {input::Home, Qt::Key_Home}};

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
                QApplication::postEvent(QApplication::focusWindow(),
                                        new QKeyEvent(event.pressed ? QEvent::KeyPress : QEvent::KeyRelease, key,
                                                      Qt::KeyboardModifier::KeyboardModifierMask, QString(),
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

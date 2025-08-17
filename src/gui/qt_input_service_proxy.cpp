//
// Created by alexs on 8/9/2025.
//

#include "qt_input_service_proxy.hpp"

#include "EventEmitter.h"
#include "input2/input_service.hpp"

#include <QGuiApplication>

namespace firelight::gui {

static QMap<input::GamepadInput, Qt::Key> gamepadToQtKeyMap = {
    {input::DpadRight, Qt::Key_Right},
    {input::DpadLeft, Qt::Key_Left},
    {input::DpadUp, Qt::Key_Up},
    {input::DpadDown, Qt::Key_Down},
    {input::LeftStickRight, Qt::Key_Right},
    {input::LeftStickLeft, Qt::Key_Left},
    {input::LeftStickUp, Qt::Key_Up},
    {input::LeftStickDown, Qt::Key_Down},
    {input::WestFace, Qt::Key_Menu},
    {input::SouthFace, Qt::Key_Select},
    {input::EastFace, Qt::Key_Back}};

QtInputServiceProxy::QtInputServiceProxy() {
  m_inputService = input::InputService::instance();

  m_inputService->setPreferGamepadOverKeyboard(
      m_settings.value("controllers/prioritizeControllerOverKeyboard", true)
          .toBool());

  m_onlyPlayerOneCanNavigateMenus =
      m_settings.value("controllers/onlyPlayerOneCanNavigateMenus", true)
          .toBool();

  shortcutToggledConnection =
      EventDispatcher::instance().subscribe<input::ShortcutToggledEvent>(
          [this](const input::ShortcutToggledEvent &event) {
            emit shortcutToggled(event.playerIndex, event.shortcut);
          });

  gamepadInputConnection =
      EventDispatcher::instance().subscribe<input::GamepadInputEvent>(
          [this](const input::GamepadInputEvent &event) {
            if (QGuiApplication::focusWindow()) {
              if (!gamepadToQtKeyMap.contains(event.input)) {
                return;
              }

              Qt::Key key = gamepadToQtKeyMap[event.input];

              QMetaObject::invokeMethod(
                  QApplication::instance(),
                  [key, event]() {
                    QApplication::postEvent(
                        QGuiApplication::focusWindow(),
                        new QKeyEvent(
                            event.pressed ? QEvent::KeyPress
                                          : QEvent::KeyRelease,
                            key, Qt::KeyboardModifier::KeyboardModifierMask));
                  },
                  Qt::QueuedConnection);
            }
          });
}
void QtInputServiceProxy::setPrioritizeControllerOverKeyboard(
    bool prioritizeControllerOverKeyboard) {
  if (m_inputService->preferGamepadOverKeyboard() ==
      prioritizeControllerOverKeyboard) {
    return;
  }

  m_inputService->setPreferGamepadOverKeyboard(
      prioritizeControllerOverKeyboard);
  m_settings.setValue("controllers/prioritizeControllerOverKeyboard",
                      prioritizeControllerOverKeyboard);
  emit prioritizeControllerOverKeyboardChanged();
}

bool QtInputServiceProxy::prioritizeControllerOverKeyboard() const {
  return m_inputService->preferGamepadOverKeyboard();
}

void QtInputServiceProxy::setOnlyPlayerOneCanNavigateMenus(
    bool onlyPlayerOneCanNavigateMenus) {
  if (m_onlyPlayerOneCanNavigateMenus == onlyPlayerOneCanNavigateMenus) {
    return;
  }

  m_onlyPlayerOneCanNavigateMenus = onlyPlayerOneCanNavigateMenus;
  m_settings.setValue("controllers/onlyPlayerOneCanNavigateMenus",
                      onlyPlayerOneCanNavigateMenus);
  emit onlyPlayerOneCanNavigateMenusChanged();
}

bool QtInputServiceProxy::getOnlyPlayerOneCanNavigateMenus() const {
  return m_onlyPlayerOneCanNavigateMenus;
}
} // namespace firelight::gui
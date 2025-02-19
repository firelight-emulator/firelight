#include "sdl_event_loop.hpp"

#define SDL_MAIN_HANDLED
#include <QApplication>
#include <qevent.h>
#include <SDL.h>
#include <SDL_hints.h>
#include <spdlog/spdlog.h>
#include "keyboard_input_handler.hpp"

namespace firelight {
  SdlEventLoop::SdlEventLoop(QWindow *window, input::ControllerManager *manager)
    : m_window(window), m_controllerManager(manager) {
    auto *keyboard = new input::KeyboardInputHandler();
    m_window->installEventFilter(keyboard);
    manager->setKeyboardRetropad(keyboard);

    SDL_SetHint(SDL_HINT_APP_NAME, "Firelight");
    SDL_SetHint(SDL_HINT_GAMECONTROLLER_USE_BUTTON_LABELS, "0");

    if (SDL_Init(SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC | SDL_INIT_TIMER) < 0) {
      spdlog::error("SDL could not initialize! SDL_Error: {}", SDL_GetError());
    }

    SDL_GameControllerAddMappingsFromFile("gamecontrollerdb.txt");
  }

  SdlEventLoop::~SdlEventLoop() {
    m_running = false;
    SDL_Event quitEvent;
    quitEvent.type = SDL_QUIT;
    SDL_PushEvent(&quitEvent);
    SDL_QuitSubSystem(SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC |
                      SDL_INIT_TIMER);
    SDL_Quit();
  }

  void SdlEventLoop::stopProcessing() {
    SDL_Event quitEvent;
    quitEvent.type = SDL_QUIT;
    SDL_PushEvent(&quitEvent);
    m_running = false;
  }

  void SdlEventLoop::run() {
    processEvents();
    SDL_QuitSubSystem(SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC |
                      SDL_INIT_TIMER);
    SDL_Quit();
  }

  void SdlEventLoop::processEvents() const {
    while (m_running) {
      SDL_Event ev;
      while (SDL_WaitEvent(&ev)) {
        switch (ev.type) {
          case SDL_KEYDOWN:
          case SDL_KEYUP:
            // TODO
            break;
          case SDL_CONTROLLERDEVICEADDED:
          case SDL_CONTROLLERDEVICEREMOVED:
          case SDL_CONTROLLERAXISMOTION:
            m_controllerManager->handleSDLControllerEvent(ev);
            break;
          case SDL_CONTROLLERBUTTONUP: {
            m_controllerManager->handleSDLControllerEvent(ev);

            if (m_controllerManager->m_blockGamepadInput) {
              break;
            }
            if (QGuiApplication::focusWindow()) {
              Qt::Key key;

              auto button = ev.cbutton.button;
              if (button == SDL_CONTROLLER_BUTTON_DPAD_RIGHT) {
                key = Qt::Key_Right;
              } else if (button == SDL_CONTROLLER_BUTTON_DPAD_LEFT) {
                key = Qt::Key_Left;
              } else if (button == SDL_CONTROLLER_BUTTON_DPAD_UP) {
                key = Qt::Key_Up;
              } else if (button == SDL_CONTROLLER_BUTTON_DPAD_DOWN) {
                key = Qt::Key_Down;
              } else if (button == SDL_CONTROLLER_BUTTON_X) {
                key = Qt::Key_Menu;
              } else if (button == SDL_CONTROLLER_BUTTON_A) {
                key = Qt::Key_Select;
              } else if (button == SDL_CONTROLLER_BUTTON_B) {
                key = Qt::Key_Back;
              } else if (button == SDL_CONTROLLER_BUTTON_GUIDE) {
                key = Qt::Key_Home;
              } else {
                break;
              }

              QMetaObject::invokeMethod(QApplication::instance(), [key]() {
                QApplication::postEvent(QGuiApplication::focusWindow(),
                                        new QKeyEvent(QEvent::KeyRelease, key, Qt::KeyboardModifier::NoModifier));
              }, Qt::QueuedConnection);

              // // TODO: I think it complains about this.
              // QApplication::postEvent(
              //   QGuiApplication::focusWindow(),
              //   new QKeyEvent(QEvent::KeyRelease, key, Qt::KeyboardModifier::NoModifier));
            }

            break;
          }
          case SDL_CONTROLLERBUTTONDOWN: {
            m_controllerManager->handleSDLControllerEvent(ev);

            if (m_controllerManager->m_blockGamepadInput) {
              break;
            }

            if (QGuiApplication::focusWindow()) {
              Qt::Key key;

              auto button = ev.cbutton.button;
              if (button == SDL_CONTROLLER_BUTTON_DPAD_RIGHT) {
                key = Qt::Key_Right;
              } else if (button == SDL_CONTROLLER_BUTTON_DPAD_LEFT) {
                key = Qt::Key_Left;
              } else if (button == SDL_CONTROLLER_BUTTON_DPAD_UP) {
                key = Qt::Key_Up;
              } else if (button == SDL_CONTROLLER_BUTTON_DPAD_DOWN) {
                key = Qt::Key_Down;
              } else if (button == SDL_CONTROLLER_BUTTON_X) {
                key = Qt::Key_Menu;
              } else if (button == SDL_CONTROLLER_BUTTON_A) {
                key = Qt::Key_Select;
              } else if (button == SDL_CONTROLLER_BUTTON_B) {
                key = Qt::Key_Back;
              } else if (button == SDL_CONTROLLER_BUTTON_GUIDE) {
                key = Qt::Key_Home;
              } else {
                break;
              }

              QMetaObject::invokeMethod(QApplication::instance(), [key]() {
                QApplication::postEvent(QGuiApplication::focusWindow(),
                                        new QKeyEvent(QEvent::KeyPress, key, Qt::KeyboardModifier::NoModifier));
              }, Qt::QueuedConnection);

              // QApplication::postEvent(
              //   QGuiApplication::focusWindow(), new QKeyEvent(QEvent::KeyPress, key, Qt::KeyboardModifier::NoModifier));
            }

            break;
          }
          // printf("button: %d, state: %d\n", ev.cbutton.button, ev.cbutton.state);
          // break;
          case SDL_JOYAXISMOTION:
          case SDL_JOYBUTTONUP:
          case SDL_JOYBUTTONDOWN:
            break;
          case SDL_QUIT:
            return;
          default:
            spdlog::debug("Got an unhandled SDL event {}", ev.type);
            break;
        }
      }
    }
  }
} // namespace firelight

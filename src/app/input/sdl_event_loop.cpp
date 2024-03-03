//
// Created by alexs on 1/6/2024.
//

#include "sdl_event_loop.hpp"

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_hints.h>
#include <spdlog/spdlog.h>

namespace firelight {

SdlEventLoop::SdlEventLoop(Input::ControllerManager *manager)
    : m_controllerManager(manager) {
  SDL_SetHint(SDL_HINT_APP_NAME, "Firelight");
  SDL_SetHint(SDL_HINT_GAMECONTROLLER_USE_BUTTON_LABELS, "0");

  if (SDL_Init(SDL_INIT_GAMECONTROLLER | SDL_INIT_AUDIO | SDL_INIT_HAPTIC |
               SDL_INIT_TIMER) < 0) {
    printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
  }

  SDL_GameControllerAddMappingsFromFile("gamecontrollerdb.txt");
}
SdlEventLoop::~SdlEventLoop() {
  m_running = false;
  SDL_Event quitEvent;
  quitEvent.type = SDL_QUIT;
  SDL_PushEvent(&quitEvent);
}

void SdlEventLoop::stopProcessing() {
  m_running = false;
  SDL_Event quitEvent;
  quitEvent.type = SDL_QUIT;
  SDL_PushEvent(&quitEvent);
}

void SdlEventLoop::run() {
  processEvents();
  SDL_QuitSubSystem(SDL_INIT_GAMECONTROLLER | SDL_INIT_AUDIO | SDL_INIT_HAPTIC |
                    SDL_INIT_TIMER);
  SDL_Quit();
}

void SdlEventLoop::processEvents() const {
  while (m_running) {
    SDL_Event ev;
    while (SDL_WaitEvent(&ev)) {
      switch (ev.type) {
      case SDL_CONTROLLERDEVICEADDED:
      case SDL_CONTROLLERDEVICEREMOVED:
        m_controllerManager->handleSDLControllerEvent(ev);
        break;
      case SDL_JOYAXISMOTION:
      case SDL_CONTROLLERAXISMOTION:
      case SDL_JOYBUTTONUP:
      case SDL_JOYBUTTONDOWN:
      case SDL_CONTROLLERBUTTONUP:
      case SDL_CONTROLLERBUTTONDOWN:
        break;
      case SDL_QUIT:
        spdlog::info("Got shut down signal; stopping SDL event loop");
        return;
      default:
        spdlog::debug("Got an unhandled SDL event {}", ev.type);
        break;
      }
    }
  }
}

} // namespace Firelight
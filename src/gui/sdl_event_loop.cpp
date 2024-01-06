//
// Created by alexs on 1/6/2024.
//

#include "sdl_event_loop.hpp"

#include <SDL.h>
#include <SDL_hints.h>
#include <spdlog/spdlog.h>

namespace firelight {

SdlEventLoop::SdlEventLoop(input::ControllerManager *manager)
    : m_controllerManager(manager) {
  SDL_SetHint(SDL_HINT_APP_NAME, "Firelight");
  SDL_SetHint(SDL_HINT_GAMECONTROLLER_USE_BUTTON_LABELS, "0");

  if (SDL_Init(SDL_INIT_GAMECONTROLLER | SDL_INIT_AUDIO | SDL_INIT_HAPTIC |
               SDL_INIT_TIMER) < 0) {
    printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
  }

  SDL_GameControllerAddMappingsFromFile("gamecontrollerdb.txt");
}
void SdlEventLoop::stop() { m_running = false; }

void SdlEventLoop::run() {
  while (m_running) {
    SDL_Event ev;
    while (SDL_WaitEvent(&ev)) {
      switch (ev.type) {
      case SDL_CONTROLLERDEVICEADDED:
      case SDL_CONTROLLERDEVICEREMOVED:
        m_controllerManager->handleSDLControllerEvent(ev);
        break;
      default:
        spdlog::debug("Got an unhandled SDL event {}", ev.type);
        break;
      }
    }
  }
}

} // namespace firelight
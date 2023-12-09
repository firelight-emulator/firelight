//
// Created by alexs on 11/13/2023.
//

#include "screen_manager.hpp"

namespace FL::GUI {
void ScreenManager::handleSdlEvent(SDL_Event *event) {
  FL::GUI::Event guiEvent{};

  switch (event->type) {
  case SDL_KEYDOWN:
    switch (event->key.keysym.sym) {
    case SDLK_DOWN:
      guiEvent.type = Event::NAV_DOWN;
      break;
    case SDLK_UP:
      guiEvent.type = Event::NAV_UP;
      break;
    case SDLK_SPACE:
    case SDLK_RETURN:
      guiEvent.type = Event::NAV_SELECT_PUSHED;
      break;
    }
    break;
  case SDL_CONTROLLERBUTTONDOWN:
    switch (event->cbutton.button) {
    case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
      guiEvent.type = Event::NAV_DOWN;
      break;
    case SDL_CONTROLLER_BUTTON_DPAD_UP:
      guiEvent.type = Event::NAV_UP;
      break;
    case SDL_CONTROLLER_BUTTON_A:
      guiEvent.type = Event::NAV_SELECT_PUSHED;
      break;
    case SDL_CONTROLLER_BUTTON_B:
      guiEvent.type = Event::NAV_BACK_PUSHED;
      break;
    case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
      guiEvent.type = Event::TEST;
      break;
    default:
      return;
    }
    break;
  default:
    return;
  }

  if (!screenStack.empty()) {
    screenStack.top()->handleEvent(guiEvent); // Update the current screen
  }
}

} // namespace FL::GUI

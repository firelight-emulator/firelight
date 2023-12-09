//
// Created by alexs on 10/27/2023.
//

#include "controller_manager.hpp"
#include <cstring>

namespace FL::Input {

void ControllerManager::handleControllerAddedEvent(int32_t sdlJoystickIndex) {
  printf("got event for joystick index %d\n", sdlJoystickIndex);
  // Check if we already have a controller for this joystick index
  auto controller = SDL_GameControllerOpen(sdlJoystickIndex);
  if (controller == nullptr) {
    return;
  }

  for (int i = 0; i < portAssignedControllers.max_size(); ++i) {
    if (portAssignedControllers[i]) {
      if (portAssignedControllers[i].get()->sdlJoystickIndex ==
          sdlJoystickIndex) {
        SDL_GameControllerClose(portAssignedControllers[i]->sdlController);
        portAssignedControllers[i].reset();
      }
    }
  }

  for (int i = 0; i < unassignedControllers.size(); ++i) {
    if (unassignedControllers[i]) {
      if (unassignedControllers[i].get()->sdlJoystickIndex ==
          sdlJoystickIndex) {
        SDL_GameControllerClose(unassignedControllers[i]->sdlController);
        unassignedControllers[i].reset();
      }
    }
  }

  // check  ^^^^
  // assign vvvv

  for (int i = 0; i < portAssignedControllers.max_size(); ++i) {
    if (!portAssignedControllers[i]) {
      portAssignedControllers[i] =
          std::make_unique<FL::Input::SDLGamepad>(controller);
      return;
    }
  }

  // If we didn't find a spot in the player slots, make it unassigned.
  unassignedControllers.push_back(
      std::make_unique<FL::Input::SDLGamepad>(controller));

  // Keyboard controller should ALWAYS be last by default - let users move it
  // if desired
}

void ControllerManager::handleControllerRemovedEvent(int32_t sdlInstanceId) {
  auto con = SDL_GameControllerFromInstanceID(sdlInstanceId);
  auto joystickIndex = SDL_JoystickGetDeviceInstanceID(sdlInstanceId);

  for (int i = 0; i < portAssignedControllers.max_size(); ++i) {
    if (portAssignedControllers[i]) {
      if (portAssignedControllers[i].get()->sdlJoystickIndex == joystickIndex) {
        SDL_GameControllerClose(portAssignedControllers[i]->sdlController);
        portAssignedControllers[i].reset();
      }
    }
  }

  for (int i = 0; i < unassignedControllers.size(); ++i) {
    if (unassignedControllers[i]) {
      if (unassignedControllers[i].get()->sdlJoystickIndex == joystickIndex) {
        SDL_GameControllerClose(unassignedControllers[i]->sdlController);
        unassignedControllers[i].reset();
      }
    }
  }
}

void ControllerManager::scanGamepads() {
  auto numJoys = SDL_NumJoysticks();
  for (int i = 0; i < numJoys; ++i) {
    auto controller = SDL_GameControllerOpen(i);
    if (controller == nullptr) {
      continue;
    }
    // else check if we already know about it somehow

    handleControllerAddedEvent(i);
  }
}
FL::Input::SDLGamepad *ControllerManager::getGamepad(int playerIndex) {
  return portAssignedControllers[playerIndex].get();
}

} // namespace FL::Input
//
// Created by alexs on 10/27/2023.
//

#ifndef FIRELIGHT_CONTROLLER_MANAGER_HPP
#define FIRELIGHT_CONTROLLER_MANAGER_HPP

#include "sdl_controller.hpp"
#include <array>
#include <cstdint>
#include <memory>
#include <vector>
namespace FL::Input {

class ControllerManager {
public:
  void handleControllerAddedEvent(int32_t sdlJoystickIndex);
  void handleControllerRemovedEvent(int32_t sdlInstanceId);
  void scanGamepads();
  FL::Input::SDLGamepad *getGamepad(int port);

private:
  static const int MAX_PLAYERS = 8;
  std::vector<std::unique_ptr<FL::Input::SDLGamepad>> unassignedControllers;
  std::array<std::unique_ptr<FL::Input::SDLGamepad>, MAX_PLAYERS>
      portAssignedControllers;
};

} // namespace FL::Input

#endif // FIRELIGHT_CONTROLLER_MANAGER_HPP

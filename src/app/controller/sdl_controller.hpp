//
// Created by alexs on 12/5/2023.
//

#ifndef FIRELIGHT_SDL_CONTROLLER_HPP
#define FIRELIGHT_SDL_CONTROLLER_HPP

#include "SDL_gamecontroller.h"
#include "SDL_joystick.h"
#include <memory>
#include <string>

namespace FL::Input {

class SDLGamepad {
public:
  explicit SDLGamepad(SDL_GameController *controller);
  std::string getSerial();
  SDL_GameController *sdlController;
  int32_t sdlJoystickIndex;

  bool isLibretroButtonPressed(int button);
  int16_t getLeftStickXPosition();
  int16_t getLeftStickYPosition();
  int16_t getRightStickXPosition();
  int16_t getRightStickYPosition();

private:
  std::string deviceName = "";
  std::string serial = "";
};

} // namespace FL::Input

#endif // FIRELIGHT_SDL_CONTROLLER_HPP

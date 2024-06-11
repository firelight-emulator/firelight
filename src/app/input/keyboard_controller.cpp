//
// Created by alexs on 2/15/2024.
//

#include "keyboard_controller.hpp"

#include <SDL_keyboard.h>

namespace firelight::Input {

void KeyboardController::setStrongRumble(uint16_t t_strength) {}
void KeyboardController::setWeakRumble(uint16_t t_strength) {}
bool KeyboardController::isButtonPressed(Button t_button) {
  const uint8_t *currentKeyStates = SDL_GetKeyboardState(nullptr);

  switch (t_button) {
  case NorthFace:
    return currentKeyStates[SDL_SCANCODE_S];
  case SouthFace:
    return currentKeyStates[SDL_SCANCODE_Z];
  case EastFace:
    return currentKeyStates[SDL_SCANCODE_X];
  case WestFace:
    return currentKeyStates[SDL_SCANCODE_A];
  case DpadUp:
    return currentKeyStates[SDL_SCANCODE_UP];
  case DpadDown:
    return currentKeyStates[SDL_SCANCODE_DOWN];
  case DpadLeft:
    return currentKeyStates[SDL_SCANCODE_LEFT];
  case DpadRight:
    return currentKeyStates[SDL_SCANCODE_RIGHT];
  case Start:
    return currentKeyStates[SDL_SCANCODE_RETURN];
  case Select:
    return currentKeyStates[SDL_SCANCODE_SLASH];
  case LeftBumper:
    return currentKeyStates[SDL_SCANCODE_Q];
  case RightBumper:
    return currentKeyStates[SDL_SCANCODE_W];
  case LeftTrigger:
  case RightTrigger:
  case L3:
  case R3:
  default:
    return false;
  }
}
int16_t KeyboardController::getLeftStickXPosition() { return 0; }
int16_t KeyboardController::getLeftStickYPosition() { return 0; }
int16_t KeyboardController::getRightStickXPosition() { return 0; }
int16_t KeyboardController::getRightStickYPosition() { return 0; }

} // namespace firelight::Input
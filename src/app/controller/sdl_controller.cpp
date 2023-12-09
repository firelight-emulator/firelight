//
// Created by alexs on 12/5/2023.
//

#include "sdl_controller.hpp"
#include "../libretro/libretro.h"

namespace FL::Input {
SDLGamepad::SDLGamepad(SDL_GameController *controller)
    : sdlController(controller) {
  auto joystick = SDL_GameControllerGetJoystick(controller);
  auto guid = SDL_JoystickGetGUID(joystick);

  char buf[33];
  SDL_GUIDToString(guid, buf, 33);
  printf("GUID: %s\n", buf);

  auto instanceID =
      SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(controller));
  sdlJoystickIndex = SDL_JoystickGetDeviceInstanceID(instanceID);

  auto serialCStr = SDL_GameControllerGetSerial(controller);
  if (serialCStr != nullptr) {
    serial = serialCStr;
  }

  printf("name: %s\n", SDL_GameControllerName(controller));
  printf("type: %u\n", SDL_GameControllerGetType(controller));

  printf("vendor: %u\n", SDL_GameControllerGetVendor(controller));
  printf("product: %u\n", SDL_GameControllerGetProduct(controller));
  printf("product version: %u\n",
         SDL_GameControllerGetProductVersion(controller));
}
std::string SDLGamepad::getSerial() { return serial; }

bool SDLGamepad::isLibretroButtonPressed(int button) {

  // retropad A is east, SDL A is south...
  if (button == RETRO_DEVICE_ID_JOYPAD_A &&
      SDL_GameControllerGetButton(sdlController, SDL_CONTROLLER_BUTTON_B)) {
    return true;
  }
  if (button == RETRO_DEVICE_ID_JOYPAD_B &&
      SDL_GameControllerGetButton(sdlController, SDL_CONTROLLER_BUTTON_A)) {
    return true;
  }
  if (button == RETRO_DEVICE_ID_JOYPAD_X &&
      SDL_GameControllerGetButton(sdlController, SDL_CONTROLLER_BUTTON_Y)) {
    return true;
  }
  if (button == RETRO_DEVICE_ID_JOYPAD_Y &&
      SDL_GameControllerGetButton(sdlController, SDL_CONTROLLER_BUTTON_X)) {
    return true;
  }
  if (button == RETRO_DEVICE_ID_JOYPAD_LEFT &&
      SDL_GameControllerGetButton(sdlController,
                                  SDL_CONTROLLER_BUTTON_DPAD_LEFT)) {
    return true;
  }
  if (button == RETRO_DEVICE_ID_JOYPAD_RIGHT &&
      SDL_GameControllerGetButton(sdlController,
                                  SDL_CONTROLLER_BUTTON_DPAD_RIGHT)) {
    return true;
  }
  if (button == RETRO_DEVICE_ID_JOYPAD_UP &&
      SDL_GameControllerGetButton(sdlController,
                                  SDL_CONTROLLER_BUTTON_DPAD_UP)) {
    return true;
  }
  if (button == RETRO_DEVICE_ID_JOYPAD_DOWN &&
      SDL_GameControllerGetButton(sdlController,
                                  SDL_CONTROLLER_BUTTON_DPAD_DOWN)) {
    return true;
  }
  if (button == RETRO_DEVICE_ID_JOYPAD_START &&
      SDL_GameControllerGetButton(sdlController, SDL_CONTROLLER_BUTTON_START)) {
    return true;
  }
  if (button == RETRO_DEVICE_ID_JOYPAD_SELECT &&
      SDL_GameControllerGetButton(sdlController, SDL_CONTROLLER_BUTTON_BACK)) {
    return true;
  }
  if (button == RETRO_DEVICE_ID_JOYPAD_L &&
      SDL_GameControllerGetButton(sdlController,
                                  SDL_CONTROLLER_BUTTON_LEFTSHOULDER)) {
    return true;
  }
  if (button == RETRO_DEVICE_ID_JOYPAD_R &&
      SDL_GameControllerGetButton(sdlController,
                                  SDL_CONTROLLER_BUTTON_RIGHTSHOULDER)) {
    return true;
  }
  if (button == RETRO_DEVICE_ID_JOYPAD_L2 &&
      SDL_GameControllerGetAxis(sdlController,
                                SDL_CONTROLLER_AXIS_TRIGGERLEFT) > 0) {
    return true;
  }
  if (button == RETRO_DEVICE_ID_JOYPAD_R2 &&
      SDL_GameControllerGetAxis(sdlController,
                                SDL_CONTROLLER_AXIS_TRIGGERRIGHT) > 0) {
    return true;
  }
  if (button == RETRO_DEVICE_ID_JOYPAD_L3 &&
      SDL_GameControllerGetButton(sdlController,
                                  SDL_CONTROLLER_BUTTON_LEFTSTICK)) {
    return true;
  }
  if (button == RETRO_DEVICE_ID_JOYPAD_R3 &&
      SDL_GameControllerGetButton(sdlController,
                                  SDL_CONTROLLER_BUTTON_RIGHTSTICK)) {
    return true;
  }

  return false;
}
int16_t SDLGamepad::getLeftStickXPosition() {
  return SDL_GameControllerGetAxis(sdlController, SDL_CONTROLLER_AXIS_LEFTX);
}
int16_t SDLGamepad::getLeftStickYPosition() {
  return SDL_GameControllerGetAxis(sdlController, SDL_CONTROLLER_AXIS_LEFTY);
}
int16_t SDLGamepad::getRightStickXPosition() {
  return SDL_GameControllerGetAxis(sdlController, SDL_CONTROLLER_AXIS_RIGHTX);
}
int16_t SDLGamepad::getRightStickYPosition() {
  return SDL_GameControllerGetAxis(sdlController, SDL_CONTROLLER_AXIS_RIGHTY);
}
} // namespace FL::Input

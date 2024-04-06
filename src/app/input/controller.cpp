//
// Created by alexs on 1/6/2024.
//

#include "controller.hpp"

namespace firelight::Input {
Controller::~Controller() = default;
Controller::Controller(SDL_GameController *t_controller,
                       const int32_t t_joystickIndex)
    : m_SDLController(t_controller), m_SDLJoystickDeviceIndex(t_joystickIndex) {

  m_SDLJoystick = SDL_GameControllerGetJoystick(t_controller);
  m_SDLJoystickInstanceId = SDL_JoystickInstanceID(m_SDLJoystick);
}

bool Controller::isButtonPressed(const Button t_button) {
  switch (t_button) {
  case NorthFace:
    return SDL_GameControllerGetButton(m_SDLController,
                                       SDL_CONTROLLER_BUTTON_Y);
  case SouthFace:
    return SDL_GameControllerGetButton(m_SDLController,
                                       SDL_CONTROLLER_BUTTON_A);
  case EastFace:
    return SDL_GameControllerGetButton(m_SDLController,
                                       SDL_CONTROLLER_BUTTON_B);
  case WestFace:
    return SDL_GameControllerGetButton(m_SDLController,
                                       SDL_CONTROLLER_BUTTON_X);
  case DpadUp:
    return SDL_GameControllerGetButton(m_SDLController,
                                       SDL_CONTROLLER_BUTTON_DPAD_UP);
  case DpadDown:
    return SDL_GameControllerGetButton(m_SDLController,
                                       SDL_CONTROLLER_BUTTON_DPAD_DOWN);
  case DpadLeft:
    return SDL_GameControllerGetButton(m_SDLController,
                                       SDL_CONTROLLER_BUTTON_DPAD_LEFT);
  case DpadRight:
    return SDL_GameControllerGetButton(m_SDLController,
                                       SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
  case Start:
    return SDL_GameControllerGetButton(m_SDLController,
                                       SDL_CONTROLLER_BUTTON_START);
  case Select:
    return SDL_GameControllerGetButton(m_SDLController,
                                       SDL_CONTROLLER_BUTTON_BACK);
  case LeftBumper:
    return SDL_GameControllerGetButton(m_SDLController,
                                       SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
  case RightBumper:
    return SDL_GameControllerGetButton(m_SDLController,
                                       SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
  case LeftTrigger:
    return SDL_GameControllerGetAxis(m_SDLController,
                                     SDL_CONTROLLER_AXIS_TRIGGERLEFT) > 0;
  case RightTrigger:
    return SDL_GameControllerGetAxis(m_SDLController,
                                     SDL_CONTROLLER_AXIS_TRIGGERRIGHT) > 0;
  case L3:
    return SDL_GameControllerGetButton(m_SDLController,
                                       SDL_CONTROLLER_BUTTON_LEFTSTICK);
  case R3:
    return SDL_GameControllerGetButton(m_SDLController,
                                       SDL_CONTROLLER_BUTTON_RIGHTSTICK);
  default:
    return false;
  }
}

int16_t Controller::getLeftStickXPosition() {
  return SDL_GameControllerGetAxis(m_SDLController, SDL_CONTROLLER_AXIS_LEFTX);
}
int16_t Controller::getLeftStickYPosition() {
  return SDL_GameControllerGetAxis(m_SDLController, SDL_CONTROLLER_AXIS_LEFTY);
}
int16_t Controller::getRightStickXPosition() {
  return SDL_GameControllerGetAxis(m_SDLController, SDL_CONTROLLER_AXIS_RIGHTX);
}
int16_t Controller::getRightStickYPosition() {
  return SDL_GameControllerGetAxis(m_SDLController, SDL_CONTROLLER_AXIS_RIGHTY);
}
int32_t Controller::getInstanceId() const { return m_SDLJoystickInstanceId; }
int32_t Controller::getDeviceIndex() const { return m_SDLJoystickDeviceIndex; }

std::string Controller::getControllerName() const {
  return {SDL_GameControllerName(m_SDLController)};
}

void Controller::setPlayerIndex(const int t_newPlayerIndex) const {
  SDL_GameControllerSetPlayerIndex(m_SDLController, t_newPlayerIndex);
}

int Controller::getPlayerIndex() const {
  return SDL_GameControllerGetPlayerIndex(m_SDLController);
}

void Controller::setStrongRumble(const uint16_t t_strength) {
  printf("Setting strong rumble: %d\n", t_strength);
  SDL_JoystickRumble(m_SDLJoystick, 0, t_strength, 2000);
}

void Controller::setWeakRumble(const uint16_t t_strength) {
  printf("Setting weak rumble: %d\n", t_strength);
  SDL_JoystickRumble(m_SDLJoystick, t_strength, 0, 2000);
}

} // namespace firelight::Input

//
// Created by alexs on 1/6/2024.
//

#include "controller.hpp"

namespace firelight::input {
Controller::~Controller() = default;
Controller::Controller(SDL_GameController *t_controller,
                       const int32_t t_joystickIndex)
    : m_SDLController(t_controller), m_SDLJoystickDeviceIndex(t_joystickIndex) {

  const auto joystick = SDL_GameControllerGetJoystick(t_controller);
  m_SDLJoystickInstanceId = SDL_JoystickInstanceID(joystick);
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

} // namespace firelight::input

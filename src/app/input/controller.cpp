#include "controller.hpp"

namespace firelight::Input {
  Controller::~Controller() = default;

  void Controller::setControllerProfile(const std::shared_ptr<input::ControllerProfile> &profile) {
    m_profile = profile;
    printf("Setting profile to %p\n", m_profile.get());
  }

  Controller::Controller(SDL_GameController *t_controller,
                         const int32_t t_joystickIndex)
    : m_SDLController(t_controller), m_SDLJoystickDeviceIndex(t_joystickIndex) {
    m_SDLJoystick = SDL_GameControllerGetJoystick(t_controller);
    m_SDLJoystickInstanceId = SDL_JoystickInstanceID(m_SDLJoystick);

    // const auto vendorId = SDL_JoystickGetVendor(m_SDLJoystick);
    // const auto productId = SDL_JoystickGetProduct(m_SDLJoystick);

    // printf("vendorId: %d\n", vendorId);
    // printf("productId: %d\n", productId);
    // printf("Power status: %d\n", SDL_JoystickCurrentPowerLevel(m_SDLJoystick));
    // printf("Has rumble: %d\n", SDL_JoystickHasRumble(m_SDLJoystick));
    // printf("Has LED: %d\n", SDL_JoystickHasLED(m_SDLJoystick));
    // printf("Has rumble triggers: %d\n", SDL_JoystickHasRumbleTriggers(m_SDLJoystick));
  }

  bool Controller::isButtonPressed(const Button t_button) {
    if (auto mapping = m_profile->getControllerMappingForPlatform(0); mapping.has_value()) {
      if (const auto result = mapping->evaluateButtonMapping(m_SDLJoystick, t_button); result.has_value()) {
        return result.value();
      }
    }

    // if (m_profile) {
    //   auto mapping = m_profile->getControllerMappingForPlatform(0);
    //
    //
    //   if (t_button == LeftTrigger || t_button == RightTrigger) {
    //     return SDL_GameControllerGetAxis(m_SDLController,
    //                                      t_button == LeftTrigger
    //                                        ? SDL_CONTROLLER_AXIS_TRIGGERLEFT
    //                                        : SDL_CONTROLLER_AXIS_TRIGGERRIGHT) > 0;
    //   }
    //
    //   const auto mappedButton = mapping.getMappedButton(t_button);
    //
    //   if (mappedButton == SDL_CONTROLLER_BUTTON_INVALID) {
    //     return false;
    //   }
    //
    //   return SDL_GameControllerGetButton(m_SDLController, mappedButton);
    // }

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
    const auto value = SDL_GameControllerGetAxis(m_SDLController, SDL_CONTROLLER_AXIS_LEFTX);

    // if (m_profile != nullptr) {
    //   auto mapping = m_profile->getControllerMappingForPlatform(0);
    //   const auto deadzone = mapping.getLeftStickXDeadzone();
    //   if (value < deadzone && value > -deadzone) {
    //     return 0;
    //   }
    // }

    return value;
  }

  int16_t Controller::getLeftStickYPosition() {
    const auto value = SDL_GameControllerGetAxis(m_SDLController, SDL_CONTROLLER_AXIS_LEFTY);

    // if (m_profile != nullptr) {
    //   auto mapping = m_profile->getControllerMappingForPlatform(0);
    //   const auto deadzone = mapping.getLeftStickYDeadzone();
    //   if (value < deadzone && value > -deadzone) {
    //     return 0;
    //   }
    // }

    return value;
  }

  int16_t Controller::getRightStickXPosition() {
    const auto value = SDL_GameControllerGetAxis(m_SDLController, SDL_CONTROLLER_AXIS_RIGHTX);

    // if (m_profile != nullptr) {
    //   auto mapping = m_profile->getControllerMappingForPlatform(0);
    //   const auto deadzone = mapping.getRightStickXDeadzone();
    //   if (value < deadzone && value > -deadzone) {
    //     return 0;
    //   }
    // }

    return value;
  }

  int16_t Controller::getRightStickYPosition() {
    const auto value = SDL_GameControllerGetAxis(m_SDLController, SDL_CONTROLLER_AXIS_RIGHTY);

    // if (m_profile != nullptr) {
    //   auto mapping = m_profile->getControllerMappingForPlatform(0);
    //   const auto deadzone = mapping.getRightStickYDeadzone();
    //   if (value < deadzone && value > -deadzone) {
    //     return 0;
    //   }
    // }

    return value;
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

  // TODO: This isn't quite right, will set the rumble for only one motor at a
  // time
  void Controller::setStrongRumble(const uint16_t t_strength) {
    SDL_JoystickRumble(m_SDLJoystick, 0, t_strength, 2000);
  }

  void Controller::setWeakRumble(const uint16_t t_strength) {
    SDL_JoystickRumble(m_SDLJoystick, t_strength, 0, 2000);
  }

  bool Controller::isWired() const {
    return SDL_JoystickCurrentPowerLevel(m_SDLJoystick) == SDL_JOYSTICK_POWER_WIRED;
  }

  GamepadType Controller::getGamepadType() const {
    auto vendorId = SDL_JoystickGetVendor(m_SDLJoystick);
    auto productId = SDL_JoystickGetProduct(m_SDLJoystick);

    switch (vendorId) {
      case 1118:
        switch (productId) {
          case 767:
            return MICROSOFT_XBOX_ONE;
          default:
            return UNKNOWN;
        }
      case 1406:
        switch (productId) {
          case 8201:
            return NINTENDO_SWITCH_PRO;
          case 8217:
            return NINTENDO_NSO_N64;
          case 8215:
            return NINTENDO_NSO_SNES;
          default:
            return UNKNOWN;
        }
      case 0x054C: // Sony
        switch (productId) {
          case 0x0268: // PS3
            return SONY_DUALSHOCK_3;
          case 0x05C4:
          case 0x09CC: // PS4
            return SONY_DUALSHOCK_4;
          case 0x0CE6: // PS5
            return SONY_DUALSENSE;
          default:
            return UNKNOWN;
        }
      default:
        return UNKNOWN;
    }
  }
} // namespace firelight::Input

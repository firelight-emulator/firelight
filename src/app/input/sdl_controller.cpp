#include "sdl_controller.hpp"

namespace firelight::Input {
  SdlController::~SdlController() = default;

  void SdlController::setControllerProfile(const std::shared_ptr<input::ControllerProfile> &profile) {
    m_profile = profile;
  }

  void SdlController::setActiveMapping(const std::shared_ptr<input::InputMapping> &mapping) {
    m_activeMapping = mapping;
  }

  SdlController::SdlController(SDL_GameController *t_controller,
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

  bool SdlController::isButtonPressed(const int platformId, const Input t_button) {
    if (m_activeMapping != nullptr) {
      const auto mapped = m_activeMapping->getMappedInput(t_button);
      if (mapped.has_value()) {
        return std::abs(evaluateMapping(*mapped)) > 0;
      }
      // if (const auto result = mapping->evaluateButtonMapping(m_SDLController, t_button); result.has_value()) {
      //   return result.value();
      // }
    }

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

  int16_t SdlController::getLeftStickXPosition(const int platformId) {
    if (m_activeMapping != nullptr) {
      const auto mapped = m_activeMapping->getMappedInput(LeftStickLeft);
      if (mapped.has_value()) {
        return evaluateMapping(*mapped);
      }
      const auto mapped2 = m_activeMapping->getMappedInput(LeftStickRight);
      if (mapped2.has_value()) {
        return evaluateMapping(*mapped2);
      }
      // if (const auto result = mapping->evaluateAxisMapping(m_SDLController, LeftStickX); result.
      //   has_value()) {
      //   return result.value();
      // }
    }

    return SDL_GameControllerGetAxis(m_SDLController, SDL_CONTROLLER_AXIS_LEFTX);
  }

  int16_t SdlController::getLeftStickYPosition(const int platformId) {
    if (m_activeMapping != nullptr) {
      const auto mapped = m_activeMapping->getMappedInput(LeftStickUp);
      if (mapped.has_value()) {
        return evaluateMapping(*mapped);
      }
      const auto mapped2 = m_activeMapping->getMappedInput(LeftStickDown);
      if (mapped2.has_value()) {
        return evaluateMapping(*mapped2);
      }
    }

    return SDL_GameControllerGetAxis(m_SDLController, SDL_CONTROLLER_AXIS_LEFTY);
  }

  int16_t SdlController::getRightStickXPosition(const int platformId) {
    if (m_activeMapping != nullptr) {
      const auto mapped = m_activeMapping->getMappedInput(RightStickLeft);
      if (mapped.has_value()) {
        return evaluateMapping(*mapped);
      }
      const auto mapped2 = m_activeMapping->getMappedInput(RightStickRight);
      if (mapped2.has_value()) {
        return evaluateMapping(*mapped2);
      }
    }
    return SDL_GameControllerGetAxis(m_SDLController, SDL_CONTROLLER_AXIS_RIGHTX);
  }

  int16_t SdlController::getRightStickYPosition(const int platformId) {
    if (m_activeMapping != nullptr) {
      const auto mapped = m_activeMapping->getMappedInput(RightStickUp);
      if (mapped.has_value()) {
        return evaluateMapping(*mapped);
      }
      const auto mapped2 = m_activeMapping->getMappedInput(RightStickDown);
      if (mapped2.has_value()) {
        return evaluateMapping(*mapped2);
      }
    }
    return SDL_GameControllerGetAxis(m_SDLController, SDL_CONTROLLER_AXIS_RIGHTY);
  }

  int32_t SdlController::getInstanceId() const { return m_SDLJoystickInstanceId; }
  int32_t SdlController::getDeviceIndex() const { return m_SDLJoystickDeviceIndex; }

  std::string SdlController::getName() const {
    return {SDL_GameControllerName(m_SDLController)};
  }

  void SdlController::setPlayerIndex(const int t_newPlayerIndex) {
    SDL_GameControllerSetPlayerIndex(m_SDLController, t_newPlayerIndex);
  }

  int SdlController::getPlayerIndex() const {
    return SDL_GameControllerGetPlayerIndex(m_SDLController);
  }

  // TODO: This isn't quite right, will set the rumble for only one motor at a
  // time
  void SdlController::setStrongRumble(int platformId, const uint16_t t_strength) {
    SDL_JoystickRumble(m_SDLJoystick, 0, t_strength, 2000);
  }

  void SdlController::setWeakRumble(int platformId, const uint16_t t_strength) {
    SDL_JoystickRumble(m_SDLJoystick, t_strength, 0, 2000);
  }

  bool SdlController::isWired() const {
    return SDL_JoystickCurrentPowerLevel(m_SDLJoystick) == SDL_JOYSTICK_POWER_WIRED;
  }

  GamepadType SdlController::getType() const {
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

  int16_t SdlController::evaluateMapping(const Input input) const {
    switch (input) {
      case SouthFace:
        return SDL_GameControllerGetButton(m_SDLController, SDL_CONTROLLER_BUTTON_A);
      case EastFace:
        return SDL_GameControllerGetButton(m_SDLController, SDL_CONTROLLER_BUTTON_B);
      case WestFace:
        return SDL_GameControllerGetButton(m_SDLController, SDL_CONTROLLER_BUTTON_X);
      case NorthFace:
        return SDL_GameControllerGetButton(m_SDLController, SDL_CONTROLLER_BUTTON_Y);
      case DpadLeft:
        return SDL_GameControllerGetButton(m_SDLController, SDL_CONTROLLER_BUTTON_DPAD_LEFT);
      case DpadRight:
        return SDL_GameControllerGetButton(m_SDLController, SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
      case DpadUp:
        return SDL_GameControllerGetButton(m_SDLController, SDL_CONTROLLER_BUTTON_DPAD_UP);
      case DpadDown:
        return SDL_GameControllerGetButton(m_SDLController, SDL_CONTROLLER_BUTTON_DPAD_DOWN);
      case Start:
        return SDL_GameControllerGetButton(m_SDLController, SDL_CONTROLLER_BUTTON_START);
      case Select:
        return SDL_GameControllerGetButton(m_SDLController, SDL_CONTROLLER_BUTTON_BACK);
      case LeftBumper:
        return SDL_GameControllerGetButton(m_SDLController, SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
      case RightBumper:
        return SDL_GameControllerGetButton(m_SDLController, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
      case LeftTrigger:
        return SDL_GameControllerGetAxis(m_SDLController, SDL_CONTROLLER_AXIS_TRIGGERLEFT);
      case RightTrigger:
        return SDL_GameControllerGetAxis(m_SDLController, SDL_CONTROLLER_AXIS_TRIGGERRIGHT);
      case L3:
        return SDL_GameControllerGetButton(m_SDLController, SDL_CONTROLLER_BUTTON_LEFTSTICK);
      case R3:
        return SDL_GameControllerGetButton(m_SDLController, SDL_CONTROLLER_BUTTON_RIGHTSTICK);
      case LeftStickUp: {
        const auto value = SDL_GameControllerGetAxis(m_SDLController, SDL_CONTROLLER_AXIS_LEFTY);
        if (value < -8192) {
          return value;
        }
        return 0;
      }
      case LeftStickDown: {
        const auto value = SDL_GameControllerGetAxis(m_SDLController, SDL_CONTROLLER_AXIS_LEFTY);
        if (value > 8192) {
          return value;
        }
        return 0;
      }
      case LeftStickLeft: {
        const auto value = SDL_GameControllerGetAxis(m_SDLController, SDL_CONTROLLER_AXIS_LEFTX);
        if (value < -8192) {
          return value;
        }
        return 0;
      }
      case LeftStickRight: {
        const auto value = SDL_GameControllerGetAxis(m_SDLController, SDL_CONTROLLER_AXIS_LEFTX);
        if (value > 8192) {
          return value;
        }
        return 0;
      }
      case RightStickUp: {
        const auto value = SDL_GameControllerGetAxis(m_SDLController, SDL_CONTROLLER_AXIS_RIGHTY);
        if (value < -8192) {
          return value;
        }
        return 0;
      }
      case RightStickDown: {
        const auto value = SDL_GameControllerGetAxis(m_SDLController, SDL_CONTROLLER_AXIS_RIGHTY);
        if (value > 8192) {
          return value;
        }
        return 0;
      }
      case RightStickLeft: {
        const auto value = SDL_GameControllerGetAxis(m_SDLController, SDL_CONTROLLER_AXIS_RIGHTX);
        if (value < -8192) {
          return value;
        }
        return 0;
      }
      case RightStickRight: {
        const auto value = SDL_GameControllerGetAxis(m_SDLController, SDL_CONTROLLER_AXIS_RIGHTX);
        if (value > 8192) {
          return value;
        }
        return 0;
      }
      case Unknown:
        break;
    }
    return 0;
  }

  int SdlController::getProfileId() const {
    return m_profile->getId();
  }
} // namespace firelight::Input

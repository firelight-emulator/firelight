#include "sdl_controller.hpp"

#include <bits/stl_algo.h>

namespace firelight::input {
SdlController::~SdlController() = default;

int16_t SdlController::evaluateRawInput(const GamepadInput input) const {
  return evaluateMapping(input);
}

std::shared_ptr<GamepadProfile> SdlController::getProfile() const {
  return m_profile;
}
void SdlController::setProfile(const std::shared_ptr<GamepadProfile> &profile) {
  m_profile = profile;
}

std::vector<Shortcut> SdlController::getToggledShortcuts(GamepadInput input) {
  std::vector<Shortcut> result;
  for (const auto &seq : m_profile->getShortcutMapping()->getMappings()) {
    auto modifiers = seq.second.modifiers;

    auto toggled = true;
    for (const auto &mod : modifiers) {
      if (!evaluateMapping(mod)) {
        toggled = false;
        break;
      }
    }
    if (toggled && seq.second.input == input) {
      result.emplace_back(seq.first);
    }
  }

  return result;
}

SdlController::SdlController(SDL_GameController *t_controller)
    : m_SDLController(t_controller) {
  m_SDLJoystick = SDL_GameControllerGetJoystick(t_controller);
  m_SDLJoystickInstanceId = SDL_JoystickInstanceID(m_SDLJoystick);

  m_deviceName = SDL_JoystickName(m_SDLJoystick);
  m_vendorId = SDL_JoystickGetVendor(m_SDLJoystick);
  m_productId = SDL_JoystickGetProduct(m_SDLJoystick);
  m_productVersion = SDL_JoystickGetProductVersion(m_SDLJoystick);
}

bool SdlController::isButtonPressed(const int platformId, int controllerTypeId,
                                    const Input t_button) {
  auto input = static_cast<GamepadInput>(t_button);

  if (m_profile != nullptr) {
    auto mapping = m_profile->getMappingForPlatformAndController(
        platformId, controllerTypeId);
    if (mapping != nullptr) {
      const auto mapped = mapping->getMappedInput(input);
      if (mapped.has_value()) {
        if (mapped.value() == None) {
          return false;
        }
        return std::abs(evaluateMapping(
                   static_cast<GamepadInput>(mapped.value()))) > 0;
      }
    }

    // if (const auto result = mapping->evaluateButtonMapping(m_SDLController,
    // t_button); result.has_value()) {
    //   return result.value();
    // }
  }

  switch (input) {
  case GamepadInput::NorthFace:
    return SDL_GameControllerGetButton(m_SDLController,
                                       SDL_CONTROLLER_BUTTON_Y);
  case GamepadInput::SouthFace:
    return SDL_GameControllerGetButton(m_SDLController,
                                       SDL_CONTROLLER_BUTTON_A);
  case GamepadInput::EastFace:
    return SDL_GameControllerGetButton(m_SDLController,
                                       SDL_CONTROLLER_BUTTON_B);
  case GamepadInput::WestFace:
    return SDL_GameControllerGetButton(m_SDLController,
                                       SDL_CONTROLLER_BUTTON_X);
  case GamepadInput::DpadUp:
    return SDL_GameControllerGetButton(m_SDLController,
                                       SDL_CONTROLLER_BUTTON_DPAD_UP);
  case GamepadInput::DpadDown:
    return SDL_GameControllerGetButton(m_SDLController,
                                       SDL_CONTROLLER_BUTTON_DPAD_DOWN);
  case GamepadInput::DpadLeft:
    return SDL_GameControllerGetButton(m_SDLController,
                                       SDL_CONTROLLER_BUTTON_DPAD_LEFT);
  case GamepadInput::DpadRight:
    return SDL_GameControllerGetButton(m_SDLController,
                                       SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
  case GamepadInput::Start:
    return SDL_GameControllerGetButton(m_SDLController,
                                       SDL_CONTROLLER_BUTTON_START);
  case GamepadInput::Select:
    return SDL_GameControllerGetButton(m_SDLController,
                                       SDL_CONTROLLER_BUTTON_BACK);
  case GamepadInput::LeftBumper:
    return SDL_GameControllerGetButton(m_SDLController,
                                       SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
  case GamepadInput::RightBumper:
    return SDL_GameControllerGetButton(m_SDLController,
                                       SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
  case GamepadInput::LeftTrigger:
    return SDL_GameControllerGetAxis(m_SDLController,
                                     SDL_CONTROLLER_AXIS_TRIGGERLEFT) > 0;
  case GamepadInput::RightTrigger:
    return SDL_GameControllerGetAxis(m_SDLController,
                                     SDL_CONTROLLER_AXIS_TRIGGERRIGHT) > 0;
  case GamepadInput::L3:
    return SDL_GameControllerGetButton(m_SDLController,
                                       SDL_CONTROLLER_BUTTON_LEFTSTICK);
  case GamepadInput::R3:
    return SDL_GameControllerGetButton(m_SDLController,
                                       SDL_CONTROLLER_BUTTON_RIGHTSTICK);
  default:
    return false;
  }
}

int16_t SdlController::getLeftStickXPosition(const int platformId,
                                             int controllerTypeId) {
  auto rawX =
      SDL_GameControllerGetAxis(m_SDLController, SDL_CONTROLLER_AXIS_LEFTX);
  if (rawX > -8192 && rawX < 8192) {
    rawX = 0;
  }

  if (!m_profile) {
    return rawX;
  }

  const auto mapping = m_profile->getMappingForPlatformAndController(
      platformId, controllerTypeId);
  if (!mapping) {
    return rawX;
  }

  const auto mappedLeft = mapping->getMappedInput(GamepadInput::LeftStickLeft);
  const auto mappedRight =
      mapping->getMappedInput(GamepadInput::LeftStickRight);

  if (!mappedLeft && !mappedRight) {
    return rawX;
  }

  const auto valLeft =
      mappedLeft ? evaluateMapping(static_cast<GamepadInput>(*mappedLeft)) : 0;
  const auto valRight =
      mappedRight ? evaluateMapping(static_cast<GamepadInput>(*mappedRight))
                  : 0;

  return valRight - valLeft;
}

int16_t SdlController::getLeftStickYPosition(const int platformId,
                                             int controllerTypeId) {
  auto rawY =
      SDL_GameControllerGetAxis(m_SDLController, SDL_CONTROLLER_AXIS_LEFTY);
  if (rawY > -8192 && rawY < 8192) {
    rawY = 0;
  }

  if (!m_profile) {
    return rawY;
  }

  const auto mapping = m_profile->getMappingForPlatformAndController(
      platformId, controllerTypeId);
  if (!mapping) {
    return rawY;
  }

  const auto mappedUp = mapping->getMappedInput(GamepadInput::LeftStickUp);
  const auto mappedDown = mapping->getMappedInput(GamepadInput::LeftStickDown);

  if (!mappedUp && !mappedDown) {
    return rawY;
  }

  const auto valUp =
      mappedUp ? evaluateMapping(static_cast<GamepadInput>(*mappedUp)) : 0;
  const auto valDown =
      mappedDown ? evaluateMapping(static_cast<GamepadInput>(*mappedDown)) : 0;

  return valDown - valUp;
}

int16_t SdlController::getRightStickXPosition(const int platformId,
                                              int controllerTypeId) {
  auto rawX =
      SDL_GameControllerGetAxis(m_SDLController, SDL_CONTROLLER_AXIS_RIGHTX);
  if (rawX > -8192 && rawX < 8192) {
    rawX = 0;
  }

  if (!m_profile) {
    return rawX;
  }

  const auto mapping = m_profile->getMappingForPlatformAndController(
      platformId, controllerTypeId);
  if (!mapping) {
    return rawX;
  }

  const auto mappedLeft = mapping->getMappedInput(GamepadInput::RightStickLeft);
  const auto mappedRight =
      mapping->getMappedInput(GamepadInput::RightStickRight);

  if (!mappedLeft && !mappedRight) {
    return rawX;
  }

  const auto valLeft =
      mappedLeft ? evaluateMapping(static_cast<GamepadInput>(*mappedLeft)) : 0;
  const auto valRight =
      mappedRight ? evaluateMapping(static_cast<GamepadInput>(*mappedRight))
                  : 0;

  return valRight - valLeft;
}

int16_t SdlController::getRightStickYPosition(const int platformId,
                                              int controllerTypeId) {
  auto rawY =
      SDL_GameControllerGetAxis(m_SDLController, SDL_CONTROLLER_AXIS_RIGHTY);
  if (rawY > -8192 && rawY < 8192) {
    rawY = 0;
  }
  if (!m_profile) {
    return rawY;
  }

  const auto mapping = m_profile->getMappingForPlatformAndController(
      platformId, controllerTypeId);
  if (!mapping) {
    return rawY;
  }

  const auto mappedUp = mapping->getMappedInput(GamepadInput::RightStickUp);
  const auto mappedDown = mapping->getMappedInput(GamepadInput::RightStickDown);

  if (!mappedUp && !mappedDown) {
    return rawY;
  }

  const auto valUp =
      mappedUp ? evaluateMapping(static_cast<GamepadInput>(*mappedUp)) : 0;
  const auto valDown =
      mappedDown ? evaluateMapping(static_cast<GamepadInput>(*mappedDown)) : 0;

  return valDown - valUp;
}

int32_t SdlController::getInstanceId() const { return m_SDLJoystickInstanceId; }

std::string SdlController::getName() const {
  return {SDL_GameControllerName(m_SDLController)};
}

void SdlController::setPlayerIndex(const int t_newPlayerIndex) {
  m_playerIndex = t_newPlayerIndex;
  SDL_GameControllerSetPlayerIndex(m_SDLController, t_newPlayerIndex);
}

int SdlController::getPlayerIndex() const { return m_playerIndex; }

// TODO: This isn't quite right, will set the rumble for only one motor at a
// time
void SdlController::setStrongRumble(int platformId, const uint16_t t_strength) {
  SDL_JoystickRumble(m_SDLJoystick, 0, t_strength, 2000);
}

void SdlController::setWeakRumble(int platformId, const uint16_t t_strength) {
  SDL_JoystickRumble(m_SDLJoystick, t_strength, 0, 2000);
}

bool SdlController::isWired() const {
  return SDL_JoystickCurrentPowerLevel(m_SDLJoystick) ==
         SDL_JOYSTICK_POWER_WIRED;
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
SDL_GameController *SdlController::getSDLController() const {
  return m_SDLController;
}
DeviceIdentifier SdlController::getDeviceIdentifier() const {
  return DeviceIdentifier{m_deviceName, m_vendorId, m_productId,
                          m_productVersion};
}

int16_t SdlController::evaluateMapping(const GamepadInput input) const {
  switch (input) {
  case GamepadInput::SouthFace:
    return SDL_GameControllerGetButton(m_SDLController,
                                       SDL_CONTROLLER_BUTTON_A) *
           32767;
  case GamepadInput::EastFace:
    return SDL_GameControllerGetButton(m_SDLController,
                                       SDL_CONTROLLER_BUTTON_B) *
           32767;
  case GamepadInput::WestFace:
    return SDL_GameControllerGetButton(m_SDLController,
                                       SDL_CONTROLLER_BUTTON_X) *
           32767;
  case GamepadInput::NorthFace:
    return SDL_GameControllerGetButton(m_SDLController,
                                       SDL_CONTROLLER_BUTTON_Y) *
           32767;
  case GamepadInput::DpadLeft:
    return SDL_GameControllerGetButton(m_SDLController,
                                       SDL_CONTROLLER_BUTTON_DPAD_LEFT) *
           32767;
  case GamepadInput::DpadRight:
    return SDL_GameControllerGetButton(m_SDLController,
                                       SDL_CONTROLLER_BUTTON_DPAD_RIGHT) *
           32767;
  case GamepadInput::DpadUp:
    return SDL_GameControllerGetButton(m_SDLController,
                                       SDL_CONTROLLER_BUTTON_DPAD_UP) *
           32767;
  case GamepadInput::DpadDown:
    return SDL_GameControllerGetButton(m_SDLController,
                                       SDL_CONTROLLER_BUTTON_DPAD_DOWN) *
           32767;
  case GamepadInput::Start:
    return SDL_GameControllerGetButton(m_SDLController,
                                       SDL_CONTROLLER_BUTTON_START) *
           32767;
  case GamepadInput::Select:
    return SDL_GameControllerGetButton(m_SDLController,
                                       SDL_CONTROLLER_BUTTON_BACK) *
           32767;
  case GamepadInput::LeftBumper:
    return SDL_GameControllerGetButton(m_SDLController,
                                       SDL_CONTROLLER_BUTTON_LEFTSHOULDER) *
           32767;
  case GamepadInput::RightBumper:
    return SDL_GameControllerGetButton(m_SDLController,
                                       SDL_CONTROLLER_BUTTON_RIGHTSHOULDER) *
           32767;
  case GamepadInput::LeftTrigger:
    return SDL_GameControllerGetAxis(m_SDLController,
                                     SDL_CONTROLLER_AXIS_TRIGGERLEFT) *
           32767;
  case GamepadInput::RightTrigger:
    return SDL_GameControllerGetAxis(m_SDLController,
                                     SDL_CONTROLLER_AXIS_TRIGGERRIGHT) *
           32767;
  case GamepadInput::L3:
    return SDL_GameControllerGetButton(m_SDLController,
                                       SDL_CONTROLLER_BUTTON_LEFTSTICK) *
           32767;
  case GamepadInput::R3:
    return SDL_GameControllerGetButton(m_SDLController,
                                       SDL_CONTROLLER_BUTTON_RIGHTSTICK) *
           32767;
  case GamepadInput::LeftStickUp: {
    const auto value =
        SDL_GameControllerGetAxis(m_SDLController, SDL_CONTROLLER_AXIS_LEFTY);
    if (value < -8192) {
      return value;
    }
    return 0;
  }
  case GamepadInput::LeftStickDown: {
    const auto value =
        SDL_GameControllerGetAxis(m_SDLController, SDL_CONTROLLER_AXIS_LEFTY);
    if (value > 8192) {
      return value;
    }
    return 0;
  }
  case GamepadInput::LeftStickLeft: {
    const auto value =
        SDL_GameControllerGetAxis(m_SDLController, SDL_CONTROLLER_AXIS_LEFTX);
    if (value < -8192) {
      return value;
    }
    return 0;
  }
  case GamepadInput::LeftStickRight: {
    const auto value =
        SDL_GameControllerGetAxis(m_SDLController, SDL_CONTROLLER_AXIS_LEFTX);
    if (value > 8192) {
      return value;
    }
    return 0;
  }
  case GamepadInput::RightStickUp: {
    const auto value =
        SDL_GameControllerGetAxis(m_SDLController, SDL_CONTROLLER_AXIS_RIGHTY);
    if (value < -8192) {
      return value;
    }
    return 0;
  }
  case GamepadInput::RightStickDown: {
    const auto value =
        SDL_GameControllerGetAxis(m_SDLController, SDL_CONTROLLER_AXIS_RIGHTY);
    if (value > 8192) {
      return value;
    }
    return 0;
  }
  case GamepadInput::RightStickLeft: {
    const auto value =
        SDL_GameControllerGetAxis(m_SDLController, SDL_CONTROLLER_AXIS_RIGHTX);
    if (value < -8192) {
      return value;
    }
    return 0;
  }
  case GamepadInput::RightStickRight: {
    const auto value =
        SDL_GameControllerGetAxis(m_SDLController, SDL_CONTROLLER_AXIS_RIGHTX);
    if (value > 8192) {
      return value;
    }
    return 0;
  }
  case None:
    break;
  }
  return 0;
}
} // namespace firelight::input

#include <firelight/input/sdl_controller.hpp>

#include <algorithm>
#include <chrono>
#include <cmath>

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
  // Drop any latched toggle/turbo state from the previous profile.
  m_togglePrevRaw.clear();
  m_toggleLatch.clear();
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

bool SdlController::evaluateBindingDigital(const Binding &binding) const {
  // All combo modifiers must be held for the binding to be active.
  for (const auto mod : binding.source.modifiers) {
    if (std::abs(evaluateMapping(static_cast<GamepadInput>(mod))) <= 16383) {
      return false;
    }
  }

  const auto code = static_cast<GamepadInput>(binding.source.code);
  if (code == None) {
    return false;
  }
  const auto threshold = static_cast<int>(binding.threshold * 32767.0f);
  return std::abs(evaluateMapping(code)) > threshold;
}

bool SdlController::evaluateBindingWithModes(const GamepadInput target,
                                             const std::size_t index,
                                             const Binding &binding) {
  bool active = evaluateBindingDigital(binding);

  if (binding.toggle) {
    const uint64_t key = (static_cast<uint64_t>(target) << 8) | index;
    const bool previous = m_togglePrevRaw[key];
    if (active && !previous) {
      // Flip the latch on the rising edge of the source.
      m_toggleLatch[key] = !m_toggleLatch[key];
    }
    m_togglePrevRaw[key] = active;
    active = m_toggleLatch[key];
  }

  if (binding.turbo.enabled && active) {
    const double rate = binding.turbo.rateHz > 0.0f ? binding.turbo.rateHz : 10.0;
    const double t = std::chrono::duration<double>(
                         std::chrono::steady_clock::now().time_since_epoch())
                         .count();
    // Square wave: "pressed" for the first half of each autofire cycle.
    return std::fmod(t * rate, 1.0) < 0.5;
  }

  return active;
}

bool SdlController::isButtonPressed(const int platformId, int controllerTypeId,
                                    const Input t_button) {
  auto input = static_cast<GamepadInput>(t_button);

  if (m_profile != nullptr) {
    auto mapping = m_profile->getMappingForPlatformAndController(
        platformId, controllerTypeId);
    if (mapping != nullptr) {
      const auto &bindings = mapping->getBindings(input);
      if (!bindings.empty()) {
        // Evaluate every binding (so toggle/turbo state stays current) and OR
        // the results together.
        bool active = false;
        for (std::size_t i = 0; i < bindings.size(); ++i) {
          if (evaluateBindingWithModes(input, i, bindings[i])) {
            active = true;
          }
        }
        return active;
      }
    }
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
  case GamepadInput::LeftTrigger: {
    const auto settings =
        m_profile ? m_profile->getAnalogSettings(platformId, controllerTypeId)
                  : AnalogSettings{};
    return SDL_GameControllerGetAxis(m_SDLController,
                                     SDL_CONTROLLER_AXIS_TRIGGERLEFT) >
           static_cast<int>(settings.leftTrigger.threshold * 32767.0f);
  }
  case GamepadInput::RightTrigger: {
    const auto settings =
        m_profile ? m_profile->getAnalogSettings(platformId, controllerTypeId)
                  : AnalogSettings{};
    return SDL_GameControllerGetAxis(m_SDLController,
                                     SDL_CONTROLLER_AXIS_TRIGGERRIGHT) >
           static_cast<int>(settings.rightTrigger.threshold * 32767.0f);
  }
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

bool SdlController::isVirtualInputActive(const int platformId,
                                         int controllerTypeId,
                                         const int virtualInput) {
  // Mouse/light-gun buttons are only ever driven by an explicit mapping (a
  // gamepad button bound to e.g. a light-gun trigger). Unmapped => not active;
  // the physical mouse is handled by the pointer provider in the core callback.
  if (m_profile == nullptr) {
    return false;
  }
  const auto mapping =
      m_profile->getMappingForPlatformAndController(platformId, controllerTypeId);
  if (mapping == nullptr) {
    return false;
  }
  const auto input = static_cast<GamepadInput>(virtualInput);
  const auto &bindings = mapping->getBindings(input);
  bool active = false;
  for (std::size_t i = 0; i < bindings.size(); ++i) {
    if (evaluateBindingWithModes(input, i, bindings[i])) {
      active = true;
    }
  }
  return active;
}

int16_t SdlController::getLeftStickXPosition(const int platformId,
                                             int controllerTypeId) {
  const auto settings =
      m_profile ? m_profile->getAnalogSettings(platformId, controllerTypeId)
                : AnalogSettings{};
  const auto rawX =
      SDL_GameControllerGetAxis(m_SDLController, SDL_CONTROLLER_AXIS_LEFTX);

  if (m_profile) {
    const auto mapping = m_profile->getMappingForPlatformAndController(
        platformId, controllerTypeId);
    if (mapping) {
      const auto &left = mapping->getBindings(GamepadInput::LeftStickLeft);
      const auto &right = mapping->getBindings(GamepadInput::LeftStickRight);
      if (!left.empty() || !right.empty()) {
        const auto valLeft =
            left.empty()
                ? 0
                : evaluateMapping(
                      static_cast<GamepadInput>(left.front().source.code));
        const auto valRight =
            right.empty()
                ? 0
                : evaluateMapping(
                      static_cast<GamepadInput>(right.front().source.code));
        return static_cast<int16_t>(valRight - valLeft);
      }
    }
  }

  return applyAxisSettings(rawX, settings.leftStick);
}

int16_t SdlController::getLeftStickYPosition(const int platformId,
                                             int controllerTypeId) {
  const auto settings =
      m_profile ? m_profile->getAnalogSettings(platformId, controllerTypeId)
                : AnalogSettings{};
  const auto rawY =
      SDL_GameControllerGetAxis(m_SDLController, SDL_CONTROLLER_AXIS_LEFTY);

  if (m_profile) {
    const auto mapping = m_profile->getMappingForPlatformAndController(
        platformId, controllerTypeId);
    if (mapping) {
      const auto &up = mapping->getBindings(GamepadInput::LeftStickUp);
      const auto &down = mapping->getBindings(GamepadInput::LeftStickDown);
      if (!up.empty() || !down.empty()) {
        const auto valUp =
            up.empty() ? 0
                       : evaluateMapping(
                             static_cast<GamepadInput>(up.front().source.code));
        const auto valDown =
            down.empty()
                ? 0
                : evaluateMapping(
                      static_cast<GamepadInput>(down.front().source.code));
        return static_cast<int16_t>(valDown - valUp);
      }
    }
  }

  return applyAxisSettings(rawY, settings.leftStick);
}

int16_t SdlController::getRightStickXPosition(const int platformId,
                                              int controllerTypeId) {
  const auto settings =
      m_profile ? m_profile->getAnalogSettings(platformId, controllerTypeId)
                : AnalogSettings{};
  const auto rawX =
      SDL_GameControllerGetAxis(m_SDLController, SDL_CONTROLLER_AXIS_RIGHTX);

  if (m_profile) {
    const auto mapping = m_profile->getMappingForPlatformAndController(
        platformId, controllerTypeId);
    if (mapping) {
      const auto &left = mapping->getBindings(GamepadInput::RightStickLeft);
      const auto &right = mapping->getBindings(GamepadInput::RightStickRight);
      if (!left.empty() || !right.empty()) {
        const auto valLeft =
            left.empty()
                ? 0
                : evaluateMapping(
                      static_cast<GamepadInput>(left.front().source.code));
        const auto valRight =
            right.empty()
                ? 0
                : evaluateMapping(
                      static_cast<GamepadInput>(right.front().source.code));
        return static_cast<int16_t>(valRight - valLeft);
      }
    }
  }

  return applyAxisSettings(rawX, settings.rightStick);
}

int16_t SdlController::getRightStickYPosition(const int platformId,
                                              int controllerTypeId) {
  const auto settings =
      m_profile ? m_profile->getAnalogSettings(platformId, controllerTypeId)
                : AnalogSettings{};
  const auto rawY =
      SDL_GameControllerGetAxis(m_SDLController, SDL_CONTROLLER_AXIS_RIGHTY);

  if (m_profile) {
    const auto mapping = m_profile->getMappingForPlatformAndController(
        platformId, controllerTypeId);
    if (mapping) {
      const auto &up = mapping->getBindings(GamepadInput::RightStickUp);
      const auto &down = mapping->getBindings(GamepadInput::RightStickDown);
      if (!up.empty() || !down.empty()) {
        const auto valUp =
            up.empty() ? 0
                       : evaluateMapping(
                             static_cast<GamepadInput>(up.front().source.code));
        const auto valDown =
            down.empty()
                ? 0
                : evaluateMapping(
                      static_cast<GamepadInput>(down.front().source.code));
        return static_cast<int16_t>(valDown - valUp);
      }
    }
  }

  return applyAxisSettings(rawY, settings.rightStick);
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
  case 0x054C:
    switch (productId) {
    case 0x0268:
      return SONY_DUALSHOCK_3;
    case 0x05C4:
    case 0x09CC:
      return SONY_DUALSHOCK_4;
    case 0x0CE6:
      return SONY_DUALSENSE;
    default:
      return UNKNOWN;
    }
  default:
    return UNKNOWN;
  }
}

DeviceType SdlController::getDeviceType() const { return DeviceType::Gamepad; }

DeviceIdentifier SdlController::getDeviceIdentifier() const {
  return DeviceIdentifier{
      .deviceName = m_deviceName,
      .type = DeviceType::Gamepad,
      .vendorId = m_vendorId,
      .productId = m_productId,
      .productVersion = m_productVersion,
  };
}
bool SdlController::disconnect() {
  SDL_GameControllerClose(m_SDLController);
  return true;
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
  case Home:
    break;
  }
  return 0;
}
} // namespace firelight::input

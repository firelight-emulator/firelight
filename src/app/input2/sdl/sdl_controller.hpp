#pragma once

#include <memory>

#include <SDL_gamecontroller.h>
#include <string>

#include "input/gamepad_type.hpp"
#include "input/profiles/shortcut_mapping.hpp"
#include "input2/device_identifier.hpp"
#include "input2/gamepad.hpp"

namespace firelight::input {
class SdlController : public IGamepad {
public:
  SdlController() = default;

  SdlController(SDL_GameController *t_controller);

  ~SdlController() override;

  int16_t evaluateRawInput(const GamepadInput input) const override;

  [[nodiscard]] std::shared_ptr<GamepadProfile> getProfile() const override;

  void setProfile(const std::shared_ptr<GamepadProfile> &profile) override;

  std::vector<Shortcut> getToggledShortcuts(GamepadInput input) override;

  bool isButtonPressed(int platformId, int controllerTypeId,
                       Input t_button) override;

  int16_t getLeftStickXPosition(int platformId, int controllerTypeId) override;

  int16_t getLeftStickYPosition(int platformId, int controllerTypeId) override;

  int16_t getRightStickXPosition(int platformId, int controllerTypeId) override;

  int16_t getRightStickYPosition(int platformId, int controllerTypeId) override;

  [[nodiscard]] int32_t getInstanceId() const override;

  [[nodiscard]] std::string getName() const override;

  void setPlayerIndex(int t_newPlayerIndex) override;

  [[nodiscard]] int getPlayerIndex() const override;

  void setStrongRumble(int platformId, uint16_t t_strength) override;

  void setWeakRumble(int platformId, uint16_t t_strength) override;

  [[nodiscard]] bool isWired() const override;

  [[nodiscard]] GamepadType getType() const override;

  [[nodiscard]] DeviceIdentifier getDeviceIdentifier() const override;

  bool disconnect() override;

private:
  [[nodiscard]] int16_t evaluateMapping(GamepadInput input) const;
  std::shared_ptr<GamepadProfile> m_profile = nullptr;

  SDL_GameController *m_SDLController = nullptr;
  SDL_Joystick *m_SDLJoystick = nullptr;
  int32_t m_SDLJoystickInstanceId = -1;

  int m_playerIndex = -1;

  std::string m_deviceName;
  int m_vendorId = 0;
  int m_productId = 0;
  int m_productVersion = 0;
};
} // namespace firelight::input

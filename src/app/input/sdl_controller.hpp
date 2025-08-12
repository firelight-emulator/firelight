#pragma once

#include <memory>

#include "firelight/libretro/retropad.hpp"
#include <SDL_gamecontroller.h>
#include <string>

#include "gamepad.hpp"
#include "gamepad_type.hpp"
#include "profiles/controller_profile.hpp"

namespace firelight::input {
class SdlController : public IGamepad {
public:
  SdlController() = default;

  SdlController(SDL_GameController *t_controller, int32_t t_joystickIndex);

  ~SdlController() override;

  void setControllerProfile(const std::shared_ptr<ControllerProfile> &profile);

  void setActiveMapping(const std::shared_ptr<InputMapping> &mapping) override;

  std::vector<Shortcut> getToggledShortcuts(int button) override;

  bool isButtonPressed(int platformId, Input t_button) override;

  int16_t getLeftStickXPosition(int platformId) override;

  int16_t getLeftStickYPosition(int platformId) override;

  int16_t getRightStickXPosition(int platformId) override;

  int16_t getRightStickYPosition(int platformId) override;

  [[nodiscard]] int32_t getInstanceId() const;

  [[nodiscard]] int32_t getDeviceIndex() const;

  [[nodiscard]] std::string getName() const override;

  void setPlayerIndex(int t_newPlayerIndex) override;

  [[nodiscard]] int getPlayerIndex() const override;

  void setStrongRumble(int platformId, uint16_t t_strength) override;

  void setWeakRumble(int platformId, uint16_t t_strength) override;

  [[nodiscard]] bool isWired() const override;

  [[nodiscard]] GamepadType getType() const override;

  int getProfileId() const override;

protected:
  std::shared_ptr<InputMapping> m_activeMapping = nullptr;

private:
  [[nodiscard]] int16_t evaluateMapping(Input input) const;
  std::shared_ptr<ControllerProfile> m_profile = nullptr;

  SDL_GameController *m_SDLController = nullptr;
  SDL_Joystick *m_SDLJoystick = nullptr;
  int32_t m_SDLJoystickDeviceIndex = -1;
  int32_t m_SDLJoystickInstanceId = -1;
};
} // namespace firelight::input

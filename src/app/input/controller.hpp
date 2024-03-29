#pragma once

#include "firelight/libretro/retropad.hpp"
#include <SDL_gamecontroller.h>
#include <string>

namespace firelight::Input {

class Controller final : public libretro::IRetroPad {
public:
  Controller(SDL_GameController *t_controller, int32_t t_joystickIndex);
  ~Controller() override;
  bool isButtonPressed(Button t_button) override;
  int16_t getLeftStickXPosition() override;
  int16_t getLeftStickYPosition() override;
  int16_t getRightStickXPosition() override;
  int16_t getRightStickYPosition() override;

  [[nodiscard]] int32_t getInstanceId() const;
  [[nodiscard]] int32_t getDeviceIndex() const;

  [[nodiscard]] std::string getControllerName() const;
  void setPlayerIndex(int t_newPlayerIndex) const;
  [[nodiscard]] int getPlayerIndex() const;

private:
  SDL_GameController *m_SDLController = nullptr;
  int32_t m_SDLJoystickDeviceIndex = -1;
  int32_t m_SDLJoystickInstanceId = -1;
};

} // namespace firelight::Input

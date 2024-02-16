//
// Created by alexs on 1/6/2024.
//

#ifndef CONTROLLER_HPP
#define CONTROLLER_HPP
#include "../libretro/retropad.hpp"

#include <SDL_gamecontroller.h>

namespace Firelight::Input {

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

private:
  int m_playerNumber = -1;
  SDL_GameController *m_SDLController = nullptr;
  int32_t m_SDLJoystickDeviceIndex = -1;
  int32_t m_SDLJoystickInstanceId = -1;
};

} // namespace Firelight::Input

#endif // CONTROLLER_HPP

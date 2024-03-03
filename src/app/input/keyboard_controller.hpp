//
// Created by alexs on 2/15/2024.
//

#ifndef KEYBOARD_CONTROLLER_HPP
#define KEYBOARD_CONTROLLER_HPP

#include "../libretro/retropad.hpp"

namespace firelight::Input {

class KeyboardController final : public libretro::IRetroPad {
public:
  bool isButtonPressed(Button t_button) override;
  int16_t getLeftStickXPosition() override;
  int16_t getLeftStickYPosition() override;
  int16_t getRightStickXPosition() override;
  int16_t getRightStickYPosition() override;
};

} // namespace Firelight::Input

#endif // KEYBOARD_CONTROLLER_HPP

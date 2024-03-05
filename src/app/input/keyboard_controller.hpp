#pragma once

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

} // namespace firelight::Input

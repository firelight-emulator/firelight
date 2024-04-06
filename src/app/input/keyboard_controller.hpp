#pragma once

#include "firelight/libretro/retropad.hpp"

namespace firelight::Input {

class KeyboardController final : public libretro::IRetroPad {
public:
  void setStrongRumble(uint16_t t_strength) override;
  void setWeakRumble(uint16_t t_strength) override;
  bool isButtonPressed(Button t_button) override;
  int16_t getLeftStickXPosition() override;
  int16_t getLeftStickYPosition() override;
  int16_t getRightStickXPosition() override;
  int16_t getRightStickYPosition() override;
};

} // namespace firelight::Input

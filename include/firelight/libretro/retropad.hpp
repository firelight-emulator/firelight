#pragma once

#include "libretro/libretro.h"
#include <cstdint>

namespace firelight::libretro {

class IRetroPad {
public:
  enum Button {
    SouthFace = RETRO_DEVICE_ID_JOYPAD_B,
    EastFace = RETRO_DEVICE_ID_JOYPAD_A,
    WestFace = RETRO_DEVICE_ID_JOYPAD_Y,
    NorthFace = RETRO_DEVICE_ID_JOYPAD_X,
    DpadLeft = RETRO_DEVICE_ID_JOYPAD_LEFT,
    DpadRight = RETRO_DEVICE_ID_JOYPAD_RIGHT,
    DpadUp = RETRO_DEVICE_ID_JOYPAD_UP,
    DpadDown = RETRO_DEVICE_ID_JOYPAD_DOWN,
    Start = RETRO_DEVICE_ID_JOYPAD_START,
    Select = RETRO_DEVICE_ID_JOYPAD_SELECT,
    LeftBumper = RETRO_DEVICE_ID_JOYPAD_L,
    RightBumper = RETRO_DEVICE_ID_JOYPAD_R,
    LeftTrigger = RETRO_DEVICE_ID_JOYPAD_L2,
    RightTrigger = RETRO_DEVICE_ID_JOYPAD_R2,
    L3 = RETRO_DEVICE_ID_JOYPAD_L3,
    R3 = RETRO_DEVICE_ID_JOYPAD_R3
  };

  virtual ~IRetroPad() = default;
  virtual bool isButtonPressed(Button t_button) = 0;
  virtual int16_t getLeftStickXPosition() = 0;
  virtual int16_t getLeftStickYPosition() = 0;
  virtual int16_t getRightStickXPosition() = 0;
  virtual int16_t getRightStickYPosition() = 0;
  virtual void setStrongRumble(uint16_t t_strength) = 0;
  virtual void setWeakRumble(uint16_t t_strength) = 0;
};

} // namespace firelight::libretro

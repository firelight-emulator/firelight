#pragma once

#include "libretro/libretro.h"
#include <cstdint>

namespace firelight::libretro {
class IRetroPad {
public:
  enum Input {
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
    R3 = RETRO_DEVICE_ID_JOYPAD_R3,
    LeftStickUp = RETRO_DEVICE_ID_JOYPAD_MASK | 0,
    LeftStickDown = RETRO_DEVICE_ID_JOYPAD_MASK | 1,
    LeftStickLeft = RETRO_DEVICE_ID_JOYPAD_MASK | 2,
    LeftStickRight = RETRO_DEVICE_ID_JOYPAD_MASK | 3,
    RightStickUp = RETRO_DEVICE_ID_JOYPAD_MASK | 4,
    RightStickDown = RETRO_DEVICE_ID_JOYPAD_MASK | 5,
    RightStickLeft = RETRO_DEVICE_ID_JOYPAD_MASK | 6,
    RightStickRight = RETRO_DEVICE_ID_JOYPAD_MASK | 7,
    Unknown = RETRO_DEVICE_ID_JOYPAD_MASK | 8
  };

  enum Axis {
    LeftStickX = RETRO_DEVICE_ID_JOYPAD_MASK | 0,
    LeftStickY = RETRO_DEVICE_ID_JOYPAD_MASK | 1,
    RightStickX = RETRO_DEVICE_ID_JOYPAD_MASK | 2,
    RightStickY = RETRO_DEVICE_ID_JOYPAD_MASK | 3
  };

  virtual ~IRetroPad() = default;

  virtual bool isButtonPressed(int platformId, int controllerTypeId,
                               Input t_button) = 0;

  virtual int16_t getLeftStickXPosition(int platformId,
                                        int controllerTypeId) = 0;

  virtual int16_t getLeftStickYPosition(int platformId,
                                        int controllerTypeId) = 0;

  virtual int16_t getRightStickXPosition(int platformId,
                                         int controllerTypeId) = 0;

  virtual int16_t getRightStickYPosition(int platformId,
                                         int controllerTypeId) = 0;

  virtual void setStrongRumble(int platformId, uint16_t t_strength) = 0;

  virtual void setWeakRumble(int platformId, uint16_t t_strength) = 0;
};
} // namespace firelight::libretro

#pragma once

namespace firelight::input {

enum GamepadInput {
  SouthFace = 0,         // RETRO_DEVICE_ID_JOYPAD_B
  WestFace = 1,          // RETRO_DEVICE_ID_JOYPAD_Y
  Select = 2,            // RETRO_DEVICE_ID_JOYPAD_SELECT
  Start = 3,             // RETRO_DEVICE_ID_JOYPAD_START
  DpadUp = 4,            // RETRO_DEVICE_ID_JOYPAD_UP
  DpadDown = 5,          // RETRO_DEVICE_ID_JOYPAD_DOWN
  DpadLeft = 6,          // RETRO_DEVICE_ID_JOYPAD_LEFT
  DpadRight = 7,         // RETRO_DEVICE_ID_JOYPAD_RIGHT
  EastFace = 8,          // RETRO_DEVICE_ID_JOYPAD_A
  NorthFace = 9,         // RETRO_DEVICE_ID_JOYPAD_X
  LeftBumper = 10,       // RETRO_DEVICE_ID_JOYPAD_L
  RightBumper = 11,      // RETRO_DEVICE_ID_JOYPAD_R
  LeftTrigger = 12,      // RETRO_DEVICE_ID_JOYPAD_L2
  RightTrigger = 13,     // RETRO_DEVICE_ID_JOYPAD_R2
  L3 = 14,               // RETRO_DEVICE_ID_JOYPAD_L3
  R3 = 15,               // RETRO_DEVICE_ID_JOYPAD_R3
  LeftStickUp = 256,     // RETRO_DEVICE_ID_JOYPAD_MASK | 0
  LeftStickDown = 257,   // RETRO_DEVICE_ID_JOYPAD_MASK | 1
  LeftStickLeft = 258,   // RETRO_DEVICE_ID_JOYPAD_MASK | 2
  LeftStickRight = 259,  // RETRO_DEVICE_ID_JOYPAD_MASK | 3
  RightStickUp = 260,    // RETRO_DEVICE_ID_JOYPAD_MASK | 4
  RightStickDown = 261,  // RETRO_DEVICE_ID_JOYPAD_MASK | 5
  RightStickLeft = 262,  // RETRO_DEVICE_ID_JOYPAD_MASK | 6
  RightStickRight = 263, // RETRO_DEVICE_ID_JOYPAD_MASK | 7
  None = 264,            // RETRO_DEVICE_ID_JOYPAD_MASK | 8
  Home = 265,
};

}
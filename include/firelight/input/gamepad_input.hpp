#pragma once

namespace firelight::input {
  // Namespacing masks for non-JOYPAD device inputs. The low byte of a masked
  // value is the raw RETRO_DEVICE_ID_* for that device, so the runtime can
  // recover it with `input & 0xFF` and detect the class with these masks.
  constexpr int MOUSE_INPUT_MASK = 1 << 9; // 512
  constexpr int LIGHTGUN_INPUT_MASK = 1 << 10; // 1024

  enum GamepadInput {
    SouthFace = 0, // RETRO_DEVICE_ID_JOYPAD_B
    WestFace = 1, // RETRO_DEVICE_ID_JOYPAD_Y
    Select = 2, // RETRO_DEVICE_ID_JOYPAD_SELECT
    Start = 3, // RETRO_DEVICE_ID_JOYPAD_START
    DpadUp = 4, // RETRO_DEVICE_ID_JOYPAD_UP
    DpadDown = 5, // RETRO_DEVICE_ID_JOYPAD_DOWN
    DpadLeft = 6, // RETRO_DEVICE_ID_JOYPAD_LEFT
    DpadRight = 7, // RETRO_DEVICE_ID_JOYPAD_RIGHT
    EastFace = 8, // RETRO_DEVICE_ID_JOYPAD_A
    NorthFace = 9, // RETRO_DEVICE_ID_JOYPAD_X
    LeftBumper = 10, // RETRO_DEVICE_ID_JOYPAD_L
    RightBumper = 11, // RETRO_DEVICE_ID_JOYPAD_R
    LeftTrigger = 12, // RETRO_DEVICE_ID_JOYPAD_L2
    RightTrigger = 13, // RETRO_DEVICE_ID_JOYPAD_R2
    L3 = 14, // RETRO_DEVICE_ID_JOYPAD_L3
    R3 = 15, // RETRO_DEVICE_ID_JOYPAD_R3
    LeftStickUp = 256, // RETRO_DEVICE_ID_JOYPAD_MASK | 0
    LeftStickDown = 257, // RETRO_DEVICE_ID_JOYPAD_MASK | 1
    LeftStickLeft = 258, // RETRO_DEVICE_ID_JOYPAD_MASK | 2
    LeftStickRight = 259, // RETRO_DEVICE_ID_JOYPAD_MASK | 3
    RightStickUp = 260, // RETRO_DEVICE_ID_JOYPAD_MASK | 4
    RightStickDown = 261, // RETRO_DEVICE_ID_JOYPAD_MASK | 5
    RightStickLeft = 262, // RETRO_DEVICE_ID_JOYPAD_MASK | 6
    RightStickRight = 263, // RETRO_DEVICE_ID_JOYPAD_MASK | 7
    None = 264, // RETRO_DEVICE_ID_JOYPAD_MASK | 8
    Home = 265,

    // Mouse device inputs (RETRO_DEVICE_MOUSE). Low byte = RETRO_DEVICE_ID_MOUSE_*.
    MouseX = MOUSE_INPUT_MASK | 0, // 512  relative X
    MouseY = MOUSE_INPUT_MASK | 1, // 513  relative Y
    MouseLeft = MOUSE_INPUT_MASK | 2, // 514
    MouseRight = MOUSE_INPUT_MASK | 3, // 515
    MouseWheelUp = MOUSE_INPUT_MASK | 4, // 516
    MouseWheelDown = MOUSE_INPUT_MASK | 5, // 517
    MouseMiddle = MOUSE_INPUT_MASK | 6, // 518
    MouseButton4 = MOUSE_INPUT_MASK | 9, // 521
    MouseButton5 = MOUSE_INPUT_MASK | 10, // 522

    // Light gun inputs (RETRO_DEVICE_LIGHTGUN). Low byte = RETRO_DEVICE_ID_LIGHTGUN_*.
    // Aim (SCREEN_X/Y) is sourced from the pointer, not a bindable input.
    LightgunTrigger = LIGHTGUN_INPUT_MASK | 2, // 1026
    LightgunAuxA = LIGHTGUN_INPUT_MASK | 3, // 1027
    LightgunAuxB = LIGHTGUN_INPUT_MASK | 4, // 1028
    LightgunStart = LIGHTGUN_INPUT_MASK | 6, // 1030
    LightgunSelect = LIGHTGUN_INPUT_MASK | 7, // 1031
    LightgunAuxC = LIGHTGUN_INPUT_MASK | 8, // 1032
    LightgunDpadUp = LIGHTGUN_INPUT_MASK | 9, // 1033
    LightgunDpadDown = LIGHTGUN_INPUT_MASK | 10, // 1034
    LightgunDpadLeft = LIGHTGUN_INPUT_MASK | 11, // 1035
    LightgunDpadRight = LIGHTGUN_INPUT_MASK | 12, // 1036
    LightgunReload = LIGHTGUN_INPUT_MASK | 16, // 1040
  };

  // The device class a GamepadInput belongs to, derived from its namespace mask.
  // Values double as the input-mapping key (controllerType): Joypad == 1 keeps
  // existing persisted joypad mappings valid.
  enum class GamepadInputClass { Joypad = 1, Mouse = 2, Lightgun = 3 };

  constexpr GamepadInputClass classOf(const GamepadInput input) {
    if (input & LIGHTGUN_INPUT_MASK) {
      return GamepadInputClass::Lightgun;
    }
    if (input & MOUSE_INPUT_MASK) {
      return GamepadInputClass::Mouse;
    }
    return GamepadInputClass::Joypad;
  }

  // The raw RETRO_DEVICE_ID_* for a mouse/lightgun input (low byte).
  constexpr unsigned retroDeviceId(const GamepadInput input) {
    return static_cast<unsigned>(input) & 0xFFu;
  }

  // The default *physical* gamepad button a console input binds to when the user
  // hasn't set an explicit binding. For most inputs this is the identity (the
  // input's own value is already a physical button, e.g. NES A == EastFace); the
  // abstract light-gun / mouse buttons default to sensible face buttons / D-pad so
  // a gamepad can fire them out of the box (aim comes from the stick separately).
  constexpr GamepadInput defaultPhysicalBinding(const GamepadInput input) {
    switch (input) {
      case LightgunTrigger:
      case MouseLeft:
        return SouthFace;
      case LightgunReload:
      case MouseRight:
        return EastFace;
      case LightgunAuxA:
      case MouseMiddle:
        return WestFace;
      case LightgunAuxB:
        return NorthFace;
      case LightgunStart:
        return Start;
      case LightgunSelect:
        return Select;
      case LightgunDpadUp:
        return DpadUp;
      case LightgunDpadDown:
        return DpadDown;
      case LightgunDpadLeft:
        return DpadLeft;
      case LightgunDpadRight:
        return DpadRight;
      default:
        return input; // identity (joypad inputs are already physical buttons)
    }
  }
}

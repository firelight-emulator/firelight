#pragma once

#include <firelight/libretro/retropad.hpp>

#include <array>
#include <cstddef>
#include <cstdint>

namespace firelight::input {

// One port's joypad input for a single frame. Captured once per frame (see
// Core::pollInput) so the libretro core reads stable input during retro_run(),
// and so a frame's input is a self-contained, serializable value — the record a
// replay or future netplay layer would transmit. `buttons` is a bitmask indexed
// by the RETRO_DEVICE_ID_JOYPAD_* id (0..15); analog axes are in the ±32767
// range. (Pointer/mouse/light-gun input is not part of this record yet; that
// path still reads live and already snapshots its per-frame mouse delta.)
struct InputFrame {
  uint16_t buttons = 0;
  int16_t leftStickX = 0;
  int16_t leftStickY = 0;
  int16_t rightStickX = 0;
  int16_t rightStickY = 0;

  [[nodiscard]] bool button(unsigned id) const {
    return id < 16 && ((buttons >> id) & 1u) != 0;
  }
  void setButton(unsigned id, bool pressed) {
    if (id >= 16) {
      return;
    }
    const auto bit = static_cast<uint16_t>(1u << id);
    buttons = pressed ? static_cast<uint16_t>(buttons | bit)
                      : static_cast<uint16_t>(buttons & ~bit);
  }

  bool operator==(const InputFrame &) const = default;

  // Fixed-size little-endian wire encoding: 2 (buttons) + 4*2 (axes) = 10 bytes.
  static constexpr std::size_t SERIALIZED_SIZE = 10;

  [[nodiscard]] std::array<uint8_t, SERIALIZED_SIZE> serialize() const {
    std::array<uint8_t, SERIALIZED_SIZE> out{};
    const auto put16 = [&](std::size_t off, uint16_t v) {
      out[off] = static_cast<uint8_t>(v & 0xFF);
      out[off + 1] = static_cast<uint8_t>((v >> 8) & 0xFF);
    };
    put16(0, buttons);
    put16(2, static_cast<uint16_t>(leftStickX));
    put16(4, static_cast<uint16_t>(leftStickY));
    put16(6, static_cast<uint16_t>(rightStickX));
    put16(8, static_cast<uint16_t>(rightStickY));
    return out;
  }

  [[nodiscard]] static InputFrame
  deserialize(const std::array<uint8_t, SERIALIZED_SIZE> &data) {
    const auto get16 = [&](std::size_t off) -> uint16_t {
      return static_cast<uint16_t>(
          data[off] | (static_cast<uint16_t>(data[off + 1]) << 8));
    };
    InputFrame f;
    f.buttons = get16(0);
    f.leftStickX = static_cast<int16_t>(get16(2));
    f.leftStickY = static_cast<int16_t>(get16(4));
    f.rightStickX = static_cast<int16_t>(get16(6));
    f.rightStickY = static_cast<int16_t>(get16(8));
    return f;
  }
};

// Samples a controller's joypad state (all 16 buttons + both analog sticks) into
// an InputFrame — the once-per-frame snapshot. `platformId`/`controllerTypeId`
// match the values the libretro input callback queries the pad with.
[[nodiscard]] inline InputFrame captureJoypadFrame(libretro::IRetroPad &pad,
                                                   int platformId,
                                                   int controllerTypeId) {
  InputFrame f;
  for (unsigned id = 0; id < 16; ++id) {
    f.setButton(id,
                pad.isButtonPressed(
                    platformId, controllerTypeId,
                    static_cast<libretro::IRetroPad::Input>(id)));
  }
  f.leftStickX = pad.getLeftStickXPosition(platformId, controllerTypeId);
  f.leftStickY = pad.getLeftStickYPosition(platformId, controllerTypeId);
  f.rightStickX = pad.getRightStickXPosition(platformId, controllerTypeId);
  f.rightStickY = pad.getRightStickYPosition(platformId, controllerTypeId);
  return f;
}

} // namespace firelight::input

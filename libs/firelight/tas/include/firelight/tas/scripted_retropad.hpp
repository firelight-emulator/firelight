#pragma once

#include <firelight/input/input_frame.hpp>
#include <firelight/libretro/retropad.hpp>

#include <cstdint>

namespace firelight::tas {

// An IRetroPad whose state is a fixed InputFrame — the input a movie dictates for
// one port on one frame. Read-only from the core's perspective: isButtonPressed()
// and the analog getters reflect the frame set via setFrame(); rumble is discarded,
// because a deterministic replay must never depend on host feedback.
//
// The libretro Input enum's face/dpad/system/shoulder ids are exactly the
// RETRO_DEVICE_ID_JOYPAD_* ids 0..15, so a button read is a direct bitmask lookup.
// Stick-direction enum values (RETRO_DEVICE_ID_JOYPAD_MASK | n) fall outside 0..15
// and read as unpressed — the sticks are supplied through the analog getters, which
// is exactly how firelight::input::captureJoypadFrame() samples a pad. Feeding a
// ScriptedRetroPad(frame) back through captureJoypadFrame() therefore reproduces
// `frame` bit-for-bit, which is the invariant the movie playback path relies on.
class ScriptedRetroPad final : public libretro::IRetroPad {
public:
  ScriptedRetroPad() = default;
  explicit ScriptedRetroPad(const input::InputFrame &frame) : m_frame(frame) {}

  void setFrame(const input::InputFrame &frame) { m_frame = frame; }
  [[nodiscard]] const input::InputFrame &frame() const { return m_frame; }

  bool isButtonPressed(int /*platformId*/, int /*controllerTypeId*/,
                       Input t_button) override {
    return m_frame.button(static_cast<unsigned>(t_button));
  }

  int16_t getLeftStickXPosition(int, int) override { return m_frame.leftStickX; }
  int16_t getLeftStickYPosition(int, int) override { return m_frame.leftStickY; }
  int16_t getRightStickXPosition(int, int) override { return m_frame.rightStickX; }
  int16_t getRightStickYPosition(int, int) override { return m_frame.rightStickY; }

  void setStrongRumble(int, uint16_t) override {}
  void setWeakRumble(int, uint16_t) override {}

private:
  input::InputFrame m_frame{};
};

} // namespace firelight::tas

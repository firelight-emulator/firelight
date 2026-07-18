#pragma once

#include <cstdint>
#include <utility>

namespace firelight::libretro {

// TODO
// Per-frame cursor delta (in ±32767 pointer units) for an analog-stick
// deflection under "velocity glide": the cursor moves each frame at a speed
// proportional to how far the stick is pushed, and holds still when centered
// `stickAxis` is the raw stick position (±32767); `stepPerFrame` is the fraction
// of the full range travelled per frame at full deflection (e.g. 0.025). Pure +
// header-only so it is unit-testable without any device
[[nodiscard]] constexpr int cursorGlideDelta(const int16_t stickAxis,
                                             const double stepPerFrame) {
  return static_cast<int>(stickAxis * stepPerFrame);
}
  // TODO
  // Supplies pointer/mouse/light-gun input to a running core. The absolute
  // position + `isPressed` back the DS touch pointer and the light-gun aim +
  // fire; the mouse extensions back RETRO_DEVICE_MOUSE and the light-gun's
  // secondary buttons. All extension methods default to no-ops so pointer-only
  // implementations (and test doubles) need not override them, and so a real
  // light-gun device can later back the same interface
  class IPointerInputProvider {
  public:
    virtual ~IPointerInputProvider() = default;

    // Absolute position, each axis normalized to the ±32767 range (viewport
    // center = 0). Used for the DS pointer and light-gun aim (SCREEN_X/Y)
    [[nodiscard]] virtual std::pair<int16_t, int16_t> getPointerPosition() const = 0;

    // The primary button (left mouse) — DS touch / light-gun trigger
    [[nodiscard]] virtual bool isPressed() const = 0;

    // TODO
    // Relative motion accumulated since the previous call, in mouse units
    // Consumed on read (implementations reset their accumulator); polled once
    // per frame so a single value serves both the X and Y queries
    [[nodiscard]] virtual std::pair<int16_t, int16_t> getRelativeMotion() {
      return {0, 0};
    }

    // State of a physical mouse button (a RETRO_DEVICE_ID_MOUSE_* button id)
    [[nodiscard]] virtual bool isMouseButtonPressed(int /*retroMouseButtonId*/) const {
      return false;
    }

    // True when the pointer is outside the game viewport (light-gun off-screen)
    [[nodiscard]] virtual bool isPointerOffscreen() const { return false; }

    // TODO
    // Moves the shared cursor by a delta (in ±32767 pointer units) and clears
    // the off-screen flag — used to drive the mouse/light-gun aim from a gamepad
    // analog stick. The mouse writes the same cursor absolutely, so whichever
    // moved last wins. Default no-op for pointer-only implementations
    virtual void nudgeCursor(int /*dx*/, int /*dy*/) {}
  };
} // namespace firelight::libretro

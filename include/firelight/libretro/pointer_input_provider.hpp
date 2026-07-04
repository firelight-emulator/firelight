#pragma once

#include <cstdint>
#include <utility>

namespace firelight::libretro {
  // Supplies pointer/mouse/light-gun input to a running core. The absolute
  // position + `isPressed` back the DS touch pointer and the light-gun aim +
  // fire; the mouse extensions back RETRO_DEVICE_MOUSE and the light-gun's
  // secondary buttons. All extension methods default to no-ops so pointer-only
  // implementations (and test doubles) need not override them, and so a real
  // light-gun device can later back the same interface.
  class IPointerInputProvider {
  public:
    virtual ~IPointerInputProvider() = default;

    // Absolute position, each axis normalized to the ±32767 range (viewport
    // center = 0). Used for the DS pointer and light-gun aim (SCREEN_X/Y).
    [[nodiscard]] virtual std::pair<int16_t, int16_t> getPointerPosition() const = 0;

    // The primary button (left mouse) — DS touch / light-gun trigger.
    [[nodiscard]] virtual bool isPressed() const = 0;

    // Relative motion accumulated since the previous call, in mouse units.
    // Consumed on read (implementations reset their accumulator); polled once
    // per frame so a single value serves both the X and Y queries.
    [[nodiscard]] virtual std::pair<int16_t, int16_t> getRelativeMotion() {
      return {0, 0};
    }

    // State of a physical mouse button (a RETRO_DEVICE_ID_MOUSE_* button id).
    [[nodiscard]] virtual bool isMouseButtonPressed(int /*retroMouseButtonId*/) const {
      return false;
    }

    // True when the pointer is outside the game viewport (light-gun off-screen).
    [[nodiscard]] virtual bool isPointerOffscreen() const { return false; }
  };
} // namespace firelight::libretro

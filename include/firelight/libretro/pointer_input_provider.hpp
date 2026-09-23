#pragma once

#include <cstdint>
#include <utility>

namespace firelight::libretro {

// Per-frame cursor delta (+/-32767) for an analog-stick
[[nodiscard]] constexpr int cursorGlideDelta(const int16_t stickAxis, const double stepPerFrame) {
  return static_cast<int>(stickAxis * stepPerFrame);
}

class IPointerInputProvider {
public:
  virtual ~IPointerInputProvider() = default;

  // Get x and y of absolute position
  [[nodiscard]] virtual std::pair<int16_t, int16_t> getPointerPosition() const = 0;

  // The primary button for DS touch / light-gun trigger
  [[nodiscard]] virtual bool isPressed() const = 0;

  // Relative motion accumulated since the previous call
  [[nodiscard]] virtual std::pair<int16_t, int16_t> getRelativeMotion() { return {0, 0}; }

  // State of a physical mouse button (a RETRO_DEVICE_ID_MOUSE_* button id)
  [[nodiscard]] virtual bool isMouseButtonPressed(int retroMouseButtonId) const { return false; }

  // True when the pointer is outside the game viewport (light-gun off-screen)
  [[nodiscard]] virtual bool isPointerOffscreen() const { return false; }

  // Moves the shared cursor by a delta (in ±32767 pointer units) and clears
  // the off-screen flag, used by analog stick
  virtual void nudgeCursor(int dx, int dy) {}
};
} // namespace firelight::libretro

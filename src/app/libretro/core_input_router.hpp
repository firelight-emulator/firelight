#pragma once

#include "firelight/libretro/pointer_input_provider.hpp"
#include "firelight/libretro/retropad_provider.hpp"

#include <firelight/input/gamepad_input.hpp>
#include <firelight/input/input_frame.hpp>

#include <array>
#include <cstdint>
#include <map>
#include <utility>

namespace firelight::libretro {

// Per-frame input snapshot + libretro input-state translation (joypad, analog,
// mouse, light gun) over the retropad + pointer providers
class CoreInputRouter {
public:
  static constexpr int MAX_INPUT_PORTS = 8;

  void setPlatformId(const int platformId) { m_platformId = platformId; }

  void setRetropadProvider(IRetropadProvider *provider) { m_retropadProvider = provider; }

  void setPointerInputProvider(IPointerInputProvider *provider) { m_pointerInputProvider = provider; }

  [[nodiscard]] IRetropadProvider *getRetropadProvider() const { return m_retropadProvider; }

  [[nodiscard]] IPointerInputProvider *getPointerInputProvider() const { return m_pointerInputProvider; }

  void setPortInputDeviceClass(const unsigned port, const int deviceClass) { m_portInputClass[port] = deviceClass; }

  // GamepadInputClass value; Joypad when unset
  [[nodiscard]] int getPortInputClass(unsigned port) const;

  void setAnalogPointerSpeed(const double stepPerFrame) { m_analogPointerSpeed = stepPerFrame; }

  void setMouseControlsPointerDevices(const bool enabled) { m_mouseControlsPointerDevices = enabled; }

  [[nodiscard]] bool mouseControlsPointerDevices() const { return m_mouseControlsPointerDevices; }

  // Snapshots pointer motion + each port's joypad state once per frame
  void pollInput();

  [[nodiscard]] int16_t readInputState(unsigned port, unsigned device, unsigned index, unsigned id) const;

private:
  int m_platformId = -1;
  IRetropadProvider *m_retropadProvider = nullptr;
  IPointerInputProvider *m_pointerInputProvider = nullptr;

  std::pair<int16_t, int16_t> m_frameMouseDelta{0, 0};
  std::array<firelight::input::InputFrame, MAX_INPUT_PORTS> m_portFrames{};
  std::array<bool, MAX_INPUT_PORTS> m_portActive{};

  std::map<unsigned, int> m_portInputClass;
  double m_analogPointerSpeed = 0.025;
  bool m_mouseControlsPointerDevices = true;
};

} // namespace firelight::libretro

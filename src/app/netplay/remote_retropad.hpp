#pragma once

#include <firelight/input/input_frame.hpp>
#include <firelight/libretro/retropad.hpp>

#include <chrono>
#include <mutex>

namespace firelight::netplay {

// A guest's controller as the host's core sees it: fed by InputPackets, read
// once per frame by the input router. Goes neutral when packets stop so a
// dropped guest never leaves a button stuck
class RemoteRetroPad final : public libretro::IRetroPad {
public:
  static constexpr auto STALE_AFTER = std::chrono::milliseconds(400);
  static constexpr int16_t STICK_DIRECTION_THRESHOLD = 16384;

  void applyFrame(const uint32_t seq, const input::InputFrame &frame) {
    std::lock_guard lock(m_mutex);
    if (m_hasFrame && static_cast<int32_t>(seq - m_lastSeq) <= 0) {
      return; // stale or duplicate (redundant copies arrive out of order)
    }
    m_lastSeq = seq;
    m_hasFrame = true;
    m_frame = frame;
    m_lastUpdate = std::chrono::steady_clock::now();
  }

  bool isButtonPressed(int, int, const Input button) override {
    const auto frame = currentFrame();
    const auto id = static_cast<unsigned>(button);
    if (id < 16) {
      return frame.button(id);
    }
    switch (button) {
    case LeftStickUp:
      return frame.leftStickY < -STICK_DIRECTION_THRESHOLD;
    case LeftStickDown:
      return frame.leftStickY > STICK_DIRECTION_THRESHOLD;
    case LeftStickLeft:
      return frame.leftStickX < -STICK_DIRECTION_THRESHOLD;
    case LeftStickRight:
      return frame.leftStickX > STICK_DIRECTION_THRESHOLD;
    case RightStickUp:
      return frame.rightStickY < -STICK_DIRECTION_THRESHOLD;
    case RightStickDown:
      return frame.rightStickY > STICK_DIRECTION_THRESHOLD;
    case RightStickLeft:
      return frame.rightStickX < -STICK_DIRECTION_THRESHOLD;
    case RightStickRight:
      return frame.rightStickX > STICK_DIRECTION_THRESHOLD;
    default:
      return false;
    }
  }

  int16_t getLeftStickXPosition(int, int) override {
    return currentFrame().leftStickX;
  }
  int16_t getLeftStickYPosition(int, int) override {
    return currentFrame().leftStickY;
  }
  int16_t getRightStickXPosition(int, int) override {
    return currentFrame().rightStickX;
  }
  int16_t getRightStickYPosition(int, int) override {
    return currentFrame().rightStickY;
  }

  void setStrongRumble(int, uint16_t) override {}
  void setWeakRumble(int, uint16_t) override {}

private:
  [[nodiscard]] input::InputFrame currentFrame() const {
    std::lock_guard lock(m_mutex);
    if (!m_hasFrame ||
        std::chrono::steady_clock::now() - m_lastUpdate > STALE_AFTER) {
      return {};
    }
    return m_frame;
  }

  mutable std::mutex m_mutex;
  input::InputFrame m_frame;
  uint32_t m_lastSeq = 0;
  bool m_hasFrame = false;
  std::chrono::steady_clock::time_point m_lastUpdate;
};

} // namespace firelight::netplay

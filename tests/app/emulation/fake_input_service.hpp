#pragma once

#include <firelight/input/input_service.hpp>

namespace firelight::emulation {

// Records the hotkey calls EmulatorInstance makes; every other method is an
// inert stub
class FakeInputService final : public input::InputService {
public:
  struct HotkeyCall {
    bool enabled;
    std::optional<DeviceType> only;
  };

  std::vector<HotkeyCall> hotkeyCalls;

  // Every value handed to setPreferGamepadOverKeyboard, in order
  std::vector<bool> preferGamepadOverKeyboardCalls;

  // What getPlayerGamepad answers, by slot
  std::map<int, std::shared_ptr<input::IGamepad>> playerSlots;

  // Every (from, to) handed to moveGamepad, in order
  std::vector<std::pair<int, int>> moveCalls;

  /** One setRumbleScale call */
  struct RumbleScaleCall {
    int profileId;
    int percent;
  };

  /** One setLightColor call */
  struct LightColorCall {
    int profileId;
    std::optional<uint32_t> rgb;
  };

  // Every setRumbleScale call, in order
  std::vector<RumbleScaleCall> rumbleScaleCalls;

  // Every setLightColor call, in order
  std::vector<LightColorCall> lightColorCalls;

  // What listGamepads answers
  std::vector<std::shared_ptr<input::IGamepad>> devices;

  void setHotkeysEnabled(const bool enabled, const std::optional<DeviceType> only = {}) override {
    hotkeyCalls.push_back({enabled, only});
  }

  // True when hotkeys are on for every device: the last blanket call enabled
  // them and nothing has since turned the keyboard off
  [[nodiscard]] bool hotkeysFullyEnabled() const {
    bool enabled = true;
    bool keyboardEnabled = true;
    for (const auto &[on, only] : hotkeyCalls) {
      if (only.has_value()) {
        if (*only == DeviceType::Keyboard) {
          keyboardEnabled = on;
        }
        continue;
      }
      enabled = on;
      keyboardEnabled = on;
    }
    return enabled && keyboardEnabled;
  }

  [[nodiscard]] bool keyboardHotkeysDisabled() const {
    for (auto it = hotkeyCalls.rbegin(); it != hotkeyCalls.rend(); ++it) {
      if (!it->only.has_value() || *it->only == DeviceType::Keyboard) {
        return !it->enabled;
      }
    }
    return false;
  }

  int addGamepad(std::shared_ptr<input::IGamepad>) override { return -1; }

  bool removeGamepadByInstanceId(int) override { return false; }

  bool removeGamepadByPlayerIndex(int) override { return false; }

  std::vector<std::shared_ptr<input::IGamepad>> listGamepads() override { return devices; }

  std::shared_ptr<input::IGamepad> getPlayerGamepad(const int playerIndex) override {
    const auto it = playerSlots.find(playerIndex);
    return it == playerSlots.end() ? nullptr : it->second;
  }

  void changeGamepadOrder(const std::map<int, int> &) override {}

  void swapGamepads(int, int) override {}

  void moveGamepad(const int from, const int to) override { moveCalls.emplace_back(from, to); }

  bool preferGamepadOverKeyboard() const override {
    return !preferGamepadOverKeyboardCalls.empty() && preferGamepadOverKeyboardCalls.back();
  }

  void setPreferGamepadOverKeyboard(const bool prefer) override { preferGamepadOverKeyboardCalls.push_back(prefer); }

  void updateMouseState(double, double, bool) override {}

  void updateMousePressed(bool) override {}

  void applyGameContext(std::optional<std::string>, int) override {}

  void clearGameContext() override {}

  void setShortcutContext(int) override {}

  std::shared_ptr<libretro::IRetroPad> getRetropadForPlayerIndex(int) override { return nullptr; }

  std::pair<int16_t, int16_t> getPointerPosition() const override { return {0, 0}; }

  bool isPressed() const override { return false; }

  /** Records the call */
  void setRumbleScale(const int profileId, const int percent) override {
    rumbleScaleCalls.push_back({profileId, percent});
  }

  /** Records the call */
  void setLightColor(const int profileId, const std::optional<uint32_t> rgb) override {
    lightColorCalls.push_back({profileId, rgb});
  }
};

} // namespace firelight::emulation

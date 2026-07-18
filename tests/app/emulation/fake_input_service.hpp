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

  std::vector<std::shared_ptr<input::IGamepad>> listGamepads() override { return {}; }

  std::shared_ptr<input::IGamepad> getPlayerGamepad(int) override { return nullptr; }

  void changeGamepadOrder(const std::map<int, int> &) override {}

  bool preferGamepadOverKeyboard() const override { return false; }

  void setPreferGamepadOverKeyboard(bool) override {}

  void updateMouseState(double, double, bool) override {}

  void updateMousePressed(bool) override {}

  void applyGameContext(std::optional<std::string>, int) override {}

  void clearGameContext() override {}

  void setShortcutContext(int) override {}

  std::shared_ptr<libretro::IRetroPad> getRetropadForPlayerIndex(int) override { return nullptr; }

  std::pair<int16_t, int16_t> getPointerPosition() const override { return {0, 0}; }

  bool isPressed() const override { return false; }
};

} // namespace firelight::emulation

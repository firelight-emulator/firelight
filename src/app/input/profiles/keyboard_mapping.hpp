#pragma once
#include "input_mapping.hpp"

namespace firelight::input {
class KeyboardMapping final : public InputMapping {
public:
  explicit KeyboardMapping(
      const std::function<void(InputMapping &)> &syncCallback = nullptr);

  void addKeyboardMapping(GamepadInput input, int mappedKeyCode);

  std::optional<int> getMappedKeyboardInput(GamepadInput input);

  std::map<GamepadInput, int> &getKeyboardMappings();

  void removeMapping(GamepadInput input) override;

  std::string serialize() override;

  void deserialize(const std::string &data) override;

private:
  std::map<GamepadInput, int> m_keyboardMappings;
};
} // namespace firelight::input
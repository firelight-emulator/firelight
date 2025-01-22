#pragma once
#include "input_mapping.hpp"

namespace firelight::input {
class KeyboardMapping final : public InputMapping {
public:
  explicit KeyboardMapping(
      const std::function<void(InputMapping &)> &syncCallback = nullptr);

  void addKeyboardMapping(libretro::IRetroPad::Input input, int mappedKeyCode);

  std::optional<int> getMappedKeyboardInput(libretro::IRetroPad::Input input);

  std::map<libretro::IRetroPad::Input, int> &getKeyboardMappings();

  void removeMapping(libretro::IRetroPad::Input input) override;

  std::string serialize() override;

  void deserialize(const std::string &data) override;

private:
  std::map<libretro::IRetroPad::Input, int> m_keyboardMappings;
};
}
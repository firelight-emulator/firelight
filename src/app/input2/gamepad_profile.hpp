#pragma once
#include "input/profiles/shortcut_mapping.hpp"

#include <input/profiles/input_mapping.hpp>
#include <memory>
#include <string>

namespace firelight::input {

class GamepadProfile {
public:
  explicit GamepadProfile(int id);

  [[nodiscard]] int getId() const;
  [[nodiscard]] std::string getName() const;

  void setName(const std::string &name);

  [[nodiscard]] std::shared_ptr<InputMapping>
  getMappingForPlatformAndController(int platformId, int controllerType) const;

  void addMapping(const std::shared_ptr<InputMapping> &mapping);

  void setShortcutMapping(std::shared_ptr<ShortcutMapping> mapping);

  [[nodiscard]] std::shared_ptr<ShortcutMapping> getShortcutMapping() const;

private:
  int m_id = -1;
  std::string m_name;

  std::vector<std::shared_ptr<InputMapping>> m_mappings;
  std::shared_ptr<ShortcutMapping> m_shortcutMapping;
};

} // namespace firelight::input
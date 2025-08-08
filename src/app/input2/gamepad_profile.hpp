#pragma once
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

private:
  int m_id = -1;
  std::string m_name;

  std::vector<std::shared_ptr<InputMapping>> m_mappings;
};

} // namespace firelight::input
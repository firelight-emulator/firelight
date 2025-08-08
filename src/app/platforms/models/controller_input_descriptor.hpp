#pragma once
#include "input/gamepad_input.hpp"

#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <string>

namespace firelight::platforms {

struct PlatformInputDescriptor {
  std::string name;
  input::GamepadInput virtualInput;
};

inline void from_json(const nlohmann::json &j, PlatformInputDescriptor &c) {
  j.at("name").get_to(c.name);
  j.at("gamepad_input").get_to(c.virtualInput);
}

inline void to_json(nlohmann::json &j, const PlatformInputDescriptor &c) {
  j = nlohmann::json{{"name", c.name}, {"gamepad_input", c.virtualInput}};
}

} // namespace firelight::platforms
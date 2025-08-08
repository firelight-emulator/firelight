#pragma once

#include "controller_input_descriptor.hpp"

#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <string>

namespace firelight::platforms {

struct ControllerType {
  unsigned id = 0;
  std::string name;
  std::string imageUrl;
  std::vector<PlatformInputDescriptor> inputs;
};

inline void from_json(const nlohmann::json &j, ControllerType &c) {
  j.at("id").get_to(c.id);
  j.at("name").get_to(c.name);
  j.at("inputs").get_to(c.inputs);
}

inline void to_json(nlohmann::json &j, const ControllerType &c) {
  j = nlohmann::json{{"id", c.id}, {"name", c.name}, {"inputs", c.inputs}};
}

} // namespace firelight::platforms
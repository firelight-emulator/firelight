#pragma once

#include <firelight/platforms/controller_input_descriptor.hpp>

#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <string>

namespace firelight::platforms {

// TODO
// A per-platform, console-native *display* entry for one input device class
// (a controller, a mouse, or a light gun). The mapping stack is keyed by the
// device class, so `id` mirrors `deviceClass` (Joypad=1, Mouse=2, Lightgun=3)
// and existing joypad mappings (key 1) stay valid. Which concrete device
// variants (3-button vs 6-button, Menacer vs Justifier, …) are *selectable*
// comes from the per-core catalog, not from here
struct ControllerType {
  unsigned id = 0;
  std::string name;
  std::string imageUrl;
  input::GamepadInputClass deviceClass = input::GamepadInputClass::Joypad;
  std::vector<PlatformInputDescriptor> inputs;
};

inline void from_json(const nlohmann::json &j, ControllerType &c) {
  j.at("id").get_to(c.id);
  j.at("name").get_to(c.name);
  c.deviceClass = static_cast<input::GamepadInputClass>(j.value(
      "device_class", static_cast<int>(input::GamepadInputClass::Joypad)));
  j.at("inputs").get_to(c.inputs);
}

inline void to_json(nlohmann::json &j, const ControllerType &c) {
  j = nlohmann::json{{"id", c.id},
                     {"name", c.name},
                     {"device_class", static_cast<int>(c.deviceClass)},
                     {"inputs", c.inputs}};
}

} // namespace firelight::platforms

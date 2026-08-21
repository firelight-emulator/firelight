#include <firelight/input/gamepad_input.hpp>

#include <map>
#include <string>

namespace firelight::input {

std::optional<GamepadInput> gamepadInputFromName(const std::string_view name) {
  // The names are the enum identifiers verbatim, so a data file and this header
  // read the same. Mouse and light-gun inputs are left out on purpose: they are
  // inputs of an emulated device, not buttons a physical pad has, so a shortcut
  // could never be triggered by one
  static const std::map<std::string, GamepadInput, std::less<>> BY_NAME = {
      {"SouthFace", SouthFace},
      {"EastFace", EastFace},
      {"WestFace", WestFace},
      {"NorthFace", NorthFace},
      {"Select", Select},
      {"Start", Start},
      {"DpadUp", DpadUp},
      {"DpadDown", DpadDown},
      {"DpadLeft", DpadLeft},
      {"DpadRight", DpadRight},
      {"LeftBumper", LeftBumper},
      {"RightBumper", RightBumper},
      {"LeftTrigger", LeftTrigger},
      {"RightTrigger", RightTrigger},
      {"L3", L3},
      {"R3", R3},
      {"LeftStickUp", LeftStickUp},
      {"LeftStickDown", LeftStickDown},
      {"LeftStickLeft", LeftStickLeft},
      {"LeftStickRight", LeftStickRight},
      {"RightStickUp", RightStickUp},
      {"RightStickDown", RightStickDown},
      {"RightStickLeft", RightStickLeft},
      {"RightStickRight", RightStickRight},
      {"Home", Home},
  };

  const auto it = BY_NAME.find(name);
  return it == BY_NAME.end() ? std::nullopt : std::optional(it->second);
}

} // namespace firelight::input

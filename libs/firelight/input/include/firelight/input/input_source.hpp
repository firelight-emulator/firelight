#pragma once
#include <firelight/input/gamepad_input.hpp>

#include <nlohmann/json.hpp>
#include <vector>

namespace firelight::input {

// What a binding physically reads from. For gamepad profiles, `code` is a
// GamepadInput; for keyboard profiles it is a Qt::Key.
enum class SourceType {
  None,
  Button,       // digital button (or an analog input treated digitally)
  AxisPositive, // positive half of an analog axis (e.g. stick right/down)
  AxisNegative, // negative half of an analog axis (e.g. stick left/up)
  Key,          // keyboard key (Qt::Key)
};

NLOHMANN_JSON_SERIALIZE_ENUM(SourceType,
                             {
                                 {SourceType::None, "none"},
                                 {SourceType::Button, "button"},
                                 {SourceType::AxisPositive, "axisPos"},
                                 {SourceType::AxisNegative, "axisNeg"},
                                 {SourceType::Key, "key"},
                             })

struct InputSource {
  SourceType type = SourceType::None;
  int code = 0;               // GamepadInput (gamepad) or Qt::Key (keyboard)
  std::vector<int> modifiers; // inputs that must be held simultaneously (combo)

  bool operator==(const InputSource &other) const {
    return type == other.type && code == other.code &&
           modifiers == other.modifiers;
  }
};

inline void to_json(nlohmann::json &j, const InputSource &s) {
  j = nlohmann::json{{"type", s.type}, {"code", s.code}};
  if (!s.modifiers.empty()) {
    j["modifiers"] = s.modifiers;
  }
}

inline void from_json(const nlohmann::json &j, InputSource &s) {
  j.at("type").get_to(s.type);
  j.at("code").get_to(s.code);
  if (j.contains("modifiers")) {
    j.at("modifiers").get_to(s.modifiers);
  } else {
    s.modifiers.clear();
  }
}

} // namespace firelight::input

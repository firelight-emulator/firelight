#pragma once
#include "gamepad_input.hpp"
#include <vector>

namespace firelight::input {

struct InputSequence {
  std::vector<GamepadInput> modifiers;
  GamepadInput input;
};

} // namespace firelight::input
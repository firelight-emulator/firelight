#pragma once
#include "gamepad_input.hpp"
#include <vector>

namespace firelight::input {

struct InputSequence {
  std::vector<int> modifiers;
  int input;
};

} // namespace firelight::input
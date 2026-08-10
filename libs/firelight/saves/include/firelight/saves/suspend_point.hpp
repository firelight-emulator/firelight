#pragma once

#include "firelight/image.hpp"

#include <cstdint>
#include <string>
#include <vector>

struct SuspendPoint {
  std::string contentHash;
  std::vector<uint8_t> state;
  std::vector<uint8_t> retroachievementsState;
  // Epoch milliseconds
  long long timestamp = 0;
  firelight::Image image;
  bool locked = false;
  int saveSlot = 0;
};

#pragma once

#include "firelight/image.hpp"

#include <cstdint>
#include <string>
#include <vector>

struct SuspendPoint {
  std::string contentHash;
  std::vector<uint8_t> state;
  std::vector<uint8_t> retroachievementsState;
  long long timestamp;
  firelight::Image image;
  bool locked = false;
  int saveSlotNumber;
};

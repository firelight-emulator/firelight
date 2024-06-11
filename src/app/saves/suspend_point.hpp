#pragma once
#include <cstdint>
#include <vector>

struct SuspendPoint {
  std::vector<uint8_t> state;
  long long timestamp;
};
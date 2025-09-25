#pragma once
#include <nlohmann/json.hpp>
#include <vector>

namespace firelight::achievements {
struct Unlock {
  unsigned ID{};
  unsigned long long When{};
};

struct StartSessionResponse {
  bool Success{};
  std::vector<Unlock> HardcoreUnlocks;
  std::vector<Unlock> Unlocks;
  long long ServerNow{};
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Unlock, ID, When)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(StartSessionResponse, Success,
                                                HardcoreUnlocks, Unlocks,
                                                ServerNow)
} // namespace firelight::achievements

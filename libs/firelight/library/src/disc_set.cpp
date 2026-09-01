// TODO: NEEDS REVIEW
#include <firelight/library/disc_set.hpp>

#include <algorithm>
#include <set>

namespace firelight::library {

namespace {
// Discs arrive in whatever order the scanner found them, and duplicates are possible when the
// same disc sits in two folders
std::set<int> presentDiscs(const std::vector<int> &discNumbers) {
  std::set<int> present;

  for (const auto number : discNumbers) {
    if (number > 0) {
      present.insert(number);
    }
  }

  return present;
}
} // namespace

std::vector<int> missingDiscs(const std::vector<int> &discNumbers, const int knownDiscCount) {
  const auto present = presentDiscs(discNumbers);
  std::vector<int> missing;

  if (present.empty()) {
    return missing;
  }

  // A known count reaches past the discs on hand, which is the only way to notice a set is short
  // of its last one rather than merely holed in the middle
  const auto highest = std::max(*present.rbegin(), knownDiscCount);

  for (auto number = 1; number <= highest; ++number) {
    if (present.count(number) == 0) {
      missing.push_back(number);
    }
  }

  return missing;
}

} // namespace firelight::library

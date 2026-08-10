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

int representativeDisc(const std::vector<int> &discNumbers) {
  const auto present = presentDiscs(discNumbers);
  return present.empty() ? 0 : *present.begin();
}

DiscSetState discSetState(const std::vector<int> &discNumbers) {
  const auto present = presentDiscs(discNumbers);

  if (present.empty() || *present.begin() != 1) {
    return DiscSetState::MissingFirstDisc;
  }

  // Contiguous from 1 means the count matches the highest number
  return present.size() == static_cast<size_t>(*present.rbegin()) ? DiscSetState::Complete
                                                                  : DiscSetState::MissingMiddleDisc;
}

std::vector<int> missingDiscs(const std::vector<int> &discNumbers) {
  const auto present = presentDiscs(discNumbers);
  std::vector<int> missing;

  if (present.empty()) {
    return missing;
  }

  for (auto number = 1; number <= *present.rbegin(); ++number) {
    if (present.count(number) == 0) {
      missing.push_back(number);
    }
  }

  return missing;
}

bool isDiscSetLaunchable(const std::vector<int> &discNumbers) {
  const auto present = presentDiscs(discNumbers);
  return !present.empty() && *present.begin() == 1;
}

} // namespace firelight::library

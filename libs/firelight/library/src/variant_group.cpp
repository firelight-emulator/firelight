#include <firelight/library/filename_tags.hpp>
#include <firelight/library/variant_group.hpp>

#include <algorithm>
#include <cctype>
#include <limits>

namespace firelight::library {

namespace {
constexpr int NO_RANK = std::numeric_limits<int>::max();

// The flags that mean a variant is not a finished release
bool isUnreleased(const std::vector<std::string> &flags) {
  static const std::vector<std::string> unreleased = {content_flags::BETA,     content_flags::PROTO,
                                                      content_flags::DEMO,     content_flags::SAMPLE,
                                                      content_flags::BAD_DUMP, content_flags::PIRATE};

  return std::any_of(flags.begin(), flags.end(), [](const std::string &flag) {
    return std::find(unreleased.begin(), unreleased.end(), flag) != unreleased.end();
  });
}

// The best position any of the values holds in the ordering, or NO_RANK when none appears
int bestRank(const std::vector<std::string> &values, const std::vector<std::string> &order) {
  auto best = NO_RANK;

  for (const auto &value : values) {
    const auto found = std::find(order.begin(), order.end(), value);

    if (found != order.end()) {
      best = std::min(best, static_cast<int>(std::distance(order.begin(), found)));
    }
  }

  return best;
}

// Negative when left sorts before right. Revisions are numbers often enough to compare as such, but
// not always, so a non-numeric pair falls back to lexicographic
int compareRevisions(const std::string &left, const std::string &right) {
  const auto isNumeric = [](const std::string &value) {
    return !value.empty() &&
           std::all_of(value.begin(), value.end(), [](const unsigned char c) { return std::isdigit(c); });
  };

  if (isNumeric(left) && isNumeric(right)) {
    const auto leftValue = std::stoi(left);
    const auto rightValue = std::stoi(right);
    return leftValue == rightValue ? 0 : (leftValue < rightValue ? -1 : 1);
  }

  return left == right ? 0 : (left < right ? -1 : 1);
}
} // namespace

std::optional<int> choosePrimaryEntry(const std::vector<VariantCandidate> &candidates,
                                      const VariantPreference &preference) {
  const VariantCandidate *best = nullptr;

  for (const auto &candidate : candidates) {
    if (candidate.hidden) {
      continue;
    }

    if (best == nullptr) {
      best = &candidate;
      continue;
    }

    const auto candidateUnreleased = isUnreleased(candidate.flags);
    const auto bestUnreleased = isUnreleased(best->flags);

    if (candidateUnreleased != bestUnreleased) {
      if (!candidateUnreleased) {
        best = &candidate;
      }

      continue;
    }

    const auto candidateRegion = bestRank(candidate.regions, preference.regionOrder);
    const auto bestRegion = bestRank(best->regions, preference.regionOrder);

    if (candidateRegion != bestRegion) {
      if (candidateRegion < bestRegion) {
        best = &candidate;
      }

      continue;
    }

    const auto candidateLanguage = bestRank(candidate.languages, preference.languageOrder);
    const auto bestLanguage = bestRank(best->languages, preference.languageOrder);

    if (candidateLanguage != bestLanguage) {
      if (candidateLanguage < bestLanguage) {
        best = &candidate;
      }

      continue;
    }

    const auto revisions = compareRevisions(candidate.revision, best->revision);

    if (revisions != 0) {
      if (revisions > 0) {
        best = &candidate;
      }

      continue;
    }

    if (candidate.entryId < best->entryId) {
      best = &candidate;
    }
  }

  if (best == nullptr) {
    return std::nullopt;
  }

  return best->entryId;
}

std::vector<std::string> parsePreferenceOrder(const std::string &value) {
  std::vector<std::string> order;
  size_t start = 0;

  while (true) {
    const auto comma = value.find(',', start);
    auto token = value.substr(start, comma == std::string::npos ? std::string::npos : comma - start);

    const auto first = token.find_first_not_of(" \t");

    if (first != std::string::npos) {
      const auto last = token.find_last_not_of(" \t");
      order.push_back(token.substr(first, last - first + 1));
    }

    if (comma == std::string::npos) {
      break;
    }

    start = comma + 1;
  }

  return order;
}

} // namespace firelight::library

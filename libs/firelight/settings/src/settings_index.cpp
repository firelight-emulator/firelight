#include "firelight/settings/settings_index.hpp"

#include <algorithm>
#include <cctype>

namespace firelight::settings {

namespace {

// Ranked strongest to weakest. A hit takes its best score, not the sum, so one
// strong match can't be outweighed by several incidental ones.
constexpr int SCORE_EXACT_KEY = 100;
constexpr int SCORE_EXACT_LABEL = 90;
constexpr int SCORE_LABEL_PREFIX = 70;
constexpr int SCORE_LABEL_SUBSTRING = 50;
constexpr int SCORE_KEYWORD_EXACT = 45;
constexpr int SCORE_KEYWORD_PREFIX = 40;
constexpr int SCORE_KEYWORD_SUBSTRING = 30;
constexpr int SCORE_KEY_SUBSTRING = 25;
constexpr int SCORE_DESCRIPTION_SUBSTRING = 10;

std::string toLower(const std::string &s) {
  std::string out;
  out.reserve(s.size());
  for (const unsigned char c : s) {
    out.push_back(static_cast<char>(std::tolower(c)));
  }
  return out;
}

std::string trim(const std::string &s) {
  const auto begin = s.find_first_not_of(" \t\n\r");
  if (begin == std::string::npos) {
    return {};
  }
  const auto end = s.find_last_not_of(" \t\n\r");
  return s.substr(begin, end - begin + 1);
}

bool startsWith(const std::string &haystack, const std::string &needle) {
  return haystack.rfind(needle, 0) == 0;
}

bool contains(const std::string &haystack, const std::string &needle) {
  return haystack.find(needle) != std::string::npos;
}

} // namespace

SettingsIndex::SettingsIndex(const SettingsCatalog &catalog) {
  rebuild(catalog);
}

void SettingsIndex::rebuild(const SettingsCatalog &catalog) {
  m_entries.clear();

  for (const auto &page : catalog.pages()) {
    Entry entry;
    entry.result.kind = SearchResultKind::Page;
    entry.result.label = page.label;
    entry.result.pageId = page.id;
    entry.result.pageLabel = page.label;
    entry.result.route = page.route;
    entry.keyLower = toLower(page.id);
    entry.labelLower = toLower(page.label);
    for (const auto &keyword : page.keywords) {
      entry.keywordsLower.push_back(toLower(keyword));
    }
    m_entries.push_back(std::move(entry));
  }

  for (const auto *setting : catalog.allSettings()) {
    const auto *group = catalog.findGroup(setting->groupId);
    const auto *page = group ? catalog.findPage(group->pageId) : nullptr;

    Entry entry;
    entry.result.kind = SearchResultKind::Setting;
    entry.result.key = setting->key;
    entry.result.label = setting->label;
    entry.result.description = setting->description;
    entry.result.groupLabel = group ? group->label : std::string{};
    entry.result.pageId = page ? page->id : std::string{};
    entry.result.pageLabel = page ? page->label : std::string{};
    entry.result.route = page ? page->route : std::string{};
    entry.result.advanced = setting->advanced;
    entry.keyLower = toLower(setting->key);
    entry.labelLower = toLower(setting->label);
    entry.descriptionLower = toLower(setting->description);
    for (const auto &keyword : setting->keywords) {
      entry.keywordsLower.push_back(toLower(keyword));
    }
    m_entries.push_back(std::move(entry));
  }
}

std::vector<SettingSearchResult> SettingsIndex::search(const std::string &query,
                                                       const int limit) const {
  const auto needle = toLower(trim(query));
  if (needle.empty()) {
    return {};
  }

  std::vector<SettingSearchResult> results;
  for (const auto &entry : m_entries) {
    int score = 0;
    const auto consider = [&score](const int candidate) {
      score = std::max(score, candidate);
    };

    if (entry.keyLower == needle) {
      consider(SCORE_EXACT_KEY);
    } else if (contains(entry.keyLower, needle)) {
      consider(SCORE_KEY_SUBSTRING);
    }

    if (entry.labelLower == needle) {
      consider(SCORE_EXACT_LABEL);
    } else if (startsWith(entry.labelLower, needle)) {
      consider(SCORE_LABEL_PREFIX);
    } else if (contains(entry.labelLower, needle)) {
      consider(SCORE_LABEL_SUBSTRING);
    }

    for (const auto &keyword : entry.keywordsLower) {
      if (keyword == needle) {
        consider(SCORE_KEYWORD_EXACT);
      } else if (startsWith(keyword, needle)) {
        consider(SCORE_KEYWORD_PREFIX);
      } else if (contains(keyword, needle)) {
        consider(SCORE_KEYWORD_SUBSTRING);
      }
    }

    if (contains(entry.descriptionLower, needle)) {
      consider(SCORE_DESCRIPTION_SUBSTRING);
    }

    if (score > 0) {
      auto result = entry.result;
      result.score = score;
      results.push_back(std::move(result));
    }
  }

  std::stable_sort(results.begin(), results.end(),
                   [](const SettingSearchResult &a,
                      const SettingSearchResult &b) {
                     if (a.score != b.score) {
                       return a.score > b.score;
                     }
                     // Ties read better alphabetically than in catalog order.
                     return a.label < b.label;
                   });

  if (limit > 0 && results.size() > static_cast<size_t>(limit)) {
    results.resize(static_cast<size_t>(limit));
  }
  return results;
}

} // namespace firelight::settings

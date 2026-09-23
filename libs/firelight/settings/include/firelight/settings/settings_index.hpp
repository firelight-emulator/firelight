#pragma once

#include <firelight/settings/settings_catalog.hpp>

#include <string>
#include <vector>

namespace firelight::settings {

/**
 * A search hit is either a whole page ("Appearance") or one setting within one
 * ("Sync method", under Emulation > Video)
 */
enum class SearchResultKind { Page, Setting };

struct SettingSearchResult {
  SearchResultKind kind = SearchResultKind::Setting;
  std::string key; // Empty for a page hit
  std::string label;
  std::string description;
  std::string pageId;
  std::string pageLabel;
  std::string groupLabel; // Empty for a page hit
  std::string route;
  bool advanced = false;
  int score = 0;
};


// Snapshots the catalog at construction
class SettingsIndex {
public:
  explicit SettingsIndex(const SettingsCatalog &catalog);

  void rebuild(const SettingsCatalog &catalog);

  // Best matches first, capped at limit (<= 0 means no cap). An empty or whitespace-only query matches nothing
  [[nodiscard]] std::vector<SettingSearchResult> search(const std::string &query, int limit = 20) const;

  [[nodiscard]] size_t size() const { return m_entries.size(); }

private:
  struct Entry {
    SettingSearchResult result;
    std::string keyLower;
    std::string labelLower;
    std::string descriptionLower;
    std::vector<std::string> keywordsLower;
  };

  std::vector<Entry> m_entries;
};

} // namespace firelight::settings

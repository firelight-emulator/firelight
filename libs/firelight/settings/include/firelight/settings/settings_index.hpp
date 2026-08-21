#pragma once

#include <firelight/settings/settings_catalog.hpp>

#include <string>
#include <vector>

namespace firelight::settings {

// A search hit is either a whole page ("Appearance") or one setting within one
// ("Sync method", under Emulation > Video). Pages answer "where do I go?";
// settings answer "where does this specific knob live?"
enum class SearchResultKind { Page, Setting };

struct SettingSearchResult {
  SearchResultKind kind = SearchResultKind::Setting;
  // Empty for a page hit
  std::string key;
  std::string label;
  std::string description;
  std::string pageId;
  std::string pageLabel;
  // Empty for a page hit
  std::string groupLabel;
  std::string route;
  bool advanced = false;
  int score = 0;
};

// Flattens the catalog into something searchable: every page and every declared
// setting, matched on label, key, keywords and description. This is the whole
// point of declaring settings in one file — the hand-maintained keyword list
// the settings nav used to carry could only ever find pages
//
// Snapshots the catalog at construction; rebuild() after a reload
class SettingsIndex {
public:
  explicit SettingsIndex(const SettingsCatalog &catalog);

  void rebuild(const SettingsCatalog &catalog);

  // Best matches first, capped at `limit` (<= 0 means no cap). An empty or
  // whitespace-only query matches nothing
  [[nodiscard]] std::vector<SettingSearchResult> search(const std::string &query, int limit = 20) const;

  [[nodiscard]] size_t size() const { return m_entries.size(); }

private:
  struct Entry {
    SettingSearchResult result;
    // Lowercased once at build time; queries are lowercased per search
    std::string keyLower;
    std::string labelLower;
    std::string descriptionLower;
    std::vector<std::string> keywordsLower;
  };

  std::vector<Entry> m_entries;
};

} // namespace firelight::settings

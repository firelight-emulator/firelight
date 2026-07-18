#include <firelight/settings/settings_index.hpp>

#include <gtest/gtest.h>

#include <algorithm>

namespace firelight::settings {

namespace {
const char *INDEX_JSON = R"JSON(
{
  "pages": [
    {"id": "appearance", "label": "Appearance", "route": "/settings/appearance",
     "order": 10, "keywords": ["theme", "look"]},
    {"id": "emulation", "label": "Emulation", "route": "/settings/emulation",
     "order": 20}
  ],
  "groups": [
    {"id": "theme", "page": "appearance", "label": "Theme", "order": 10},
    {"id": "video", "page": "emulation", "label": "Video", "order": 10}
  ],
  "app": [
    {"key": "accent-color", "group": "theme", "label": "Accent color",
     "type": "color", "default": "#f76b15", "keywords": ["highlight"]}
  ],
  "common": [
    {"key": "sync-method", "group": "video", "label": "Sync method",
     "description": "How gameplay is paced.",
     "keywords": ["vsync", "refresh rate", "tearing"],
     "type": "options", "default": "audio",
     "options": [{"label": "Audio", "value": "audio"}]},
    {"key": "target-framerate", "group": "video", "label": "Target framerate",
     "type": "spinbox", "default": "60", "advanced": true,
     "keywords": ["fps"]}
  ],
  "cores": {
    "mgba_libretro": {
      "settings": [
        {"key": "gba-color-correction", "group": "video",
         "label": "Color correction", "type": "options", "default": "Off",
         "options": [{"label": "Off", "value": "Off"}]}
      ]
    }
  }
}
)JSON";

SettingsIndex indexFor(const char *json) {
  SettingsCatalog catalog;
  EXPECT_TRUE(catalog.loadFromJson(json));
  return SettingsIndex(catalog);
}

const SettingSearchResult *find(const std::vector<SettingSearchResult> &v,
                                const std::string &key) {
  const auto it = std::find_if(v.begin(), v.end(),
                               [&](const auto &r) { return r.key == key; });
  return it != v.end() ? &*it : nullptr;
}
} // namespace

TEST(SettingsIndexTest, IndexesEveryPageAndSetting) {
  const auto index = indexFor(INDEX_JSON);
  // 2 pages + 1 app + 2 common + 1 core setting
  EXPECT_EQ(index.size(), 6u);
}

TEST(SettingsIndexTest, FindsSettingByKeywordNotInItsLabel) {
  // The whole point of the index: "vsync" appears in no label or page name
  const auto results = indexFor(INDEX_JSON).search("vsync");
  ASSERT_FALSE(results.empty());
  EXPECT_EQ(results.front().key, "sync-method");
  EXPECT_EQ(results.front().kind, SearchResultKind::Setting);
}

TEST(SettingsIndexTest, ResultCarriesWhereItLives) {
  const auto results = indexFor(INDEX_JSON).search("vsync");
  ASSERT_FALSE(results.empty());
  const auto &hit = results.front();
  EXPECT_EQ(hit.label, "Sync method");
  EXPECT_EQ(hit.groupLabel, "Video");
  EXPECT_EQ(hit.pageLabel, "Emulation");
  EXPECT_EQ(hit.route, "/settings/emulation");
}

TEST(SettingsIndexTest, ResolvesRouteForCoreSpecificSettings) {
  // A core's settings are declared under `cores`, but their group still lands
  // them on a page
  const auto results = indexFor(INDEX_JSON).search("gba-color-correction");
  ASSERT_FALSE(results.empty());
  EXPECT_EQ(results.front().route, "/settings/emulation");
  EXPECT_EQ(results.front().groupLabel, "Video");
}

TEST(SettingsIndexTest, MatchesPagesByNameAndKeyword) {
  const auto index = indexFor(INDEX_JSON);

  const auto byName = index.search("appearance");
  ASSERT_FALSE(byName.empty());
  EXPECT_EQ(byName.front().kind, SearchResultKind::Page);
  EXPECT_EQ(byName.front().route, "/settings/appearance");
  EXPECT_TRUE(byName.front().key.empty());

  // "theme" is a page keyword, and also the name of a group — the page hit is
  // what's navigable
  const auto byKeyword = index.search("theme");
  ASSERT_FALSE(byKeyword.empty());
  EXPECT_EQ(byKeyword.front().pageId, "appearance");
}

TEST(SettingsIndexTest, RanksStrongerMatchesFirst) {
  const auto index = indexFor(INDEX_JSON);

  // An exact label beats a description-only mention
  const auto results = index.search("sync method");
  ASSERT_FALSE(results.empty());
  EXPECT_EQ(results.front().key, "sync-method");

  // A label prefix beats a keyword hit elsewhere
  const auto accent = index.search("accent");
  ASSERT_FALSE(accent.empty());
  EXPECT_EQ(accent.front().key, "accent-color");
}

TEST(SettingsIndexTest, MatchesDescriptionAsLastResort) {
  const auto results = indexFor(INDEX_JSON).search("paced");
  ASSERT_FALSE(results.empty());
  EXPECT_EQ(results.front().key, "sync-method");
}

TEST(SettingsIndexTest, IsCaseInsensitiveAndTrims) {
  const auto index = indexFor(INDEX_JSON);
  EXPECT_FALSE(index.search("VSYNC").empty());
  EXPECT_FALSE(index.search("  vsync  ").empty());
  EXPECT_FALSE(index.search("Sync Method").empty());
}

TEST(SettingsIndexTest, EmptyQueryMatchesNothing) {
  const auto index = indexFor(INDEX_JSON);
  EXPECT_TRUE(index.search("").empty());
  EXPECT_TRUE(index.search("   ").empty());
  EXPECT_TRUE(index.search("zzzznotathing").empty());
}

TEST(SettingsIndexTest, HonorsLimit) {
  const auto index = indexFor(INDEX_JSON);
  // "e" appears all over; the cap is what keeps the results list sane
  EXPECT_LE(index.search("e", 2).size(), 2u);
  EXPECT_GT(index.search("e", 0).size(), 2u);
}

TEST(SettingsIndexTest, SurfacesAdvancedSettingsAndFlagsThem) {
  const auto results = indexFor(INDEX_JSON).search("fps");
  const auto *hit = find(results, "target-framerate");
  ASSERT_NE(hit, nullptr);
  // Findable even though the page hides it by default; the flag lets the UI
  // decide what to do about that
  EXPECT_TRUE(hit->advanced);
}

TEST(SettingsIndexTest, RebuildReplacesEntries) {
  SettingsCatalog catalog;
  ASSERT_TRUE(catalog.loadFromJson(INDEX_JSON));
  SettingsIndex index(catalog);
  ASSERT_FALSE(index.search("vsync").empty());

  ASSERT_TRUE(catalog.loadFromJson("{}"));
  index.rebuild(catalog);
  EXPECT_EQ(index.size(), 0u);
  EXPECT_TRUE(index.search("vsync").empty());
}

} // namespace firelight::settings

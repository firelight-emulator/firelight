#include <firelight/settings/sqlite_settings_repository.hpp>

#include <gtest/gtest.h>

namespace firelight::settings {

namespace {
CoreOption option(std::string key, std::string defaultValue, std::vector<CoreOptionValue> values) {
  CoreOption o;
  o.key = std::move(key);
  o.label = "label-" + o.key;
  o.description = "desc-" + o.key;
  o.defaultValue = std::move(defaultValue);
  o.values = std::move(values);
  return o;
}
} // namespace

TEST(SqliteSettingsRepositoryCoreOptionsTest, RoundTripsOptionsInOrder) {
  SqliteSettingsRepository repo(":memory:");
  repo.upsertCoreOptions("mgba_libretro",
                         {option("opt_a", "on", {{"on", "On"}, {"off", "Off"}}), option("opt_b", "1", {{"1", "One"}})});

  const auto read = repo.getCoreOptions("mgba_libretro");
  ASSERT_EQ(read.size(), 2u);

  EXPECT_EQ(read[0].key, "opt_a");
  EXPECT_EQ(read[0].label, "label-opt_a");
  EXPECT_EQ(read[0].defaultValue, "on");
  ASSERT_EQ(read[0].values.size(), 2u);
  EXPECT_EQ(read[0].values[1].value, "off");
  EXPECT_EQ(read[0].values[1].label, "Off");

  // Insertion order is preserved
  EXPECT_EQ(read[1].key, "opt_b");
}

TEST(SqliteSettingsRepositoryCoreOptionsTest, RoundTripsCategory) {
  SqliteSettingsRepository repo(":memory:");
  CoreOption withCategory = option("mgba_color", "OFF", {{"OFF", "Off"}});
  withCategory.category = "video";
  withCategory.categoryLabel = "Video";
  repo.upsertCoreOptions("core", {withCategory});

  const auto read = repo.getCoreOptions("core");
  ASSERT_EQ(read.size(), 1u);
  EXPECT_EQ(read[0].category, "video");
  EXPECT_EQ(read[0].categoryLabel, "Video");
}

TEST(SqliteSettingsRepositoryCoreOptionsTest, UpsertReplacesPreviousSet) {
  SqliteSettingsRepository repo(":memory:");
  repo.upsertCoreOptions("core", {option("old", "x", {})});
  repo.upsertCoreOptions("core", {option("new", "y", {})});

  const auto read = repo.getCoreOptions("core");
  ASSERT_EQ(read.size(), 1u);
  EXPECT_EQ(read[0].key, "new");
}

TEST(SqliteSettingsRepositoryCoreOptionsTest, IsolatesByCoreName) {
  SqliteSettingsRepository repo(":memory:");
  repo.upsertCoreOptions("core_one", {option("a", "1", {})});
  repo.upsertCoreOptions("core_two", {option("b", "2", {})});

  EXPECT_EQ(repo.getCoreOptions("core_one").size(), 1u);
  EXPECT_EQ(repo.getCoreOptions("core_two").at(0).key, "b");
}

TEST(SqliteSettingsRepositoryCoreOptionsTest, UnknownCoreReturnsEmpty) {
  SqliteSettingsRepository repo(":memory:");
  EXPECT_TRUE(repo.getCoreOptions("nope").empty());
}

} // namespace firelight::settings

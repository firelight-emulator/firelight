#include <firelight/settings/setting_definition.hpp>

#include <gtest/gtest.h>

namespace firelight::settings {

TEST(EmulationSettingMappingTest, NoMappingReturnsEmpty) {
  SettingDefinition s;
  s.key = "rewind-enabled"; // frontend-only setting
  EXPECT_TRUE(resolveCoreOptionValues(s, "true").empty());
}

TEST(EmulationSettingMappingTest, IdentityMappingPassesValueThrough) {
  SettingDefinition s;
  s.key = "friendly-x";
  s.mapping = {{.coreKey = "core_x"}}; // empty valueMap => identity
  const auto r = resolveCoreOptionValues(s, "somevalue");
  ASSERT_EQ(r.size(), 1u);
  EXPECT_EQ(r[0].first, "core_x");
  EXPECT_EQ(r[0].second, "somevalue");
}

TEST(EmulationSettingMappingTest, ValueTranslation) {
  SettingDefinition s;
  s.key = "color-correction";
  s.mapping = {{.coreKey = "gambatte_gbc_color_correction_mode",
                .valueMap = {{"Accurate", "accurate"}, {"Fast", "fast"}}}};

  const auto accurate = resolveCoreOptionValues(s, "Accurate");
  ASSERT_EQ(accurate.size(), 1u);
  EXPECT_EQ(accurate[0].first, "gambatte_gbc_color_correction_mode");
  EXPECT_EQ(accurate[0].second, "accurate");

  // A friendly value with no explicit translation falls back to identity.
  const auto unknown = resolveCoreOptionValues(s, "Weird");
  ASSERT_EQ(unknown.size(), 1u);
  EXPECT_EQ(unknown[0].second, "Weird");
}

TEST(EmulationSettingMappingTest, CompositeDrivesMultipleCoreOptions) {
  SettingDefinition s;
  s.key = "enhanced-color";
  s.mapping = {{.coreKey = "core_correction", .valueMap = {{"on", "enabled"}}},
               {.coreKey = "core_palette", .valueMap = {{"on", "rich"}}}};

  const auto r = resolveCoreOptionValues(s, "on");
  ASSERT_EQ(r.size(), 2u);
  EXPECT_EQ(r[0].first, "core_correction");
  EXPECT_EQ(r[0].second, "enabled");
  EXPECT_EQ(r[1].first, "core_palette");
  EXPECT_EQ(r[1].second, "rich");
}

} // namespace firelight::settings

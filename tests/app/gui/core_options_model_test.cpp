#include "gui/models/core_options_model.hpp"

#include <firelight/settings/settings_service.hpp>
#include <firelight/settings/sqlite_core_option_repository.hpp>
#include <firelight/settings/sqlite_settings_repository.hpp>
#include <service_accessor.hpp>

#include <gtest/gtest.h>

namespace firelight::settings {
namespace {

// Platform 3 = Game Boy Advance -> core "mgba_libretro" (PlatformMetadata)
constexpr int GBA_PLATFORM_ID = 3;
const char *GBA_CORE = "mgba_libretro";

CoreOption option(std::string key, std::string defaultValue,
                  std::vector<CoreOptionValue> values,
                  std::string category = "", std::string categoryLabel = "") {
  CoreOption o;
  o.key = std::move(key);
  o.label = "label-" + o.key;
  o.description = "desc-" + o.key;
  o.defaultValue = std::move(defaultValue);
  o.values = std::move(values);
  o.category = std::move(category);
  o.categoryLabel = std::move(categoryLabel);
  return o;
}

int roleFor(const CoreOptionsModel &model, const QByteArray &name) {
  const auto roles = model.roleNames();
  for (auto it = roles.cbegin(); it != roles.cend(); ++it) {
    if (it.value() == name) {
      return it.key();
    }
  }
  return -1;
}
} // namespace

class CoreOptionsModelTest : public testing::Test {
protected:
  SqliteSettingsRepository m_repo{":memory:"};
  SettingsService m_service{m_repo};
  SqliteCoreOptionRepository m_coreOptions{":memory:"};

  void SetUp() override {
    SettingsService::setInstance(&m_service);
    ServiceAccessor::setCoreOptionRepository(&m_coreOptions);
    m_coreOptions.upsertCoreOptions(
        GBA_CORE,
        {option("mgba_color_correction", "OFF",
                {{"OFF", "Off"}, {"GBA", "Game Boy Advance"}}),
         option("mgba_frameskip", "disabled", {{"disabled", "Disabled"}})});
  }

  void TearDown() override {
    ServiceAccessor::setCoreOptionRepository(nullptr);
    SettingsService::setInstance(nullptr);
  }

  QVariant value(const CoreOptionsModel &model, int row,
                 const QByteArray &role) {
    return model.data(model.index(row), roleFor(model, role));
  }
};

TEST_F(CoreOptionsModelTest, LoadsCachedOptionsForPlatformCore) {
  CoreOptionsModel model;
  model.setLevel(Global);
  model.setPlatformId(GBA_PLATFORM_ID);

  ASSERT_EQ(model.rowCount({}), 2);
  EXPECT_EQ(value(model, 0, "key").toString(), "mgba_color_correction");
  EXPECT_EQ(value(model, 0, "label").toString(), "label-mgba_color_correction");
  EXPECT_EQ(value(model, 0, "defaultValue").toString(), "OFF");
  // No override yet -> shows the core's declared default, not overridden
  EXPECT_EQ(value(model, 0, "value").toString(), "OFF");
  EXPECT_FALSE(value(model, 0, "overridden").toBool());
}

TEST_F(CoreOptionsModelTest, FiltersByCategoryAndListsCategories) {
  m_coreOptions.upsertCoreOptions(
      GBA_CORE,
      {option("top_opt", "a", {{"a", "A"}}),
       option("vid_opt", "x", {{"x", "X"}}, "video", "Video")});

  CoreOptionsModel model;
  model.setLevel(Global);
  model.setPlatformId(GBA_PLATFORM_ID);

  // Default filter "" -> only uncategorized options
  ASSERT_EQ(model.rowCount({}), 1);
  EXPECT_EQ(value(model, 0, "key").toString(), "top_opt");

  // categories() lists the distinct declared category
  const auto cats = model.categories();
  ASSERT_EQ(cats.size(), 1);
  EXPECT_EQ(cats[0].toHash().value("key").toString(), "video");
  EXPECT_EQ(cats[0].toHash().value("label").toString(), "Video");

  // Filtering to the category shows only its options
  model.setCategoryFilter("video");
  ASSERT_EQ(model.rowCount({}), 1);
  EXPECT_EQ(value(model, 0, "key").toString(), "vid_opt");
}

TEST_F(CoreOptionsModelTest, UnknownCoreIsEmpty) {
  CoreOptionsModel model;
  model.setLevel(Global);
  model.setPlatformId(9999); // no core mapping
  EXPECT_EQ(model.rowCount({}), 0);
}

TEST_F(CoreOptionsModelTest, SetDataWritesOverrideAtLevel) {
  CoreOptionsModel model;
  model.setLevel(Global);
  model.setPlatformId(GBA_PLATFORM_ID);

  const int valueRole = roleFor(model, "value");
  ASSERT_TRUE(model.setData(model.index(0), "GBA", valueRole));

  EXPECT_EQ(value(model, 0, "value").toString(), "GBA");
  EXPECT_TRUE(value(model, 0, "overridden").toBool());
  // Persisted to the global tier
  EXPECT_EQ(m_service.getValueAtLevel(Global, "", GBA_PLATFORM_ID,
                                      "mgba_color_correction")
                .value_or(""),
            "GBA");
}

TEST_F(CoreOptionsModelTest, GameOverrideShadowsGlobalWhenViewedAtGame) {
  const std::string hash = "game-hash";
  m_service.setGlobalValue("mgba_color_correction", "GBA");

  CoreOptionsModel model;
  model.setPlatformId(GBA_PLATFORM_ID);
  model.setContentHash(QString::fromStdString(hash));
  model.setLevel(Game);

  // At the game tier with no game override, the effective value is the global
  // one, but it's not overridden *here*
  EXPECT_EQ(value(model, 0, "value").toString(), "GBA");
  EXPECT_FALSE(value(model, 0, "overridden").toBool());

  // Set a game override; it now shadows global and is overridden here
  const int valueRole = roleFor(model, "value");
  ASSERT_TRUE(model.setData(model.index(0), "OFF", valueRole));
  EXPECT_EQ(value(model, 0, "value").toString(), "OFF");
  EXPECT_TRUE(value(model, 0, "overridden").toBool());
}

TEST_F(CoreOptionsModelTest, ResetValueFallsBackToInherited) {
  const std::string hash = "game-hash";
  m_service.setGlobalValue("mgba_color_correction", "GBA");

  CoreOptionsModel model;
  model.setPlatformId(GBA_PLATFORM_ID);
  model.setContentHash(QString::fromStdString(hash));
  model.setLevel(Game);

  const int valueRole = roleFor(model, "value");
  ASSERT_TRUE(model.setData(model.index(0), "OFF", valueRole));
  ASSERT_TRUE(value(model, 0, "overridden").toBool());

  model.resetValue(0);
  // Falls back to the inherited global value
  EXPECT_EQ(value(model, 0, "value").toString(), "GBA");
  EXPECT_FALSE(value(model, 0, "overridden").toBool());
}

} // namespace firelight::settings

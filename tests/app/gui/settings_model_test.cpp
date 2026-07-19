#include "gui/models/settings_model.hpp"

#include "service_accessor.hpp"

#include <firelight/library/sqlite_user_library.hpp>
#include <firelight/library/user_library_service.hpp>
#include <firelight/settings/settings_catalog.hpp>
#include <firelight/settings/settings_service.hpp>
#include <firelight/settings/sqlite_settings_repository.hpp>

#include <QDir>
#include <gtest/gtest.h>

namespace firelight::settings {
namespace {

// Platform 3 = Game Boy Advance -> core "mgba_libretro"
constexpr int GBA_PLATFORM_ID = 3;

const char *CATALOG = R"JSON(
{
  "groups": [{"id": "emulation", "label": "Emulation"}],
  "common": [
    {"key": "rewind-enabled", "label": "Rewind", "group": "emulation",
     "type": "boolean", "default": "true"},
    {"key": "aspect-ratio", "label": "Aspect ratio", "group": "emulation",
     "type": "options", "default": "corrected",
     "options": [{"label": "Pixel", "value": "pixel"},
                 {"label": "Corrected", "value": "corrected"}]}
  ],
  "cores": {
    "mgba_libretro": {
      "settings": [
        {"key": "solar-sensor", "label": "Solar sensor", "group": "emulation",
         "type": "boolean", "default": "false"},
        {"key": "solar-level", "label": "Solar level", "group": "emulation",
         "type": "slider", "default": "0", "min": 0, "max": 10, "step": 1,
         "visibleWhen": [{"key": "solar-sensor", "values": ["true"]}]},
        {"key": "adv-opt", "label": "Advanced option", "group": "emulation",
         "type": "boolean", "default": "false", "advanced": true}
      ]
    }
  }
}
)JSON";

int roleFor(const SettingsModel &model, const QByteArray &name) {
  const auto roles = model.roleNames();
  for (auto it = roles.cbegin(); it != roles.cend(); ++it) {
    if (it.value() == name) {
      return it.key();
    }
  }
  return -1;
}

int findRow(const SettingsModel &model, const QString &key) {
  const int keyRole = roleFor(model, "key");
  for (int i = 0; i < model.rowCount({}); ++i) {
    if (model.data(model.index(i), keyRole).toString() == key) {
      return i;
    }
  }
  return -1;
}
} // namespace

class SettingsModelTest : public testing::Test {
protected:
  SqliteSettingsRepository m_repo{":memory:"};
  SettingsService m_service{m_repo};

  void SetUp() override {
    SettingsService::setInstance(&m_service);
    ASSERT_TRUE(SettingsCatalog::instance().loadFromJson(CATALOG));
  }

  void TearDown() override {
    SettingsCatalog::instance().loadFromJson("{}");
    SettingsService::setInstance(nullptr);
    // Game-picker tests wire a library service; make sure it doesn't leak
    ServiceAccessor::setLibraryService(nullptr);
  }

  QVariant value(const SettingsModel &model, int row, const QByteArray &role) {
    return model.data(model.index(row), roleFor(model, role));
  }
};

TEST_F(SettingsModelTest, GlobalShowsCommonSettingsOnly) {
  SettingsModel model;
  model.setGroup("emulation");
  model.setLevel(Global);
  // No platform -> only the common settings
  EXPECT_EQ(model.rowCount({}), 2);
}

TEST_F(SettingsModelTest, PlatformShowsCommonPlusCoreSettings) {
  SettingsModel model;
  model.setGroup("emulation");
  model.setPlatformId(GBA_PLATFORM_ID);
  model.setLevel(Platform);
  EXPECT_EQ(model.rowCount({}), 5); // 2 common + 3 core (incl. advanced)
  EXPECT_NE(findRow(model, "solar-sensor"), -1);
}

TEST_F(SettingsModelTest, ExposesWidgetAndSliderBounds) {
  SettingsModel model;
  model.setGroup("emulation");
  model.setPlatformId(GBA_PLATFORM_ID);
  model.setLevel(Platform);

  const int rewind = findRow(model, "rewind-enabled");
  ASSERT_NE(rewind, -1);
  EXPECT_EQ(value(model, rewind, "widget").toString(), "toggle");
  EXPECT_EQ(value(model, rewind, "value").toBool(), true); // default true

  const int level = findRow(model, "solar-level");
  ASSERT_NE(level, -1);
  EXPECT_EQ(value(model, level, "widget").toString(), "slider");
  EXPECT_DOUBLE_EQ(value(model, level, "maximumValue").toDouble(), 10.0);
  EXPECT_DOUBLE_EQ(value(model, level, "stepValue").toDouble(), 1.0);
}

TEST_F(SettingsModelTest, ShowsInheritedValueThenGameOverride) {
  m_service.setPlatformValue(GBA_PLATFORM_ID, "aspect-ratio", "pixel");

  SettingsModel model;

  model.setGroup("emulation");
  model.setPlatformId(GBA_PLATFORM_ID);
  model.setContentHash("hash1");
  model.setLevel(Game);

  const int row = findRow(model, "aspect-ratio");
  ASSERT_NE(row, -1);
  // Inherited from platform; not overridden at the game tier
  EXPECT_EQ(value(model, row, "value").toString(), "pixel");
  EXPECT_FALSE(value(model, row, "resettable").toBool());

  const int valueRole = roleFor(model, "value");
  ASSERT_TRUE(model.setData(model.index(row), "corrected", valueRole));
  EXPECT_EQ(value(model, row, "value").toString(), "corrected");
  EXPECT_TRUE(value(model, row, "resettable").toBool());
  EXPECT_EQ(m_service.getValueAtLevel(Game, "hash1", GBA_PLATFORM_ID, "aspect-ratio").value_or(""), "corrected");
}

TEST_F(SettingsModelTest, NotResettableAtTheGlobalTier) {
  // Global is the base tier for emulation settings: nothing sits below it, so
  // there's nothing to fall back to and no Reset
  SettingsModel model;
  model.setGroup("emulation");
  model.setLevel(Global);

  const int row = findRow(model, "aspect-ratio");
  ASSERT_NE(row, -1);
  EXPECT_FALSE(value(model, row, "resettable").toBool());

  ASSERT_TRUE(model.setData(model.index(row), "pixel", roleFor(model, "value")));
  EXPECT_FALSE(value(model, row, "resettable").toBool());
}

TEST_F(SettingsModelTest, NotResettableWhenTheOverrideMatchesWhatItInherits) {
  // Setting a value and putting it back leaves an override that changes
  // nothing. Offering to clear it is noise: Reset would do nothing visible
  m_service.setPlatformValue(GBA_PLATFORM_ID, "aspect-ratio", "pixel");

  SettingsModel model;

  model.setGroup("emulation");
  model.setPlatformId(GBA_PLATFORM_ID);
  model.setContentHash("hash1");
  model.setLevel(Game);

  const int row = findRow(model, "aspect-ratio");
  const int valueRole = roleFor(model, "value");

  ASSERT_TRUE(model.setData(model.index(row), "corrected", valueRole));
  EXPECT_TRUE(value(model, row, "resettable").toBool());

  ASSERT_TRUE(model.setData(model.index(row), "pixel", valueRole));
  EXPECT_FALSE(value(model, row, "resettable").toBool());
}

TEST_F(SettingsModelTest, MarksRowsThatDependOnAnotherShownRow) {
  SettingsModel model;
  model.setGroup("emulation");
  model.setPlatformId(GBA_PLATFORM_ID);
  model.setContentHash("hash1");
  model.setLevel(Game);

  // solar-level is visibleWhen solar-sensor, which is right here
  const int dependent = findRow(model, "solar-level");
  ASSERT_NE(dependent, -1);
  EXPECT_TRUE(value(model, dependent, "subItem").toBool());

  // The row it depends on is not itself a sub-item
  const int parent = findRow(model, "solar-sensor");
  ASSERT_NE(parent, -1);
  EXPECT_FALSE(value(model, parent, "subItem").toBool());

  const int independent = findRow(model, "aspect-ratio");
  ASSERT_NE(independent, -1);
  EXPECT_FALSE(value(model, independent, "subItem").toBool());
}

TEST_F(SettingsModelTest, ResetFallsBackToInherited) {
  m_service.setPlatformValue(GBA_PLATFORM_ID, "aspect-ratio", "pixel");

  SettingsModel model;

  model.setGroup("emulation");
  model.setPlatformId(GBA_PLATFORM_ID);
  model.setContentHash("hash1");
  model.setLevel(Game);

  const int row = findRow(model, "aspect-ratio");
  const int valueRole = roleFor(model, "value");
  ASSERT_TRUE(model.setData(model.index(row), "corrected", valueRole));
  ASSERT_TRUE(value(model, row, "resettable").toBool());

  model.resetValue(row);
  EXPECT_EQ(value(model, row, "value").toString(), "pixel");
  EXPECT_FALSE(value(model, row, "resettable").toBool());
}

TEST_F(SettingsModelTest, VisibleWhenTracksDependency) {
  SettingsModel model;
  model.setGroup("emulation");
  model.setPlatformId(GBA_PLATFORM_ID);
  model.setContentHash("hash1");
  model.setLevel(Game);

  const int sensor = findRow(model, "solar-sensor");
  const int level = findRow(model, "solar-level");
  ASSERT_NE(sensor, -1);
  ASSERT_NE(level, -1);

  // solar-sensor defaults false -> solar-level hidden
  EXPECT_FALSE(value(model, level, "visible").toBool());

  // Turn the sensor on -> the dependent slider becomes visible
  const int valueRole = roleFor(model, "value");
  ASSERT_TRUE(model.setData(model.index(sensor), true, valueRole));
  EXPECT_TRUE(value(model, level, "visible").toBool());
}

TEST_F(SettingsModelTest, AdvancedHiddenUnlessShowAdvanced) {
  SettingsModel model;
  model.setGroup("emulation");
  model.setPlatformId(GBA_PLATFORM_ID);
  model.setLevel(Platform);

  const int adv = findRow(model, "adv-opt");
  ASSERT_NE(adv, -1);

  // Advanced settings are hidden by default (showAdvanced defaults false)
  EXPECT_FALSE(value(model, adv, "visible").toBool());

  model.setShowAdvanced(true);
  EXPECT_TRUE(value(model, adv, "visible").toBool());

  model.setShowAdvanced(false);
  EXPECT_FALSE(value(model, adv, "visible").toBool());
}

// A game-picker setting sourced from the library: platform 3's core is
// mgba_libretro (see CoreRegistry), so a game-picker declared there surfaces
// when the model is scoped to platform 3
const char *GAME_PICKER_CATALOG = R"JSON(
{
  "common": [],
  "cores": {
    "mgba_libretro": {
      "settings": [
        {"key": "tpak", "label": "Cartridge", "group": "emulation",
         "type": "game-picker", "eligiblePlatformIds": [1], "default": ""}
      ]
    }
  }
}
)JSON";

TEST_F(SettingsModelTest, GamePickerOptionsFromLibraryFilteredByPlatform) {
  ASSERT_TRUE(SettingsCatalog::instance().loadFromJson(GAME_PICKER_CATALOG));

  // Seed a library with one eligible (Game Boy, platform 1) and one ineligible
  // (SNES, platform 4) entry
  library::SqliteUserLibraryRepository repo(":memory:");
  library::UserLibraryService lib(repo, (QDir::tempPath() + "/fl_tpak_test").toStdString());
  library::Entry gb{.displayName = "Tetris", .contentHash = "gbhash", .platformId = 1};
  library::Entry snes{.displayName = "Zelda", .contentHash = "sneshash", .platformId = 4};
  ASSERT_TRUE(repo.createEntry(gb));
  ASSERT_TRUE(repo.createEntry(snes));
  ServiceAccessor::setLibraryService(&lib);

  SettingsModel model;

  model.setGroup("emulation");
  model.setPlatformId(GBA_PLATFORM_ID); // -> mgba_libretro
  model.setLevel(Platform);

  const int row = findRow(model, "tpak");
  ASSERT_NE(row, -1);
  EXPECT_EQ(value(model, row, "widget").toString(), "dropdown");

  const auto options = value(model, row, "options").value<QVector<QVariantHash>>();
  // "None" plus only the eligible Game Boy game (SNES filtered out)
  ASSERT_EQ(options.size(), 2);
  EXPECT_EQ(options[0].value("label").toString(), "None");
  EXPECT_EQ(options[0].value("value").toString(), "");
  EXPECT_EQ(options[1].value("label").toString(), "Tetris");
  EXPECT_EQ(options[1].value("value").toString(), "gbhash"); // content hash
}

const char *NEW_WIDGETS_CATALOG = R"JSON(
{
  "groups": [{"id": "emulation", "label": "Emulation"}],
  "common": [],
  "cores": {
    "mgba_libretro": {
      "settings": [
        {"key": "title", "label": "Title", "group": "emulation", "type": "text",
         "placeholder": "Enter a name", "default": "hi"},
        {"key": "bios", "label": "BIOS", "group": "emulation", "type": "file-picker",
         "extensions": ["bin", "bios"], "default": ""},
        {"key": "romdir", "label": "ROM folder", "group": "emulation", "type": "folder-picker",
         "default": ""},
        {"key": "cheats", "label": "Cheats", "group": "emulation", "type": "multi-select",
         "default": "[]",
         "options": [{"label": "A", "value": "a"}, {"label": "B", "value": "b"}]}
      ]
    }
  }
}
)JSON";

TEST_F(SettingsModelTest, ExposesNewWidgetRolesAndStringValues) {
  ASSERT_TRUE(SettingsCatalog::instance().loadFromJson(NEW_WIDGETS_CATALOG));

  SettingsModel model;

  model.setGroup("emulation");
  model.setPlatformId(GBA_PLATFORM_ID);
  model.setContentHash("hash1");
  model.setLevel(Game);

  const int text = findRow(model, "title");
  ASSERT_NE(text, -1);
  EXPECT_EQ(value(model, text, "widget").toString(), "text");
  EXPECT_EQ(value(model, text, "placeholder").toString(), "Enter a name");
  EXPECT_EQ(value(model, text, "value").toString(), "hi"); // string default

  const int bios = findRow(model, "bios");
  ASSERT_NE(bios, -1);
  EXPECT_EQ(value(model, bios, "widget").toString(), "file-picker");
  EXPECT_FALSE(value(model, bios, "directoryMode").toBool());
  EXPECT_EQ(value(model, bios, "fileExtensions").toStringList(), (QStringList{"bin", "bios"}));

  const int romdir = findRow(model, "romdir");
  ASSERT_NE(romdir, -1);
  EXPECT_EQ(value(model, romdir, "widget").toString(), "folder-picker");
  EXPECT_TRUE(value(model, romdir, "directoryMode").toBool());

  // Multi-select stores its selection as an opaque (JSON array) string
  const int cheats = findRow(model, "cheats");
  ASSERT_NE(cheats, -1);
  EXPECT_EQ(value(model, cheats, "widget").toString(), "multi-select");
  const int valueRole = roleFor(model, "value");
  ASSERT_TRUE(model.setData(model.index(cheats), R"(["a","b"])", valueRole));
  EXPECT_EQ(value(model, cheats, "value").toString(), R"(["a","b"])");
  EXPECT_EQ(m_service.getValueAtLevel(Game, "hash1", GBA_PLATFORM_ID, "cheats").value_or(""), R"(["a","b"])");
}

namespace {
// An app setting and an emulation setting sharing one group — the case the
// group filter exists for
const char *GROUP_CATALOG = R"JSON(
{
  "pages": [
    {"id": "appearance", "label": "Appearance", "route": "/settings/appearance"}
  ],
  "groups": [
    {"id": "theme", "page": "appearance", "label": "Theme", "order": 10},
    {"id": "other", "page": "appearance", "label": "Other", "order": 20}
  ],
  "app": [
    {"key": "accent-color", "group": "theme", "label": "Accent color",
     "type": "color", "default": "#f76b15", "order": 20},
    {"key": "elsewhere", "group": "other", "label": "Elsewhere",
     "type": "boolean", "default": "false"}
  ],
  "common": [
    {"key": "in-theme-too", "group": "theme", "label": "In theme too",
     "type": "boolean", "default": "true", "order": 10}
  ]
}
)JSON";
} // namespace

class SettingsModelGroupTest : public SettingsModelTest {
protected:
  void SetUp() override {
    SettingsService::setInstance(&m_service);
    ASSERT_TRUE(SettingsCatalog::instance().loadFromJson(GROUP_CATALOG));
  }
};

TEST_F(SettingsModelGroupTest, ShowsOnlyTheGroupInDeclaredOrder) {
  SettingsModel model;
  model.setGroup("emulation");
  model.setGroup("theme");

  ASSERT_EQ(model.rowCount({}), 2);
  // Ordered by `order`, across both arrays
  EXPECT_EQ(value(model, 0, "key").toString(), "in-theme-too");
  EXPECT_EQ(value(model, 1, "key").toString(), "accent-color");
  EXPECT_EQ(findRow(model, "elsewhere"), -1);
}

TEST_F(SettingsModelGroupTest, ExposesTheGroupTitle) {
  SettingsModel model;
  model.setGroup("emulation");
  model.setGroup("theme");
  EXPECT_EQ(model.getGroupLabel(), "Theme");

  model.setGroup("nope");
  EXPECT_EQ(model.getGroupLabel(), QString());
  EXPECT_EQ(model.rowCount({}), 0);
}

TEST_F(SettingsModelGroupTest, AppSettingReadsAndWritesGlobalWithoutALevel) {
  // No level set (Unknown) — an app setting still resolves, because it's
  // pinned to the global tier rather than following the tier chain
  SettingsModel model;
  model.setGroup("emulation");
  model.setGroup("theme");

  const int row = findRow(model, "accent-color");
  ASSERT_NE(row, -1);
  EXPECT_EQ(value(model, row, "value").toString(), "#f76b15");

  ASSERT_TRUE(model.setData(model.index(row), "#00ff00", roleFor(model, "value")));
  EXPECT_EQ(m_service.getGlobalValue("accent-color").value_or(""), "#00ff00");
  EXPECT_EQ(value(model, row, "value").toString(), "#00ff00");
}

TEST_F(SettingsModelGroupTest, AppSettingIsNeverResettable) {
  // Reset means "fall back to what I inherit", and an app setting inherits
  // from nothing — so changing one is just changing it, with no Reset to
  // clutter the row
  SettingsModel model;
  model.setGroup("emulation");
  model.setGroup("theme");

  const int row = findRow(model, "accent-color");
  ASSERT_NE(row, -1);
  EXPECT_FALSE(value(model, row, "resettable").toBool());

  ASSERT_TRUE(model.setData(model.index(row), "#00ff00", roleFor(model, "value")));
  EXPECT_FALSE(value(model, row, "resettable").toBool());
}

TEST_F(SettingsModelGroupTest, AppSettingIgnoresTheModelLevel) {
  // Even asked for the Game tier, an app setting writes global: nothing
  // overrides it per-game
  SettingsModel model;
  model.setGroup("emulation");
  model.setGroup("theme");
  model.setLevel(Game);
  model.setContentHash("abc");

  const int row = findRow(model, "accent-color");
  ASSERT_NE(row, -1);
  ASSERT_TRUE(model.setData(model.index(row), "#123456", roleFor(model, "value")));

  EXPECT_EQ(m_service.getGlobalValue("accent-color").value_or(""), "#123456");
  EXPECT_FALSE(m_service.getValueAtLevel(Game, "abc", -1, "accent-color").has_value());
}

TEST_F(SettingsModelGroupTest, AppSettingResetFallsBackToCatalogDefault) {
  SettingsModel model;
  model.setGroup("emulation");
  model.setGroup("theme");

  const int row = findRow(model, "accent-color");
  ASSERT_NE(row, -1);
  ASSERT_TRUE(model.setData(model.index(row), "#00ff00", roleFor(model, "value")));

  // No Reset row affordance, but resetValue still works — the CLI and any
  // future "restore defaults" both go through it
  model.resetValue(row);
  EXPECT_EQ(value(model, row, "value").toString(), "#f76b15");
  EXPECT_FALSE(m_service.getGlobalValue("accent-color").has_value());
}

TEST_F(SettingsModelGroupTest, EmulationSettingInAGroupStillNeedsALevel) {
  // The emulation row shares the group but is tiered; with no level it can't
  // resolve, while the app row alongside it still does
  SettingsModel model;
  model.setGroup("emulation");
  model.setGroup("theme");
  const int row = findRow(model, "in-theme-too");
  ASSERT_NE(row, -1);
  EXPECT_FALSE(model.setData(model.index(row), false, roleFor(model, "value")));

  model.setLevel(Global);
  EXPECT_TRUE(model.setData(model.index(row), false, roleFor(model, "value")));
  EXPECT_EQ(m_service.getGlobalValue("in-theme-too").value_or(""), "false");
}

TEST_F(SettingsModelTest, GamePickerWithoutLibraryHasOnlyNone) {
  ASSERT_TRUE(SettingsCatalog::instance().loadFromJson(GAME_PICKER_CATALOG));
  // No library service wired (TearDown clears it) -> just the "None" option
  ServiceAccessor::setLibraryService(nullptr);

  SettingsModel model;

  model.setGroup("emulation");
  model.setPlatformId(GBA_PLATFORM_ID);
  model.setLevel(Platform);

  const int row = findRow(model, "tpak");
  ASSERT_NE(row, -1);
  const auto options = value(model, row, "options").value<QVector<QVariantHash>>();
  ASSERT_EQ(options.size(), 1);
  EXPECT_EQ(options[0].value("label").toString(), "None");
}

} // namespace firelight::settings

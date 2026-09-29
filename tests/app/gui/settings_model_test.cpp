// TODO: NEEDS REVIEW
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
  "groups": [{"id": "emulation", "label": "Emulation",
              "settings": ["rewind-enabled", "aspect-ratio", "solar-sensor", "solar-level", "adv-opt"]},
             {"id": "rowstyle", "label": "Row style",
              "settings": ["rowstyle-parent", "rowstyle-inferred", "rowstyle-optout", "rowstyle-optin",
                           "rowstyle-crossgroup"]}],
  "common": [
    {"key": "rewind-enabled", "label": "Rewind",
     "type": "boolean", "default": "true"},
    {"key": "aspect-ratio", "label": "Aspect ratio",
     "type": "options", "default": "corrected",
     "options": [{"label": "Pixel", "value": "pixel"},
                 {"label": "Corrected", "value": "corrected"}]}
  ],
  "cores": {
    "mgba_libretro": {
      "settings": [
        {"key": "solar-sensor", "label": "Solar sensor",
         "type": "boolean", "default": "false"},
        {"key": "solar-level", "label": "Solar level",
         "type": "slider", "default": "0", "min": 0, "max": 10, "step": 1,
         "visibleWhen": [{"key": "solar-sensor", "values": ["true"]}]},
        {"key": "adv-opt", "label": "Advanced option",
         "type": "boolean", "default": "false", "advanced": true},
        {"key": "rowstyle-parent", "label": "Row style parent",
         "type": "boolean", "default": "true"},
        {"key": "rowstyle-inferred", "label": "Inferred",
         "type": "boolean", "default": "false",
         "visibleWhen": [{"key": "rowstyle-parent", "values": ["true"]}]},
        {"key": "rowstyle-optout", "label": "Opted out",
         "type": "boolean", "default": "false", "subItem": false,
         "visibleWhen": [{"key": "rowstyle-parent", "values": ["true"]}]},
        {"key": "rowstyle-optin", "label": "Opted in",
         "type": "boolean", "default": "false", "subItem": true},
        {"key": "rowstyle-crossgroup", "label": "Cross group",
         "type": "boolean", "default": "false",
         "visibleWhen": [{"key": "rewind-enabled", "values": ["true"]}]}
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

const char *CHECKBOX_CATALOG = R"JSON(
{
  "groups": [{"id": "emulation", "label": "Emulation", "settings": ["show-box"]}],
  "common": [
    {"key": "show-box", "label": "Show box",
     "type": "boolean", "widget": "checkbox", "default": "true"}
  ]
}
)JSON";

TEST_F(SettingsModelTest, ABooleanDrawnAsACheckboxStillHoldsABool) {
  ASSERT_TRUE(SettingsCatalog::instance().loadFromJson(CHECKBOX_CATALOG));

  SettingsModel model;
  model.setGroup("emulation");
  model.setLevel(Global);

  const int row = findRow(model, "show-box");
  ASSERT_NE(row, -1);
  EXPECT_EQ(value(model, row, "widget").toString(), "checkbox");
  EXPECT_EQ(value(model, row, "value").typeId(), QMetaType::Bool);
  EXPECT_TRUE(value(model, row, "value").toBool());

  ASSERT_TRUE(model.setData(model.index(row), false, roleFor(model, "value")));
  EXPECT_EQ(value(model, row, "value").typeId(), QMetaType::Bool);
  EXPECT_FALSE(value(model, row, "value").toBool());
  EXPECT_EQ(m_service.getGlobalValue("show-box").value_or(""), "false");
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

TEST_F(SettingsModelTest, AnExplicitSubItemBeatsWhatTheDependencyImplies) {
  SettingsModel model;
  model.setGroup("rowstyle");
  model.setPlatformId(GBA_PLATFORM_ID);
  model.setContentHash("hash1");
  model.setLevel(Game);

  // The dependency on its own still decides, so the opt-out below is the field talking
  const int inferred = findRow(model, "rowstyle-inferred");
  ASSERT_NE(inferred, -1);
  EXPECT_TRUE(value(model, inferred, "subItem").toBool());

  // Same dependency, opted out
  const int optedOut = findRow(model, "rowstyle-optout");
  ASSERT_NE(optedOut, -1);
  EXPECT_FALSE(value(model, optedOut, "subItem").toBool());

  // Asks to be one while depending on nothing
  const int optedIn = findRow(model, "rowstyle-optin");
  ASSERT_NE(optedIn, -1);
  EXPECT_TRUE(value(model, optedIn, "subItem").toBool());
}

TEST_F(SettingsModelTest, AConditionCanNameASettingFromAnotherGroup) {
  SettingsModel model;
  model.setGroup("rowstyle");
  model.setPlatformId(GBA_PLATFORM_ID);
  model.setContentHash("hash1");
  model.setLevel(Game);

  // rewind-enabled is in the emulation group, so this model does not hold it. It defaults true, which
  // is what the condition wants, so the row belongs on screen
  const int row = findRow(model, "rowstyle-crossgroup");
  ASSERT_NE(row, -1);
  EXPECT_TRUE(value(model, row, "visible").toBool());

  // And it follows that setting afterwards, wherever it was changed from
  m_service.setGlobalValue("rewind-enabled", "false");
  EXPECT_FALSE(value(model, row, "visible").toBool());

  m_service.setGlobalValue("rewind-enabled", "true");
  EXPECT_TRUE(value(model, row, "visible").toBool());
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

// TODO
// Two models showing the same setting, as when a setting sits in two groups on screen: a reset in
// one shows up in the other
TEST_F(SettingsModelTest, AResetInOneModelRefreshesAnotherShowingTheSameSetting) {
  m_service.setPlatformValue(GBA_PLATFORM_ID, "aspect-ratio", "pixel");

  SettingsModel editing;
  SettingsModel watching;

  for (auto *model : {&editing, &watching}) {
    model->setGroup("emulation");
    model->setPlatformId(GBA_PLATFORM_ID);
    model->setContentHash("hash1");
    model->setLevel(Game);
  }

  const int editingRow = findRow(editing, "aspect-ratio");
  const int watchingRow = findRow(watching, "aspect-ratio");
  ASSERT_NE(editingRow, -1);
  ASSERT_NE(watchingRow, -1);

  ASSERT_TRUE(editing.setData(editing.index(editingRow), "corrected", roleFor(editing, "value")));
  ASSERT_EQ(value(watching, watchingRow, "value").toString(), "corrected");

  editing.resetValue(editingRow);
  EXPECT_EQ(value(watching, watchingRow, "value").toString(), "pixel");
  EXPECT_FALSE(value(watching, watchingRow, "resettable").toBool());
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
  "groups": [{"id": "emulation", "label": "Emulation", "settings": ["tpak"]}],
  "common": [],
  "cores": {
    "mgba_libretro": {
      "settings": [
        {"key": "tpak", "label": "Cartridge",
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

  const auto options = value(model, row, "options").toList();
  // "None" plus only the eligible Game Boy game (SNES filtered out)
  ASSERT_EQ(options.size(), 2);
  EXPECT_EQ(options[0].toHash().value("label").toString(), "None");
  EXPECT_EQ(options[0].toHash().value("value").toString(), "");
  EXPECT_EQ(options[1].toHash().value("label").toString(), "Tetris");
  EXPECT_EQ(options[1].toHash().value("value").toString(), "gbhash"); // content hash
}

const char *NEW_WIDGETS_CATALOG = R"JSON(
{
  "groups": [{"id": "emulation", "label": "Emulation", "settings": ["title", "bios", "romdir", "cheats"]},
             {"id": "rowstyle", "label": "Row style"}],
  "common": [],
  "cores": {
    "mgba_libretro": {
      "settings": [
        {"key": "title", "label": "Title", "type": "text",
         "placeholder": "Enter a name", "default": "hi"},
        {"key": "bios", "label": "BIOS", "type": "file-picker",
         "extensions": ["bin", "bios"], "default": ""},
        {"key": "romdir", "label": "ROM folder", "type": "folder-picker",
         "default": ""},
        {"key": "cheats", "label": "Cheats", "type": "multi-select",
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
    {"id": "appearance", "label": "Appearance", "route": "/settings/appearance",
     "groups": ["theme", "other"]}
  ],
  "groups": [
    {"id": "theme", "label": "Theme", "settings": ["in-theme-too", "accent-color"]},
    {"id": "other", "label": "Other", "settings": ["elsewhere"]}
  ],
  "app": [
    {"key": "accent-color", "label": "Accent color",
     "type": "color", "default": "#f76b15"},
    {"key": "elsewhere", "label": "Elsewhere",
     "type": "boolean", "default": "false"}
  ],
  "common": [
    {"key": "in-theme-too", "label": "In theme too",
     "type": "boolean", "default": "true"}
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
  // TODO
  // Ordered by the group's list, across both arrays
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
  const auto options = value(model, row, "options").toList();
  ASSERT_EQ(options.size(), 1);
  EXPECT_EQ(options[0].toHash().value("label").toString(), "None");
}

namespace {
// Two controller settings and an app setting sharing one group
const char *CONTROLLER_CATALOG = R"JSON(
{
  "groups": [
    {"id": "pad", "label": "Rumble & lights",
     "settings": ["rumble-strength", "light-bar-color", "pad-app"]}
  ],
  "app": [
    {"key": "pad-app", "label": "App setting", "type": "boolean", "default": "false"}
  ],
  "controller": [
    {"key": "rumble-strength", "label": "Rumble strength",
     "type": "slider", "min": 0, "max": 100, "step": 5, "default": "100"},
    {"key": "light-bar-color", "label": "Light bar colour", "type": "color", "default": ""}
  ]
}
)JSON";

constexpr int PROFILE_ID = 7;
constexpr int OTHER_PROFILE_ID = 8;
} // namespace

/** SettingsModel rows declared in the catalog's controller array */
class SettingsModelControllerTest : public SettingsModelTest {
protected:
  void SetUp() override {
    SettingsService::setInstance(&m_service);
    ASSERT_TRUE(SettingsCatalog::instance().loadFromJson(CONTROLLER_CATALOG));
  }

  /** Points a model at the controller group for one profile, at the profile's own tier */
  static void showProfile(SettingsModel &model, const int profileId) {
    model.setGroup("pad");
    model.setLevel(Global);
    model.setProfileId(profileId);
  }

  /** Points a model at the controller group for one profile and one game */
  static void showGame(SettingsModel &model, const int profileId, const QString &contentHash) {
    model.setGroup("pad");
    model.setContentHash(contentHash);
    model.setLevel(Game);
    model.setProfileId(profileId);
  }
};

TEST_F(SettingsModelControllerTest, ExposesTheSliderAndColorWidgets) {
  SettingsModel model;
  showProfile(model, PROFILE_ID);
  ASSERT_EQ(model.rowCount({}), 3);

  const int rumble = findRow(model, "rumble-strength");
  ASSERT_NE(rumble, -1);
  EXPECT_EQ(value(model, rumble, "widget").toString(), "slider");
  EXPECT_DOUBLE_EQ(value(model, rumble, "minimumValue").toDouble(), 0.0);
  EXPECT_DOUBLE_EQ(value(model, rumble, "maximumValue").toDouble(), 100.0);
  EXPECT_DOUBLE_EQ(value(model, rumble, "stepValue").toDouble(), 5.0);
  EXPECT_EQ(value(model, rumble, "defaultValue").toString(), "100");

  const int light = findRow(model, "light-bar-color");
  ASSERT_NE(light, -1);
  EXPECT_EQ(value(model, light, "widget").toString(), "color");
  EXPECT_EQ(value(model, light, "defaultValue").toString(), "");
  EXPECT_EQ(value(model, light, "value").toString(), "");
}

TEST_F(SettingsModelControllerTest, ShowsTheDefaultThenTheStoredValue) {
  SettingsModel model;
  showProfile(model, PROFILE_ID);

  const int row = findRow(model, "rumble-strength");
  ASSERT_NE(row, -1);
  EXPECT_EQ(value(model, row, "value").toString(), "100");
  EXPECT_FALSE(value(model, row, "resettable").toBool());

  ASSERT_TRUE(m_service.setControllerValue("", PROFILE_ID, "rumble-strength", "40"));
  EXPECT_EQ(value(model, row, "value").toString(), "40");
  EXPECT_TRUE(value(model, row, "resettable").toBool());

  SettingsModel fresh;
  showProfile(fresh, PROFILE_ID);
  EXPECT_EQ(value(fresh, findRow(fresh, "rumble-strength"), "value").toString(), "40");
}

TEST_F(SettingsModelControllerTest, WritesTheProfileTierAtTheGlobalLevel) {
  SettingsModel model;
  showProfile(model, PROFILE_ID);

  const int row = findRow(model, "rumble-strength");
  ASSERT_NE(row, -1);
  ASSERT_TRUE(model.setData(model.index(row), "50", roleFor(model, "value")));

  EXPECT_EQ(m_service.getControllerValue("", PROFILE_ID, "rumble-strength").value_or(""), "50");
  EXPECT_FALSE(m_service.getControllerValue("", OTHER_PROFILE_ID, "rumble-strength").has_value());
  EXPECT_FALSE(m_service.getGlobalValue("rumble-strength").has_value());
  EXPECT_EQ(value(model, row, "value").toString(), "50");
  EXPECT_TRUE(value(model, row, "resettable").toBool());
}

TEST_F(SettingsModelControllerTest, WritesTheGameTierAtTheGameLevel) {
  SettingsModel model;
  showGame(model, PROFILE_ID, "abc");

  const int row = findRow(model, "light-bar-color");
  ASSERT_NE(row, -1);
  ASSERT_TRUE(model.setData(model.index(row), "#ff0000", roleFor(model, "value")));

  EXPECT_EQ(m_service.getControllerValue("abc", PROFILE_ID, "light-bar-color").value_or(""), "#ff0000");
  EXPECT_FALSE(m_service.getControllerValue("", PROFILE_ID, "light-bar-color").has_value());
  EXPECT_FALSE(m_service.getGameValue("abc", "light-bar-color").has_value());
  EXPECT_EQ(value(model, row, "value").toString(), "#ff0000");
  EXPECT_TRUE(value(model, row, "resettable").toBool());
}

TEST_F(SettingsModelControllerTest, AGameLevelWithoutAContentHashWritesTheProfileTier) {
  SettingsModel model;
  showGame(model, PROFILE_ID, "");

  const int row = findRow(model, "rumble-strength");
  ASSERT_NE(row, -1);
  ASSERT_TRUE(model.setData(model.index(row), "25", roleFor(model, "value")));
  EXPECT_EQ(m_service.getControllerValue("", PROFILE_ID, "rumble-strength").value_or(""), "25");
}

TEST_F(SettingsModelControllerTest, TheGameTierFallsBackToTheProfileValue) {
  ASSERT_TRUE(m_service.setControllerValue("", PROFILE_ID, "rumble-strength", "60"));

  SettingsModel model;
  showGame(model, PROFILE_ID, "abc");
  const int row = findRow(model, "rumble-strength");
  ASSERT_NE(row, -1);
  EXPECT_EQ(value(model, row, "value").toString(), "60");
  EXPECT_FALSE(value(model, row, "resettable").toBool());

  // TODO
  // A game value matching the profile value is not resettable
  ASSERT_TRUE(model.setData(model.index(row), "60", roleFor(model, "value")));
  EXPECT_FALSE(value(model, row, "resettable").toBool());

  ASSERT_TRUE(model.setData(model.index(row), "20", roleFor(model, "value")));
  EXPECT_EQ(value(model, row, "value").toString(), "20");
  EXPECT_TRUE(value(model, row, "resettable").toBool());
}

TEST_F(SettingsModelControllerTest, NothingIsWrittenWithoutAProfile) {
  for (const int profileId : {-1, 0}) {
    SettingsModel model;
    showProfile(model, profileId);

    const int row = findRow(model, "rumble-strength");
    ASSERT_NE(row, -1);
    EXPECT_EQ(value(model, row, "value").toString(), "100");
    EXPECT_FALSE(model.setData(model.index(row), "10", roleFor(model, "value")));
    EXPECT_FALSE(m_service.getControllerValue("", profileId, "rumble-strength").has_value());
    EXPECT_FALSE(value(model, row, "resettable").toBool());
  }
}

TEST_F(SettingsModelControllerTest, TheAppSettingAlongsideIgnoresTheProfile) {
  SettingsModel model;
  showProfile(model, PROFILE_ID);

  const int row = findRow(model, "pad-app");
  ASSERT_NE(row, -1);
  ASSERT_TRUE(model.setData(model.index(row), true, roleFor(model, "value")));
  EXPECT_EQ(m_service.getGlobalValue("pad-app").value_or(""), "true");
  EXPECT_FALSE(m_service.getControllerValue("", PROFILE_ID, "pad-app").has_value());
}

TEST_F(SettingsModelControllerTest, ResetFallsBackToTheDefaultAtTheProfileTier) {
  SettingsModel model;
  showProfile(model, PROFILE_ID);

  const int row = findRow(model, "light-bar-color");
  ASSERT_NE(row, -1);
  ASSERT_TRUE(model.setData(model.index(row), "#00ff00", roleFor(model, "value")));
  ASSERT_TRUE(value(model, row, "resettable").toBool());

  model.resetValue(row);
  EXPECT_EQ(value(model, row, "value").toString(), "");
  EXPECT_FALSE(value(model, row, "resettable").toBool());
  EXPECT_FALSE(m_service.getControllerValue("", PROFILE_ID, "light-bar-color").has_value());
}

TEST_F(SettingsModelControllerTest, ResetFallsBackToTheProfileValueAtTheGameTier) {
  ASSERT_TRUE(m_service.setControllerValue("", PROFILE_ID, "rumble-strength", "60"));

  SettingsModel model;
  showGame(model, PROFILE_ID, "abc");
  const int row = findRow(model, "rumble-strength");
  ASSERT_NE(row, -1);
  ASSERT_TRUE(model.setData(model.index(row), "20", roleFor(model, "value")));

  model.resetValue(row);
  EXPECT_EQ(value(model, row, "value").toString(), "60");
  EXPECT_FALSE(value(model, row, "resettable").toBool());
  EXPECT_FALSE(m_service.getControllerValue("abc", PROFILE_ID, "rumble-strength").has_value());
  EXPECT_EQ(m_service.getControllerValue("", PROFILE_ID, "rumble-strength").value_or(""), "60");
}

TEST_F(SettingsModelControllerTest, ChangingTheProfileRefreshesTheValues) {
  ASSERT_TRUE(m_service.setControllerValue("", PROFILE_ID, "rumble-strength", "40"));

  SettingsModel model;
  showProfile(model, PROFILE_ID);
  const int row = findRow(model, "rumble-strength");
  ASSERT_NE(row, -1);
  EXPECT_EQ(value(model, row, "value").toString(), "40");
  EXPECT_TRUE(value(model, row, "resettable").toBool());

  model.setProfileId(OTHER_PROFILE_ID);
  EXPECT_EQ(model.getProfileId(), OTHER_PROFILE_ID);
  EXPECT_EQ(findRow(model, "rumble-strength"), row);
  EXPECT_EQ(value(model, row, "value").toString(), "100");
  EXPECT_FALSE(value(model, row, "resettable").toBool());

  model.setProfileId(PROFILE_ID);
  EXPECT_EQ(value(model, row, "value").toString(), "40");

  model.setProfileId(-1);
  EXPECT_EQ(value(model, row, "value").toString(), "100");
  EXPECT_FALSE(value(model, row, "resettable").toBool());
}

TEST_F(SettingsModelControllerTest, AChangeInAnotherModelRefreshesThisOne) {
  SettingsModel editing;
  SettingsModel watching;
  SettingsModel inGame;
  SettingsModel otherProfile;
  showProfile(editing, PROFILE_ID);
  showProfile(watching, PROFILE_ID);
  showGame(inGame, PROFILE_ID, "abc");
  showProfile(otherProfile, OTHER_PROFILE_ID);

  const int editingRow = findRow(editing, "rumble-strength");
  ASSERT_NE(editingRow, -1);
  ASSERT_TRUE(editing.setData(editing.index(editingRow), "25", roleFor(editing, "value")));

  EXPECT_EQ(value(watching, findRow(watching, "rumble-strength"), "value").toString(), "25");
  EXPECT_EQ(value(inGame, findRow(inGame, "rumble-strength"), "value").toString(), "25");
  EXPECT_EQ(value(otherProfile, findRow(otherProfile, "rumble-strength"), "value").toString(), "100");

  editing.resetValue(editingRow);
  EXPECT_EQ(value(watching, findRow(watching, "rumble-strength"), "value").toString(), "100");
  EXPECT_FALSE(value(watching, findRow(watching, "rumble-strength"), "resettable").toBool());
  EXPECT_EQ(value(inGame, findRow(inGame, "rumble-strength"), "value").toString(), "100");
}

} // namespace firelight::settings

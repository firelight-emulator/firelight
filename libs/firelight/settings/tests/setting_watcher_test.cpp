#include <firelight/settings/setting_watcher.hpp>
#include <firelight/settings/settings_catalog.hpp>
#include <firelight/settings/settings_service.hpp>
#include <firelight/settings/sqlite_settings_repository.hpp>

#include <gtest/gtest.h>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace firelight::settings {

namespace {
// Two catalog keys, so a change on one can be shown not to reach a watcher on the other
auto CATALOG = R"JSON(
{
  "common": [
    {"key": "watched", "label": "Watched", "type": "boolean", "default": "false"},
    {"key": "other", "label": "Other", "type": "boolean", "default": "false"}
  ]
}
)JSON";
} // namespace

/**
 * A service on an in-memory repository, the shared catalog loaded with the two keys above, and a record of every value
 * a watcher handed to its callback
 */
class SettingWatcherTest : public testing::Test {
protected:
  std::unique_ptr<SqliteSettingsRepository> repository;
  std::unique_ptr<SettingsService> service;
  std::vector<std::string> applied;

  void SetUp() override {
    ASSERT_TRUE(SettingsCatalog::instance().loadFromJson(CATALOG));
    repository = std::make_unique<SqliteSettingsRepository>(":memory:");
    service = std::make_unique<SettingsService>(*repository);
  }

  void TearDown() override {
    service.reset();
    repository.reset();
    SettingsCatalog::instance().loadFromJson("{}");
  }

  /**
   * A watcher on the given key, writes values to applied vector
   */
  std::unique_ptr<SettingWatcher> watch(const std::string &key) {
    return std::make_unique<SettingWatcher>(*service, key,
                                            [this](const std::string &value) { applied.push_back(value); });
  }
};

//****************
// construction
//****************

TEST_F(SettingWatcherTest, AppliesTheCatalogDefaultWhenNothingIsStored) {
  const auto watcher = watch("watched");

  EXPECT_EQ(applied, (std::vector<std::string>{"false"}));
}

TEST_F(SettingWatcherTest, AppliesTheStoredValueWhenThereIsOne) {
  ASSERT_TRUE(service->setGlobalValue("watched", "true"));

  const auto watcher = watch("watched");

  EXPECT_EQ(applied, (std::vector<std::string>{"true"}));
}

TEST_F(SettingWatcherTest, AppliesASessionOverrideOverTheStoredValue) {
  ASSERT_TRUE(service->setGlobalValue("watched", "false"));
  service->setSessionOverride("watched", "true");

  const auto watcher = watch("watched");

  EXPECT_EQ(applied, (std::vector<std::string>{"true"}));
}

TEST_F(SettingWatcherTest, ThrowsForAKeyThatIsNotInTheCatalog) {
  EXPECT_THROW(watch("not-a-setting"), std::runtime_error);
  EXPECT_TRUE(applied.empty());
}

//****************
// changes
//****************

TEST_F(SettingWatcherTest, AppliesAChangedValueOnce) {
  const auto watcher = watch("watched");

  ASSERT_TRUE(service->setGlobalValue("watched", "true"));
  ASSERT_TRUE(service->setGlobalValue("watched", "true"));

  EXPECT_EQ(applied, (std::vector<std::string>{"false", "true"}));

  ASSERT_TRUE(service->setGlobalValue("watched", "false"));

  EXPECT_EQ(applied, (std::vector<std::string>{"false", "true", "false"}));
}

TEST_F(SettingWatcherTest, AChangeToAnotherKeyIsNotApplied) {
  const auto watcher = watch("watched");

  ASSERT_TRUE(service->setGlobalValue("other", "true"));

  EXPECT_EQ(applied, (std::vector<std::string>{"false"}));
}

TEST_F(SettingWatcherTest, AChangeUnderASessionOverrideIsNotApplied) {
  service->setSessionOverride("watched", "true");
  const auto watcher = watch("watched");

  ASSERT_TRUE(service->setGlobalValue("watched", "false"));

  EXPECT_EQ(applied, (std::vector<std::string>{"true"}));
}

TEST_F(SettingWatcherTest, ACallbackThatWritesItsOwnKeyDoesNotLoop) {
  std::vector<std::string> seen;
  const SettingWatcher watcher(*service, "watched", [this, &seen](const std::string &value) {
    seen.push_back(value);
    service->setGlobalValue("watched", value);
  });

  ASSERT_TRUE(service->setGlobalValue("watched", "true"));

  EXPECT_EQ(seen, (std::vector<std::string>{"false", "true"}));
}

//****************
// resets
//****************

TEST_F(SettingWatcherTest, AResetAppliesTheCatalogDefault) {
  const auto watcher = watch("watched");
  ASSERT_TRUE(service->setGlobalValue("watched", "true"));

  ASSERT_TRUE(service->resetGlobalValue("watched"));

  EXPECT_EQ(applied, (std::vector<std::string>{"false", "true", "false"}));
}

TEST_F(SettingWatcherTest, AResetUnderASessionOverrideAppliesTheOverride) {
  ASSERT_TRUE(service->setGlobalValue("watched", "false"));
  service->setSessionOverride("watched", "true");
  const auto watcher = watch("watched");

  ASSERT_TRUE(service->resetGlobalValue("watched"));

  EXPECT_EQ(applied, (std::vector<std::string>{"true"}));
}

//****************
// lifetime
//****************

TEST_F(SettingWatcherTest, StopsApplyingOnceDestroyed) {
  {
    const auto watcher = watch("watched");
  }

  ASSERT_TRUE(service->setGlobalValue("watched", "true"));

  EXPECT_EQ(applied, (std::vector<std::string>{"false"}));
}

} // namespace firelight::settings

#include "app/input/input_settings_applier.hpp"

#include "../emulation/fake_input_service.hpp"

#include <firelight/settings/settings_catalog.hpp>
#include <firelight/settings/settings_service.hpp>
#include <firelight/settings/sqlite_settings_repository.hpp>

#include <QCoreApplication>
#include <gtest/gtest.h>
#include <memory>
#include <stdexcept>
#include <vector>

namespace firelight::input {

namespace {
constexpr auto KEY = "prioritize-controller-over-keyboard";

auto CATALOG = R"JSON(
{
  "app": [
    {"key": "prioritize-controller-over-keyboard", "label": "Prioritize", "type": "boolean", "default": "false"}
  ]
}
)JSON";
} // namespace

/**
 * A service on an in-memory repository, the shared catalog loaded with the one input key, and a fake input service
 * that records what it is handed
 */
class InputSettingsApplierTest : public testing::Test {
protected:
  emulation::FakeInputService inputService;
  std::unique_ptr<settings::SqliteSettingsRepository> repository;
  std::unique_ptr<settings::SettingsService> service;

  void SetUp() override {
    ASSERT_TRUE(settings::SettingsCatalog::instance().loadFromJson(CATALOG));
    repository = std::make_unique<settings::SqliteSettingsRepository>(":memory:");
    service = std::make_unique<settings::SettingsService>(*repository);
  }

  void TearDown() override {
    service.reset();
    repository.reset();
    settings::SettingsCatalog::instance().loadFromDirectory(
        (QCoreApplication::applicationDirPath() + "/system/settings").toStdString());
  }
};

//****************
// construction
//****************

TEST_F(InputSettingsApplierTest, PushesTheCatalogDefaultWhenNothingIsStored) {
  const InputSettingsApplier applier(inputService, *service);

  EXPECT_EQ(inputService.preferGamepadOverKeyboardCalls, std::vector<bool>{false});
}

TEST_F(InputSettingsApplierTest, PushesTheStoredValueWhenThereIsOne) {
  ASSERT_TRUE(service->setGlobalValue(KEY, "true"));

  const InputSettingsApplier applier(inputService, *service);

  EXPECT_TRUE(inputService.preferGamepadOverKeyboard());
}

TEST_F(InputSettingsApplierTest, StopsWhenTheCatalogLacksTheKey) {
  ASSERT_TRUE(settings::SettingsCatalog::instance().loadFromJson("{}"));

  EXPECT_THROW({ const InputSettingsApplier applier(inputService, *service); }, std::runtime_error);
  EXPECT_TRUE(inputService.preferGamepadOverKeyboardCalls.empty());
}

//****************
// changes
//****************

TEST_F(InputSettingsApplierTest, FollowsAChange) {
  const InputSettingsApplier applier(inputService, *service);

  ASSERT_TRUE(service->setGlobalValue(KEY, "true"));

  EXPECT_TRUE(inputService.preferGamepadOverKeyboard());
  EXPECT_EQ(inputService.preferGamepadOverKeyboardCalls, (std::vector<bool>{false, true}));
}

TEST_F(InputSettingsApplierTest, FollowsAResetBackToTheDefault) {
  ASSERT_TRUE(service->setGlobalValue(KEY, "true"));
  const InputSettingsApplier applier(inputService, *service);

  ASSERT_TRUE(service->resetGlobalValue(KEY));

  EXPECT_FALSE(inputService.preferGamepadOverKeyboard());
}

TEST_F(InputSettingsApplierTest, StopsFollowingWhenDestroyed) {
  {
    const InputSettingsApplier applier(inputService, *service);
  }

  ASSERT_TRUE(service->setGlobalValue(KEY, "true"));

  EXPECT_EQ(inputService.preferGamepadOverKeyboardCalls, std::vector<bool>{false});
}

} // namespace firelight::input

#include "gui/qt_input_service_proxy.hpp"

#include "../../../libs/firelight/input/tests/test_gamepad.hpp"
#include "../emulation/fake_input_service.hpp"

#include <firelight/event_dispatcher.hpp>
#include <firelight/input/input_service.hpp>
#include <firelight/settings/settings_catalog.hpp>
#include <firelight/settings/settings_service.hpp>
#include <firelight/settings/sqlite_settings_repository.hpp>

#include <QCoreApplication>
#include <QTest>
#include <gtest/gtest.h>
#include <memory>
#include <stdexcept>

namespace firelight::gui {

namespace {
constexpr auto KEY = "only-allow-player-one-navigate-menus";

auto CATALOG = R"JSON(
{
  "app": [
    {"key": "only-allow-player-one-navigate-menus", "label": "Only player 1", "type": "boolean", "default": "false"}
  ]
}
)JSON";
} // namespace

/**
 * A service on an in-memory repository and the shared catalog loaded with the one key the proxy watches
 */
class QtInputServiceProxyTest : public testing::Test {
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

  std::unique_ptr<QtInputServiceProxy> makeProxy() {
    return std::make_unique<QtInputServiceProxy>(inputService, *service);
  }
};

//****************
// only player one navigates menus
//****************

TEST_F(QtInputServiceProxyTest, ReadsTheCatalogDefaultWhenNothingIsStored) {
  const auto proxy = makeProxy();

  EXPECT_FALSE(proxy->getOnlyPlayerOneCanNavigateMenus());
}

TEST_F(QtInputServiceProxyTest, ReadsTheStoredValueAtConstruction) {
  ASSERT_TRUE(service->setGlobalValue(KEY, "true"));

  const auto proxy = makeProxy();

  EXPECT_TRUE(proxy->getOnlyPlayerOneCanNavigateMenus());
}

TEST_F(QtInputServiceProxyTest, FollowsAChangeAndNotifies) {
  const auto proxy = makeProxy();
  int notified = 0;
  QObject::connect(proxy.get(), &QtInputServiceProxy::onlyPlayerOneCanNavigateMenusChanged, [&] { ++notified; });

  ASSERT_TRUE(service->setGlobalValue(KEY, "true"));

  EXPECT_TRUE(proxy->getOnlyPlayerOneCanNavigateMenus());
  EXPECT_EQ(notified, 1);
}

TEST_F(QtInputServiceProxyTest, FollowsAResetBackToTheDefault) {
  ASSERT_TRUE(service->setGlobalValue(KEY, "true"));
  const auto proxy = makeProxy();

  ASSERT_TRUE(service->resetGlobalValue(KEY));

  EXPECT_FALSE(proxy->getOnlyPlayerOneCanNavigateMenus());
}

TEST_F(QtInputServiceProxyTest, StopsWhenTheCatalogLacksTheKey) {
  ASSERT_TRUE(settings::SettingsCatalog::instance().loadFromJson("{}"));

  EXPECT_THROW({ const auto proxy = makeProxy(); }, std::runtime_error);
}

TEST_F(QtInputServiceProxyTest, EveryPlayerNavigatesWhenTheSettingIsOff) {
  const auto proxy = makeProxy();

  EXPECT_TRUE(proxy->isAllowedToNavigateMenus(0));
  EXPECT_TRUE(proxy->isAllowedToNavigateMenus(1));
  EXPECT_TRUE(proxy->isAllowedToNavigateMenus(3));
  EXPECT_TRUE(proxy->isAllowedToNavigateMenus(-1));
}

TEST_F(QtInputServiceProxyTest, OnlyPlayerOneNavigatesWhenTheSettingIsOn) {
  ASSERT_TRUE(service->setGlobalValue(KEY, "true"));
  const auto proxy = makeProxy();

  EXPECT_TRUE(proxy->isAllowedToNavigateMenus(0));
  EXPECT_FALSE(proxy->isAllowedToNavigateMenus(1));
  EXPECT_FALSE(proxy->isAllowedToNavigateMenus(3));
  EXPECT_TRUE(proxy->isAllowedToNavigateMenus(-1));
}

//****************
// auto-repeat
//****************

namespace {
constexpr int PAST_THE_INITIAL_REPEAT_DELAY_MS = 650;

/**
 * Counts the auto-repeat presses the proxy republishes
 */
class RepeatCounter {
public:
  RepeatCounter()
      : m_connection(EventDispatcher::instance().subscribe<input::GamepadInputEvent>(
            [this](const input::GamepadInputEvent &event) { m_count += event.autoRepeat ? 1 : 0; })) {}

  [[nodiscard]] int getCount() const { return m_count; }

private:
  int m_count = 0;
  ScopedConnection m_connection;
};
} // namespace

TEST_F(QtInputServiceProxyTest, AHeldButtonRepeatsAfterTheInitialDelay) {
  const auto proxy = makeProxy();
  input::TestGamepad pad(7);
  RepeatCounter repeats;

  EventDispatcher::instance().publish(
      input::GamepadInputEvent{.gamepad = &pad, .playerIndex = 0, .input = input::DpadRight, .pressed = true});
  QTest::qWait(PAST_THE_INITIAL_REPEAT_DELAY_MS);

  EXPECT_GT(repeats.getCount(), 0);

  EventDispatcher::instance().publish(
      input::GamepadInputEvent{.gamepad = &pad, .playerIndex = 0, .input = input::DpadRight, .pressed = false});
}

TEST_F(QtInputServiceProxyTest, AReleaseFromAnotherSlotStillStopsTheRepeat) {
  const auto proxy = makeProxy();
  input::TestGamepad pad(7);
  RepeatCounter repeats;

  EventDispatcher::instance().publish(
      input::GamepadInputEvent{.gamepad = &pad, .playerIndex = 0, .input = input::DpadRight, .pressed = true});
  EventDispatcher::instance().publish(
      input::GamepadInputEvent{.gamepad = &pad, .playerIndex = 1, .input = input::DpadRight, .pressed = false});
  QTest::qWait(PAST_THE_INITIAL_REPEAT_DELAY_MS);

  EXPECT_EQ(repeats.getCount(), 0);
}

} // namespace firelight::gui

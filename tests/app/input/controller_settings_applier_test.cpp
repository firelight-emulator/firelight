#include "app/input/controller_settings_applier.hpp"

#include "../../../libs/firelight/input/tests/test_gamepad.hpp"
#include "../emulation/fake_input_service.hpp"
#include "app/emulation/emulation_service.hpp"

#include <firelight/event_dispatcher.hpp>
#include <firelight/input/gamepad_profile.hpp>
#include <firelight/input/keyboard_input_handler.hpp>
#include <firelight/settings/settings_catalog.hpp>
#include <firelight/settings/settings_service.hpp>
#include <firelight/settings/sqlite_settings_repository.hpp>

#include <QCoreApplication>
#include <cstdint>
#include <gtest/gtest.h>
#include <memory>
#include <optional>
#include <stdexcept>
#include <vector>

namespace firelight::input {

namespace {
constexpr auto RUMBLE_KEY = "rumble-strength";
constexpr auto LIGHT_KEY = "light-bar-color";
constexpr auto GAME_HASH = "game-hash";
constexpr auto OTHER_GAME_HASH = "other-game-hash";

using LightColors = std::vector<std::optional<uint32_t>>;

auto CATALOG = R"JSON(
{
  "controller": [
    {"key": "rumble-strength", "label": "Rumble", "type": "slider", "min": 0, "max": 100, "step": 5, "default": "100"},
    {"key": "light-bar-color", "label": "Light", "type": "color", "default": ""}
  ]
}
)JSON";
} // namespace

/**
 * A service on an in-memory repository, the shared catalog loaded with the two controller keys, and a fake input
 * service that lists the pads added to it and records what it is handed
 */
class ControllerSettingsApplierTest : public testing::Test {
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

  /** Adds a pad using a profile to the devices the input service lists */
  std::shared_ptr<TestGamepad> addPad(const int instanceId, const int profileId) {
    auto pad = std::make_shared<TestGamepad>(instanceId);
    pad->setProfile(std::make_shared<GamepadProfile>(profileId));
    inputService.devices.push_back(pad);
    return pad;
  }

  /** Every percent pushed to a profile, in order */
  [[nodiscard]] std::vector<int> rumbleFor(const int profileId) const {
    std::vector<int> percents;
    for (const auto &call : inputService.rumbleScaleCalls) {
      if (call.profileId == profileId) {
        percents.push_back(call.percent);
      }
    }

    return percents;
  }

  /** Every colour pushed to a profile, in order */
  [[nodiscard]] LightColors lightFor(const int profileId) const {
    LightColors colors;
    for (const auto &call : inputService.lightColorCalls) {
      if (call.profileId == profileId) {
        colors.push_back(call.rgb);
      }
    }

    return colors;
  }
};

//****************
// construction
//****************

TEST_F(ControllerSettingsApplierTest, PushesTheDefaultOncePerConnectedProfile) {
  addPad(1, 7);
  addPad(2, 7);
  addPad(3, 8);

  const ControllerSettingsApplier applier(inputService, *service);

  EXPECT_EQ(rumbleFor(7), std::vector<int>{100});
  EXPECT_EQ(rumbleFor(8), std::vector<int>{100});
  EXPECT_EQ(lightFor(7), LightColors{std::nullopt});
  EXPECT_EQ(lightFor(8), LightColors{std::nullopt});
  EXPECT_EQ(inputService.rumbleScaleCalls.size(), 2U);
  EXPECT_EQ(inputService.lightColorCalls.size(), 2U);
}

TEST_F(ControllerSettingsApplierTest, PushesTheStoredProfileValue) {
  ASSERT_TRUE(service->setControllerValue({}, 7, RUMBLE_KEY, "40"));
  ASSERT_TRUE(service->setControllerValue({}, 7, LIGHT_KEY, "#FF8000"));
  addPad(1, 7);

  const ControllerSettingsApplier applier(inputService, *service);

  EXPECT_EQ(rumbleFor(7), std::vector<int>{40});
  EXPECT_EQ(lightFor(7), LightColors{0xFF8000});
}

TEST_F(ControllerSettingsApplierTest, PushesTheSessionOverride) {
  service->setSessionOverride(RUMBLE_KEY, "30");
  ASSERT_TRUE(service->setControllerValue({}, 7, RUMBLE_KEY, "40"));
  addPad(1, 7);

  const ControllerSettingsApplier applier(inputService, *service);

  EXPECT_EQ(rumbleFor(7), std::vector<int>{30});
}

TEST_F(ControllerSettingsApplierTest, PushesNothingWithNoControllerConnected) {
  const ControllerSettingsApplier applier(inputService, *service);

  EXPECT_TRUE(inputService.rumbleScaleCalls.empty());
  EXPECT_TRUE(inputService.lightColorCalls.empty());
}

TEST_F(ControllerSettingsApplierTest, IgnoresTheKeyboard) {
  auto keyboard = std::make_shared<KeyboardInputHandler>();
  keyboard->setProfile(std::make_shared<GamepadProfile>(9));
  inputService.devices.push_back(keyboard);
  addPad(1, 7);

  const ControllerSettingsApplier applier(inputService, *service);
  EventDispatcher::instance().publish(GamepadConnectedEvent{keyboard});

  EXPECT_TRUE(rumbleFor(9).empty());
  EXPECT_TRUE(lightFor(9).empty());
  EXPECT_EQ(rumbleFor(7), std::vector<int>{100});
  EXPECT_EQ(inputService.rumbleScaleCalls.size(), 1U);
  EXPECT_EQ(inputService.lightColorCalls.size(), 1U);
}

TEST_F(ControllerSettingsApplierTest, IgnoresAControllerWithNoProfile) {
  inputService.devices.push_back(std::make_shared<TestGamepad>(1));

  const ControllerSettingsApplier applier(inputService, *service);

  EXPECT_TRUE(inputService.rumbleScaleCalls.empty());
  EXPECT_TRUE(inputService.lightColorCalls.empty());
}

TEST_F(ControllerSettingsApplierTest, StopsWhenTheCatalogLacksAKey) {
  addPad(1, 7);

  ASSERT_TRUE(settings::SettingsCatalog::instance().loadFromJson(R"JSON(
{"controller": [{"key": "rumble-strength", "label": "Rumble", "type": "slider", "default": "100"}]}
)JSON"));
  EXPECT_THROW({ const ControllerSettingsApplier applier(inputService, *service); }, std::runtime_error);

  ASSERT_TRUE(settings::SettingsCatalog::instance().loadFromJson(R"JSON(
{"controller": [{"key": "light-bar-color", "label": "Light", "type": "color", "default": ""}]}
)JSON"));
  EXPECT_THROW({ const ControllerSettingsApplier applier(inputService, *service); }, std::runtime_error);

  EXPECT_TRUE(inputService.rumbleScaleCalls.empty());
  EXPECT_TRUE(inputService.lightColorCalls.empty());
}

TEST_F(ControllerSettingsApplierTest, StopsWhenAKeyIsNotAControllerSetting) {
  ASSERT_TRUE(settings::SettingsCatalog::instance().loadFromJson(R"JSON(
{
  "app": [{"key": "rumble-strength", "label": "Rumble", "type": "slider", "default": "100"}],
  "controller": [{"key": "light-bar-color", "label": "Light", "type": "color", "default": ""}]
}
)JSON"));

  EXPECT_THROW({ const ControllerSettingsApplier applier(inputService, *service); }, std::runtime_error);
}

//****************
// parsing
//****************

TEST_F(ControllerSettingsApplierTest, ClampsTheRumbleStrengthAndReadsAnythingElseAsFull) {
  ASSERT_TRUE(service->setControllerValue({}, 1, RUMBLE_KEY, "150"));
  ASSERT_TRUE(service->setControllerValue({}, 2, RUMBLE_KEY, "-20"));
  ASSERT_TRUE(service->setControllerValue({}, 3, RUMBLE_KEY, "loud"));
  ASSERT_TRUE(service->setControllerValue({}, 4, RUMBLE_KEY, ""));
  for (int profileId = 1; profileId <= 4; ++profileId) {
    addPad(profileId, profileId);
  }

  const ControllerSettingsApplier applier(inputService, *service);

  EXPECT_EQ(rumbleFor(1), std::vector<int>{100});
  EXPECT_EQ(rumbleFor(2), std::vector<int>{0});
  EXPECT_EQ(rumbleFor(3), std::vector<int>{100});
  EXPECT_EQ(rumbleFor(4), std::vector<int>{100});
}

TEST_F(ControllerSettingsApplierTest, ReadsOnlyAHashAndSixHexDigitsAsAColour) {
  ASSERT_TRUE(service->setControllerValue({}, 1, LIGHT_KEY, "#00ff7f"));
  ASSERT_TRUE(service->setControllerValue({}, 2, LIGHT_KEY, "red"));
  ASSERT_TRUE(service->setControllerValue({}, 3, LIGHT_KEY, "#12345"));
  ASSERT_TRUE(service->setControllerValue({}, 4, LIGHT_KEY, "#12345g"));
  ASSERT_TRUE(service->setControllerValue({}, 5, LIGHT_KEY, "00ff7f0"));
  ASSERT_TRUE(service->setControllerValue({}, 6, LIGHT_KEY, "#-12345"));
  for (int profileId = 1; profileId <= 6; ++profileId) {
    addPad(profileId, profileId);
  }

  const ControllerSettingsApplier applier(inputService, *service);

  EXPECT_EQ(lightFor(1), LightColors{0x00FF7F});
  for (int profileId = 2; profileId <= 6; ++profileId) {
    EXPECT_EQ(lightFor(profileId), LightColors{std::nullopt}) << "profile " << profileId;
  }
}

//****************
// changes
//****************

TEST_F(ControllerSettingsApplierTest, FollowsAProfileChangeAndPushesOnlyWhatChanged) {
  addPad(1, 7);
  const ControllerSettingsApplier applier(inputService, *service);

  ASSERT_TRUE(service->setControllerValue({}, 7, RUMBLE_KEY, "60"));

  EXPECT_EQ(rumbleFor(7), (std::vector<int>{100, 60}));
  EXPECT_EQ(lightFor(7), LightColors{std::nullopt});

  ASSERT_TRUE(service->setControllerValue({}, 7, LIGHT_KEY, "#0000ff"));

  EXPECT_EQ(rumbleFor(7), (std::vector<int>{100, 60}));
  EXPECT_EQ(lightFor(7), (LightColors{std::nullopt, 0x0000FF}));
}

TEST_F(ControllerSettingsApplierTest, PushesNothingWhenTheValueIsUnchanged) {
  addPad(1, 7);
  const ControllerSettingsApplier applier(inputService, *service);

  ASSERT_TRUE(service->setControllerValue({}, 7, RUMBLE_KEY, "100"));
  ASSERT_TRUE(service->setControllerValue(GAME_HASH, 7, RUMBLE_KEY, "10"));
  ASSERT_TRUE(service->setControllerValue({}, 8, RUMBLE_KEY, "20"));
  ASSERT_TRUE(service->resetControllerValue({}, 7, LIGHT_KEY));

  EXPECT_EQ(inputService.rumbleScaleCalls.size(), 1U);
  EXPECT_EQ(inputService.lightColorCalls.size(), 1U);
}

TEST_F(ControllerSettingsApplierTest, FollowsAProfileResetBackToTheDefault) {
  ASSERT_TRUE(service->setControllerValue({}, 7, RUMBLE_KEY, "40"));
  ASSERT_TRUE(service->setControllerValue({}, 7, LIGHT_KEY, "#0000ff"));
  addPad(1, 7);
  const ControllerSettingsApplier applier(inputService, *service);

  ASSERT_TRUE(service->resetControllerValue({}, 7, RUMBLE_KEY));
  ASSERT_TRUE(service->resetControllerValue({}, 7, LIGHT_KEY));

  EXPECT_EQ(rumbleFor(7), (std::vector<int>{40, 100}));
  EXPECT_EQ(lightFor(7), (LightColors{0x0000FF, std::nullopt}));
}

TEST_F(ControllerSettingsApplierTest, PushesTheGameValueOnceTheGameStarts) {
  ASSERT_TRUE(service->setControllerValue({}, 7, RUMBLE_KEY, "40"));
  ASSERT_TRUE(service->setControllerValue(GAME_HASH, 7, RUMBLE_KEY, "70"));
  addPad(1, 7);
  const ControllerSettingsApplier applier(inputService, *service);

  EventDispatcher::instance().publish(emulation::EmulationStartedEvent{.contentHash = GAME_HASH, .saveSlotNumber = 1});

  EXPECT_EQ(rumbleFor(7), (std::vector<int>{40, 70}));
}

TEST_F(ControllerSettingsApplierTest, FallsBackToTheProfileValueOnceTheGameStops) {
  ASSERT_TRUE(service->setControllerValue({}, 7, RUMBLE_KEY, "40"));
  ASSERT_TRUE(service->setControllerValue(GAME_HASH, 7, RUMBLE_KEY, "70"));
  addPad(1, 7);
  const ControllerSettingsApplier applier(inputService, *service);
  EventDispatcher::instance().publish(emulation::EmulationStartedEvent{.contentHash = GAME_HASH, .saveSlotNumber = 1});

  EventDispatcher::instance().publish(emulation::EmulationStoppedEvent{});

  EXPECT_EQ(rumbleFor(7), (std::vector<int>{40, 70, 40}));
}

TEST_F(ControllerSettingsApplierTest, FollowsAGameChangeAndResetWhileTheGameRuns) {
  ASSERT_TRUE(service->setControllerValue({}, 7, RUMBLE_KEY, "40"));
  addPad(1, 7);
  const ControllerSettingsApplier applier(inputService, *service);
  EventDispatcher::instance().publish(emulation::EmulationStartedEvent{.contentHash = GAME_HASH, .saveSlotNumber = 1});

  ASSERT_TRUE(service->setControllerValue(OTHER_GAME_HASH, 7, RUMBLE_KEY, "10"));
  ASSERT_TRUE(service->setControllerValue(GAME_HASH, 7, RUMBLE_KEY, "85"));
  ASSERT_TRUE(service->resetControllerValue(GAME_HASH, 7, RUMBLE_KEY));

  EXPECT_EQ(rumbleFor(7), (std::vector<int>{40, 40, 85, 40}));
}

TEST_F(ControllerSettingsApplierTest, FollowsTheProfileAGameSwapsIn) {
  ASSERT_TRUE(service->setControllerValue({}, 7, RUMBLE_KEY, "40"));
  ASSERT_TRUE(service->setControllerValue({}, 8, RUMBLE_KEY, "40"));
  const auto pad = addPad(1, 7);
  const ControllerSettingsApplier applier(inputService, *service);

  for (int session = 0; session < 2; ++session) {
    pad->setProfile(std::make_shared<GamepadProfile>(8));
    EventDispatcher::instance().publish(
        emulation::EmulationStartedEvent{.contentHash = GAME_HASH, .saveSlotNumber = 1});
    pad->setProfile(std::make_shared<GamepadProfile>(7));
    EventDispatcher::instance().publish(emulation::EmulationStoppedEvent{});
  }

  EXPECT_EQ(rumbleFor(8), (std::vector<int>{40, 40}));
  EXPECT_EQ(rumbleFor(7), (std::vector<int>{40, 40, 40}));
}

TEST_F(ControllerSettingsApplierTest, PushesToANewlyConnectedController) {
  ASSERT_TRUE(service->setControllerValue({}, 7, RUMBLE_KEY, "40"));
  ASSERT_TRUE(service->setControllerValue({}, 8, LIGHT_KEY, "#ff0000"));
  addPad(1, 7);
  const ControllerSettingsApplier applier(inputService, *service);

  const auto samePad = addPad(2, 7);
  EventDispatcher::instance().publish(GamepadConnectedEvent{samePad});
  const auto newPad = addPad(3, 8);
  EventDispatcher::instance().publish(GamepadConnectedEvent{newPad});

  EXPECT_EQ(rumbleFor(7), (std::vector<int>{40, 40}));
  EXPECT_EQ(lightFor(7), (LightColors{std::nullopt, std::nullopt}));
  EXPECT_EQ(rumbleFor(8), std::vector<int>{100});
  EXPECT_EQ(lightFor(8), LightColors{0xFF0000});
}

//****************
// destruction
//****************

TEST_F(ControllerSettingsApplierTest, StopsFollowingWhenDestroyed) {
  addPad(1, 7);
  {
    const ControllerSettingsApplier applier(inputService, *service);
  }

  ASSERT_TRUE(service->setControllerValue({}, 7, RUMBLE_KEY, "40"));
  EventDispatcher::instance().publish(emulation::EmulationStartedEvent{.contentHash = GAME_HASH, .saveSlotNumber = 1});
  EventDispatcher::instance().publish(GamepadConnectedEvent{addPad(2, 7)});
  EventDispatcher::instance().publish(emulation::EmulationStoppedEvent{});

  EXPECT_EQ(rumbleFor(7), std::vector<int>{100});
  EXPECT_EQ(lightFor(7), LightColors{std::nullopt});
}

} // namespace firelight::input

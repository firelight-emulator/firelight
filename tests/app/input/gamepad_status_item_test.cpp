// TODO: NEEDS REVIEW
#include "app/input/gui/gamepad_status_item.hpp"

#include "../../../libs/firelight/input/tests/test_gamepad.hpp"
#include "../emulation/fake_input_service.hpp"
#include "service_accessor.hpp"

#include <firelight/event_dispatcher.hpp>
#include <firelight/input/gamepad_profile.hpp>
#include <firelight/input/input_service.hpp>

#include <QCoreApplication>
#include <QSignalSpy>
#include <gtest/gtest.h>
#include <memory>

namespace firelight::input {

// TODO
/** One test pad in slot 0 behind the locator the item reads from */
class GamepadStatusItemTest : public testing::Test {
protected:
  static constexpr int PAD_PROFILE = 100;
  static constexpr int OTHER_PROFILE = 101;

  emulation::FakeInputService inputService;
  std::shared_ptr<TestGamepad> pad;

  void SetUp() override {
    pad = addPad(0, PAD_PROFILE);
    ServiceAccessor::setInputService(&inputService);
  }

  void TearDown() override { ServiceAccessor::setInputService(nullptr); }

  std::shared_ptr<TestGamepad> addPad(const int slot, const int profileId) {
    auto device = std::make_shared<TestGamepad>(slot + 1);
    device->setProfile(std::make_shared<GamepadProfile>(profileId));
    device->setPlayerIndex(slot);
    inputService.playerSlots[slot] = device;
    return device;
  }
};

TEST_F(GamepadStatusItemTest, ResolvesTheSlotsDevice) {
  GamepadStatusItem item;

  item.setPlayerIndex(0);
  EXPECT_TRUE(item.isConnected());
  EXPECT_EQ(item.getName(), "TestGamepad");
  EXPECT_EQ(item.getProfileId(), PAD_PROFILE);

  item.setPlayerIndex(3);
  EXPECT_FALSE(item.isConnected());
  EXPECT_TRUE(item.getName().isEmpty());
  EXPECT_EQ(item.getProfileId(), 0);
}

TEST_F(GamepadStatusItemTest, PollsThePhysicalValues) {
  GamepadStatusItem item;
  item.setPlayerIndex(0);
  QSignalSpy values(&item, &GamepadStatusItem::valuesChanged);

  pad->setPhysical(SouthFace, 32767);
  pad->setPhysical(LeftStickRight, 16384);
  pad->setPhysical(LeftStickUp, 32767);
  pad->setPhysical(RightStickLeft, 32767);
  pad->setPhysical(LeftTrigger, 32767);
  ASSERT_TRUE(values.wait(500));

  const auto pressed = item.getPressed();
  EXPECT_TRUE(pressed["SouthFace"].toBool());
  EXPECT_FALSE(pressed["EastFace"].toBool());
  EXPECT_EQ(pressed.size(), 15);
  EXPECT_NEAR(item.getLeftStickX(), 0.5, 0.001);
  EXPECT_DOUBLE_EQ(item.getLeftStickY(), -1.0);
  EXPECT_DOUBLE_EQ(item.getRightStickX(), -1.0);
  EXPECT_DOUBLE_EQ(item.getRightStickY(), 0.0);
  EXPECT_DOUBLE_EQ(item.getLeftTrigger(), 1.0);
  EXPECT_DOUBLE_EQ(item.getRightTrigger(), 0.0);
  EXPECT_TRUE(item.isButtonPressed(SouthFace));
}

TEST_F(GamepadStatusItemTest, NothingMovedNothingReported) {
  GamepadStatusItem item;
  item.setPlayerIndex(0);
  QSignalSpy values(&item, &GamepadStatusItem::valuesChanged);

  EXPECT_FALSE(values.wait(60));
}

TEST_F(GamepadStatusItemTest, LiveOffStopsPolling) {
  GamepadStatusItem item;
  item.setPlayerIndex(0);
  QSignalSpy values(&item, &GamepadStatusItem::valuesChanged);

  item.setLive(false);
  pad->setPhysical(EastFace, 32767);
  EXPECT_FALSE(values.wait(60));

  item.setLive(true);
  EXPECT_TRUE(values.wait(500));
  EXPECT_TRUE(item.getPressed()["EastFace"].toBool());
}

TEST_F(GamepadStatusItemTest, AnOrderChangeFollowsTheSlot) {
  addPad(1, OTHER_PROFILE);
  GamepadStatusItem item;
  item.setPlayerIndex(0);

  std::swap(inputService.playerSlots[0], inputService.playerSlots[1]);
  EventDispatcher::instance().publish(GamepadOrderChangedEvent{});
  QCoreApplication::processEvents();

  EXPECT_EQ(item.getProfileId(), OTHER_PROFILE);
}

TEST_F(GamepadStatusItemTest, DisconnectingClearsTheValues) {
  GamepadStatusItem item;
  pad->setPhysical(SouthFace, 32767);
  pad->setPhysical(LeftStickRight, 32767);
  item.setPlayerIndex(0);
  ASSERT_TRUE(item.getPressed()["SouthFace"].toBool());
  QSignalSpy values(&item, &GamepadStatusItem::valuesChanged);

  inputService.playerSlots.erase(0);
  EventDispatcher::instance().publish(GamepadDisconnectedEvent{0});
  QCoreApplication::processEvents();

  EXPECT_FALSE(item.isConnected());
  EXPECT_FALSE(item.getPressed()["SouthFace"].toBool());
  EXPECT_DOUBLE_EQ(item.getLeftStickX(), 0.0);
  EXPECT_EQ(values.count(), 1);
}

TEST_F(GamepadStatusItemTest, InputEventsForTheSlotAreForwarded) {
  GamepadStatusItem item;
  item.setPlayerIndex(0);
  QSignalSpy inputs(&item, &GamepadStatusItem::inputChanged);

  EventDispatcher::instance().publish(GamepadInputEvent{pad.get(), 0, SouthFace, true});
  EventDispatcher::instance().publish(GamepadInputEvent{pad.get(), 1, SouthFace, true});
  QCoreApplication::processEvents();

  ASSERT_EQ(inputs.count(), 1);
  EXPECT_EQ(inputs.first().at(0).toInt(), static_cast<int>(SouthFace));
  EXPECT_TRUE(inputs.first().at(1).toBool());
}

} // namespace firelight::input

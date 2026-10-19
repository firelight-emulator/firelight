// TODO: NEEDS REVIEW
#include "gui/gamepad_profile_item.hpp"

#include "../input/fake_controller_repository.hpp"
#include "app/input/profile_events.hpp"
#include "service_accessor.hpp"

#include <firelight/event_dispatcher.hpp>

#include <QCoreApplication>
#include <QSignalSpy>
#include <gtest/gtest.h>

namespace firelight::gui {

// TODO
/** One pad profile behind the locator the item reads from */
class GamepadProfileItemTest : public testing::Test {
protected:
  static constexpr int PAD_PROFILE = 7;

  input::FakeControllerRepository repository;
  std::vector<int> announcedProfileIds;
  ScopedConnection announced;

  void SetUp() override {
    repository.addProfile(PAD_PROFILE, "Pad", false, "xbox");
    ServiceAccessor::setControllerProfileRepository(&repository);
    announced = EventDispatcher::instance().subscribe<input::ProfileSettingsChangedEvent>(
        [this](const input::ProfileSettingsChangedEvent &event) { announcedProfileIds.push_back(event.profileId); });
  }

  void TearDown() override { ServiceAccessor::setControllerProfileRepository(nullptr); }
};

TEST_F(GamepadProfileItemTest, SettingTheProfileLoadsItsValues) {
  GamepadProfileItem item;
  item.setId(PAD_PROFILE);

  EXPECT_EQ(item.id(), PAD_PROFILE);
  EXPECT_EQ(item.name(), "Pad");
  EXPECT_EQ(item.icon(), "xbox");
  EXPECT_FALSE(item.isKeyboardProfile());
  EXPECT_DOUBLE_EQ(item.leftStickInnerDeadzone(), 0.25);
  EXPECT_DOUBLE_EQ(item.rightTriggerThreshold(), 0.5);
  EXPECT_EQ(item.leftStickCurve(), 0);
}

TEST_F(GamepadProfileItemTest, RenamingPersistsAndAnnounces) {
  GamepadProfileItem item;
  item.setId(PAD_PROFILE);
  QSignalSpy renamed(&item, &GamepadProfileItem::nameChanged);

  item.setName("Gamepad");

  ASSERT_EQ(repository.renameCalls.size(), 1u);
  EXPECT_EQ(repository.getProfile(PAD_PROFILE)->getName(), "Gamepad");
  EXPECT_EQ(item.name(), "Gamepad");
  EXPECT_EQ(renamed.count(), 1);
  EXPECT_EQ(announcedProfileIds, std::vector<int>{PAD_PROFILE});
}

TEST_F(GamepadProfileItemTest, ARefusedRenameLeavesTheNameAlone) {
  repository.addProfile(8, "Taken");
  GamepadProfileItem item;
  item.setId(PAD_PROFILE);
  QSignalSpy renamed(&item, &GamepadProfileItem::nameChanged);

  item.setName("Taken");

  EXPECT_EQ(item.name(), "Pad");
  EXPECT_EQ(renamed.count(), 0);
  EXPECT_TRUE(announcedProfileIds.empty());
}

TEST_F(GamepadProfileItemTest, WritingAStickValuePersistsIt) {
  GamepadProfileItem item;
  item.setId(PAD_PROFILE);
  QSignalSpy changed(&item, &GamepadProfileItem::analogChanged);

  item.setLeftStickSensitivity(1.5);
  item.setLeftStickSensitivity(1.5);

  ASSERT_EQ(repository.analogCalls.size(), 1u);
  EXPECT_EQ(repository.analogCalls.front().profileId, PAD_PROFILE);
  EXPECT_FLOAT_EQ(repository.analogCalls.front().settings.leftStick.sensitivity, 1.5f);
  EXPECT_DOUBLE_EQ(item.leftStickSensitivity(), 1.5);
  EXPECT_EQ(changed.count(), 1);
  EXPECT_EQ(announcedProfileIds, std::vector<int>{PAD_PROFILE});
}

TEST_F(GamepadProfileItemTest, CurvesAreIntsAndRejectUnknownValues) {
  GamepadProfileItem item;
  item.setId(PAD_PROFILE);

  item.setRightStickCurve(1);
  item.setRightStickCurve(5);

  ASSERT_EQ(repository.analogCalls.size(), 1u);
  EXPECT_EQ(repository.analogCalls.front().settings.rightStick.curve, input::ResponseCurve::Exponential);
  EXPECT_EQ(item.rightStickCurve(), 1);
}

TEST_F(GamepadProfileItemTest, ResetRestoresTheDefaults) {
  GamepadProfileItem item;
  item.setId(PAD_PROFILE);
  item.setLeftStickInnerDeadzone(0.1);

  item.resetAnalogToDefault();
  item.resetAnalogToDefault();

  ASSERT_EQ(repository.analogCalls.size(), 2u);
  EXPECT_TRUE(repository.analogCalls.back().settings == input::AnalogSettings{});
  EXPECT_DOUBLE_EQ(item.leftStickInnerDeadzone(), 0.25);
}

TEST_F(GamepadProfileItemTest, AChangeElsewhereReloadsTheValues) {
  GamepadProfileItem item;
  item.setId(PAD_PROFILE);
  QSignalSpy renamed(&item, &GamepadProfileItem::nameChanged);
  QSignalSpy changed(&item, &GamepadProfileItem::analogChanged);

  auto settings = input::AnalogSettings{};
  settings.rightStick.antiDeadzone = 0.2f;
  const auto profile = repository.getProfile(PAD_PROFILE);
  profile->setName("Renamed");
  profile->setDefaultAnalogSettings(settings);
  EventDispatcher::instance().publish(input::ProfileSettingsChangedEvent{PAD_PROFILE});
  EventDispatcher::instance().publish(input::ProfileSettingsChangedEvent{PAD_PROFILE + 1});
  QCoreApplication::processEvents();

  EXPECT_EQ(item.name(), "Renamed");
  EXPECT_FLOAT_EQ(static_cast<float>(item.rightStickAntiDeadzone()), 0.2f);
  EXPECT_EQ(renamed.count(), 1);
  EXPECT_EQ(changed.count(), 1);
}

TEST_F(GamepadProfileItemTest, ItsOwnWritesDoNotEchoBack) {
  GamepadProfileItem item;
  item.setId(PAD_PROFILE);
  QSignalSpy changed(&item, &GamepadProfileItem::analogChanged);

  item.setLeftStickOuterDeadzone(0.05);
  QCoreApplication::processEvents();

  EXPECT_EQ(changed.count(), 1);
}

TEST_F(GamepadProfileItemTest, ADeletedProfileClearsTheItem) {
  GamepadProfileItem item;
  item.setId(PAD_PROFILE);
  QSignalSpy renamed(&item, &GamepadProfileItem::nameChanged);

  repository.deleteProfile(PAD_PROFILE);
  EventDispatcher::instance().publish(input::ProfileSettingsChangedEvent{PAD_PROFILE});
  QCoreApplication::processEvents();
  item.setName("Anything");

  EXPECT_TRUE(item.name().isEmpty());
  EXPECT_TRUE(item.icon().isEmpty());
  EXPECT_EQ(renamed.count(), 1);
  EXPECT_TRUE(repository.renameCalls.empty());
}

TEST_F(GamepadProfileItemTest, AnUnknownProfileIsInert) {
  GamepadProfileItem item;
  item.setId(999);

  item.setName("Anything");
  item.setLeftStickSensitivity(2.0);
  item.resetAnalogToDefault();

  EXPECT_TRUE(item.name().isEmpty());
  EXPECT_TRUE(repository.renameCalls.empty());
  EXPECT_TRUE(repository.analogCalls.empty());
}

} // namespace firelight::gui

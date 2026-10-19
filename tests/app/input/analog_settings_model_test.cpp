// TODO: NEEDS REVIEW
#include "app/input/gui/analog_settings_model.hpp"

#include "app/input/profile_events.hpp"
#include "fake_controller_repository.hpp"
#include "service_accessor.hpp"

#include <firelight/event_dispatcher.hpp>

#include <QCoreApplication>
#include <QSignalSpy>
#include <gtest/gtest.h>

namespace firelight::input {

// TODO
/** One pad profile behind the locator the model reads from */
class AnalogSettingsModelTest : public testing::Test {
protected:
  static constexpr int PAD_PROFILE = 7;

  FakeControllerRepository repository;
  std::vector<int> announcedProfileIds;
  ScopedConnection announced;

  void SetUp() override {
    repository.addProfile(PAD_PROFILE, "Pad");
    ServiceAccessor::setControllerProfileRepository(&repository);
    announced = EventDispatcher::instance().subscribe<ProfileSettingsChangedEvent>(
        [this](const ProfileSettingsChangedEvent &event) { announcedProfileIds.push_back(event.profileId); });
  }

  void TearDown() override { ServiceAccessor::setControllerProfileRepository(nullptr); }
};

TEST_F(AnalogSettingsModelTest, WritingPersistsAndAnnouncesWithoutEchoing) {
  AnalogSettingsModel model;
  model.setProfileId(PAD_PROFILE);
  QSignalSpy changed(&model, &AnalogSettingsModel::changed);

  model.setLeftStickSensitivity(1.5);
  QCoreApplication::processEvents();

  ASSERT_EQ(repository.analogCalls.size(), 1u);
  EXPECT_FLOAT_EQ(repository.analogCalls.front().settings.leftStick.sensitivity, 1.5f);
  EXPECT_EQ(announcedProfileIds, std::vector<int>{PAD_PROFILE});
  EXPECT_EQ(changed.count(), 1);
}

TEST_F(AnalogSettingsModelTest, AChangeElsewhereReloadsTheValues) {
  AnalogSettingsModel model;
  model.setProfileId(PAD_PROFILE);
  QSignalSpy changed(&model, &AnalogSettingsModel::changed);

  auto settings = AnalogSettings{};
  settings.rightStick.antiDeadzone = 0.2f;
  repository.getProfile(PAD_PROFILE)->setDefaultAnalogSettings(settings);
  EventDispatcher::instance().publish(ProfileSettingsChangedEvent{PAD_PROFILE});
  QCoreApplication::processEvents();

  EXPECT_FLOAT_EQ(static_cast<float>(model.rightStickAntiDeadzone()), 0.2f);
  EXPECT_EQ(changed.count(), 1);
}

} // namespace firelight::input

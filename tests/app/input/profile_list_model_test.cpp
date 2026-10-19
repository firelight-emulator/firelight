// TODO: NEEDS REVIEW
#include "app/input/gui/profile_list_model.hpp"

#include "../../../libs/firelight/input/tests/test_gamepad.hpp"
#include "../emulation/fake_input_service.hpp"
#include "app/input/profile_events.hpp"
#include "fake_controller_repository.hpp"
#include "service_accessor.hpp"

#include <firelight/event_dispatcher.hpp>
#include <firelight/input/input_service.hpp>

#include <QAbstractItemModelTester>
#include <QCoreApplication>
#include <QSignalSpy>
#include <gtest/gtest.h>
#include <memory>

namespace firelight::input {

// TODO
/** A pad profile used by slots 0 and 2 and a keyboard profile in slot 1, behind the locator the model reads from */
class ProfileListModelTest : public testing::Test {
protected:
  static constexpr int PAD_PROFILE = 100;
  static constexpr int KEYBOARD_PROFILE = 101;

  FakeControllerRepository repository;
  emulation::FakeInputService inputService;
  std::vector<int> announcedProfileIds;
  ScopedConnection announced;

  void SetUp() override {
    repository.addProfile(PAD_PROFILE, "Pad", false, "xbox");
    repository.addProfile(KEYBOARD_PROFILE, "Keys", true);
    addDevice(0, PAD_PROFILE);
    addDevice(1, KEYBOARD_PROFILE);
    addDevice(2, PAD_PROFILE);

    ServiceAccessor::setControllerProfileRepository(&repository);
    ServiceAccessor::setInputService(&inputService);
    announced = EventDispatcher::instance().subscribe<ProfileSettingsChangedEvent>(
        [this](const ProfileSettingsChangedEvent &event) { announcedProfileIds.push_back(event.profileId); });
  }

  void TearDown() override {
    ServiceAccessor::setControllerProfileRepository(nullptr);
    ServiceAccessor::setInputService(nullptr);
  }

  void addDevice(const int slot, const int profileId) {
    auto pad = std::make_shared<TestGamepad>(slot + 1);
    pad->setProfile(repository.getProfile(profileId));
    pad->setPlayerIndex(slot);
    inputService.devices.push_back(pad);
  }

  static int roleId(const QAbstractItemModel &model, const char *roleName) { return model.roleNames().key(roleName); }

  static QVariant valueAt(const QAbstractItemModel &model, const int row, const char *roleName) {
    return model.data(model.index(row, 0), roleId(model, roleName));
  }

  static int rowOf(const QAbstractItemModel &model, const int profileId) {
    for (int row = 0; row < model.rowCount({}); ++row) {
      if (valueAt(model, row, "profileId").toInt() == profileId) {
        return row;
      }
    }

    return -1;
  }
};

TEST_F(ProfileListModelTest, ListsEveryProfileWithItsSettingsAndSlots) {
  const ProfileListModel model;
  ASSERT_EQ(model.rowCount({}), 2);

  const auto pad = rowOf(model, PAD_PROFILE);
  ASSERT_GE(pad, 0);
  EXPECT_EQ(valueAt(model, pad, "name").toString(), "Pad");
  EXPECT_EQ(valueAt(model, pad, "icon").toString(), "xbox");
  EXPECT_FALSE(valueAt(model, pad, "isKeyboard").toBool());
  EXPECT_EQ(valueAt(model, pad, "playerSlots").value<QList<int>>(), (QList<int>{0, 2}));
  EXPECT_DOUBLE_EQ(valueAt(model, pad, "leftStickInnerDeadzone").toDouble(), 0.25);
  EXPECT_DOUBLE_EQ(valueAt(model, pad, "rightTriggerThreshold").toDouble(), 0.5);
  EXPECT_EQ(valueAt(model, pad, "leftStickCurve").toInt(), 0);

  const auto keyboard = rowOf(model, KEYBOARD_PROFILE);
  ASSERT_GE(keyboard, 0);
  EXPECT_TRUE(valueAt(model, keyboard, "isKeyboard").toBool());
  EXPECT_EQ(valueAt(model, keyboard, "playerSlots").value<QList<int>>(), (QList<int>{1}));
}

TEST_F(ProfileListModelTest, RenamingThroughSetDataPersistsAndAnnounces) {
  ProfileListModel model;
  QSignalSpy changed(&model, &QAbstractItemModel::dataChanged);
  const auto row = rowOf(model, PAD_PROFILE);

  EXPECT_TRUE(model.setData(model.index(row, 0), "Gamepad", roleId(model, "name")));

  ASSERT_EQ(repository.renameCalls.size(), 1u);
  EXPECT_EQ(repository.renameCalls.front().name, "Gamepad");
  EXPECT_EQ(repository.getProfile(PAD_PROFILE)->getName(), "Gamepad");
  EXPECT_EQ(valueAt(model, row, "name").toString(), "Gamepad");
  ASSERT_EQ(changed.count(), 1);
  EXPECT_EQ(changed.first().at(2).value<QList<int>>(), (QList<int>{roleId(model, "name")}));
  EXPECT_EQ(announcedProfileIds, std::vector<int>{PAD_PROFILE});
}

TEST_F(ProfileListModelTest, ARenameTheRepositoryRefusesChangesNothing) {
  ProfileListModel model;
  QSignalSpy changed(&model, &QAbstractItemModel::dataChanged);
  const auto row = rowOf(model, PAD_PROFILE);

  EXPECT_FALSE(model.setData(model.index(row, 0), "Keys", roleId(model, "name")));

  EXPECT_EQ(valueAt(model, row, "name").toString(), "Pad");
  EXPECT_EQ(changed.count(), 0);
  EXPECT_TRUE(announcedProfileIds.empty());
}

TEST_F(ProfileListModelTest, WritingAnAnalogRolePersistsTheWholeSettings) {
  ProfileListModel model;
  QSignalSpy changed(&model, &QAbstractItemModel::dataChanged);
  const auto row = rowOf(model, PAD_PROFILE);

  EXPECT_TRUE(model.setData(model.index(row, 0), 0.1, roleId(model, "leftStickInnerDeadzone")));
  EXPECT_TRUE(model.setData(model.index(row, 0), 1, roleId(model, "rightStickCurve")));

  ASSERT_EQ(repository.analogCalls.size(), 2u);
  EXPECT_EQ(repository.analogCalls.front().profileId, PAD_PROFILE);
  EXPECT_FLOAT_EQ(repository.analogCalls.back().settings.leftStick.innerDeadzone, 0.1f);
  EXPECT_EQ(repository.analogCalls.back().settings.rightStick.curve, ResponseCurve::Exponential);
  EXPECT_FLOAT_EQ(static_cast<float>(valueAt(model, row, "leftStickInnerDeadzone").toDouble()), 0.1f);
  EXPECT_EQ(valueAt(model, row, "rightStickCurve").toInt(), 1);
  EXPECT_EQ(changed.count(), 2);
  EXPECT_EQ(announcedProfileIds, (std::vector<int>{PAD_PROFILE, PAD_PROFILE}));
}

TEST_F(ProfileListModelTest, WritingTheStoredValueAgainIsANoOp) {
  ProfileListModel model;
  QSignalSpy changed(&model, &QAbstractItemModel::dataChanged);
  const auto row = rowOf(model, PAD_PROFILE);

  EXPECT_FALSE(model.setData(model.index(row, 0), 0.25, roleId(model, "leftStickInnerDeadzone")));
  EXPECT_FALSE(model.setData(model.index(row, 0), "Pad", roleId(model, "name")));

  EXPECT_TRUE(repository.analogCalls.empty());
  EXPECT_TRUE(repository.renameCalls.empty());
  EXPECT_EQ(changed.count(), 0);
}

TEST_F(ProfileListModelTest, ReadOnlyRolesRefuseWrites) {
  ProfileListModel model;
  const auto row = rowOf(model, PAD_PROFILE);

  EXPECT_FALSE(model.setData(model.index(row, 0), "other", roleId(model, "icon")));
  EXPECT_FALSE(model.setData(model.index(row, 0), 5, roleId(model, "profileId")));
  EXPECT_FALSE(model.setData(model.index(row, 0), true, roleId(model, "isKeyboard")));
  EXPECT_FALSE(model.setData(model.index(row, 0), QVariant::fromValue(QList<int>{3}), roleId(model, "playerSlots")));

  EXPECT_EQ(valueAt(model, row, "icon").toString(), "xbox");
}

TEST_F(ProfileListModelTest, PlayerSlotsFollowDeviceChanges) {
  ProfileListModel model;
  QSignalSpy changed(&model, &QAbstractItemModel::dataChanged);
  const auto pad = rowOf(model, PAD_PROFILE);

  inputService.devices.back()->setPlayerIndex(3);
  EventDispatcher::instance().publish(GamepadOrderChangedEvent{});
  QCoreApplication::processEvents();

  EXPECT_EQ(valueAt(model, pad, "playerSlots").value<QList<int>>(), (QList<int>{0, 3}));
  ASSERT_EQ(changed.count(), 1);
  EXPECT_EQ(changed.first().at(0).toModelIndex().row(), pad);
  EXPECT_EQ(changed.first().at(2).value<QList<int>>(), (QList<int>{roleId(model, "playerSlots")}));

  inputService.devices.pop_back();
  EventDispatcher::instance().publish(GamepadDisconnectedEvent{3});
  QCoreApplication::processEvents();

  EXPECT_EQ(valueAt(model, pad, "playerSlots").value<QList<int>>(), (QList<int>{0}));
  EXPECT_EQ(changed.count(), 2);
}

TEST_F(ProfileListModelTest, ANewDeviceWithANewProfileGetsARow) {
  ProfileListModel model;
  repository.addProfile(102, "Default Pad Profile");
  addDevice(3, 102);

  EventDispatcher::instance().publish(GamepadConnectedEvent{inputService.devices.back()});
  QCoreApplication::processEvents();

  ASSERT_EQ(model.rowCount({}), 3);
  const auto row = rowOf(model, 102);
  ASSERT_GE(row, 0);
  EXPECT_EQ(valueAt(model, row, "playerSlots").value<QList<int>>(), (QList<int>{3}));
}

TEST_F(ProfileListModelTest, TheRenameInvokablePersistsAndAnnounces) {
  ProfileListModel model;

  EXPECT_TRUE(model.renameProfile(PAD_PROFILE, "Gamepad"));
  EXPECT_FALSE(model.renameProfile(PAD_PROFILE, "Keys"));

  EXPECT_EQ(valueAt(model, rowOf(model, PAD_PROFILE), "name").toString(), "Gamepad");
  EXPECT_EQ(announcedProfileIds, std::vector<int>{PAD_PROFILE});
}

TEST_F(ProfileListModelTest, AnEventForADeletedProfileReloadsTheList) {
  ProfileListModel model;
  QSignalSpy reset(&model, &QAbstractItemModel::modelReset);

  repository.deleteProfile(PAD_PROFILE);
  EventDispatcher::instance().publish(ProfileSettingsChangedEvent{PAD_PROFILE});
  QCoreApplication::processEvents();

  EXPECT_EQ(model.rowCount({}), 1);
  EXPECT_EQ(reset.count(), 1);
}

TEST_F(ProfileListModelTest, ANonNumericAnalogValueIsRefused) {
  ProfileListModel model;
  QSignalSpy changed(&model, &QAbstractItemModel::dataChanged);
  const auto row = rowOf(model, PAD_PROFILE);

  EXPECT_FALSE(model.setData(model.index(row, 0), "abc", roleId(model, "leftStickSensitivity")));

  EXPECT_TRUE(repository.analogCalls.empty());
  EXPECT_EQ(changed.count(), 0);
}

TEST_F(ProfileListModelTest, ASettingsChangeElsewhereRefreshesTheRow) {
  ProfileListModel model;
  QSignalSpy changed(&model, &QAbstractItemModel::dataChanged);
  const auto row = rowOf(model, PAD_PROFILE);

  auto settings = AnalogSettings{};
  settings.leftStick.sensitivity = 1.5f;
  const auto profile = repository.getProfile(PAD_PROFILE);
  profile->setName("Renamed");
  profile->setDefaultAnalogSettings(settings);
  EventDispatcher::instance().publish(ProfileSettingsChangedEvent{PAD_PROFILE});
  QCoreApplication::processEvents();

  EXPECT_EQ(valueAt(model, row, "name").toString(), "Renamed");
  EXPECT_DOUBLE_EQ(valueAt(model, row, "leftStickSensitivity").toDouble(), 1.5);
  EXPECT_EQ(changed.count(), 1);
}

TEST_F(ProfileListModelTest, ItsOwnWritesDoNotEchoBack) {
  ProfileListModel model;
  QSignalSpy changed(&model, &QAbstractItemModel::dataChanged);
  const auto row = rowOf(model, PAD_PROFILE);

  model.setData(model.index(row, 0), "Gamepad", roleId(model, "name"));
  QCoreApplication::processEvents();

  EXPECT_EQ(changed.count(), 1);
}

TEST_F(ProfileListModelTest, PassesQtsModelChecks) {
  ProfileListModel model;
  const QAbstractItemModelTester tester(&model, QAbstractItemModelTester::FailureReportingMode::Fatal);

  model.setData(model.index(rowOf(model, PAD_PROFILE), 0), 0.3, roleId(model, "rightStickSensitivity"));
  model.refresh();

  EXPECT_EQ(model.rowCount({}), 2);
}

} // namespace firelight::input

#include "app/input/gui/controller_list_model.hpp"

#include "../../../libs/firelight/input/tests/test_gamepad.hpp"
#include "../emulation/fake_input_service.hpp"
#include "service_accessor.hpp"

#include <firelight/event_dispatcher.hpp>
#include <firelight/input/gamepad_profile.hpp>
#include <firelight/input/input_service.hpp>

#include <QAbstractItemModelTester>
#include <QCoreApplication>
#include <QSignalSpy>
#include <gtest/gtest.h>
#include <memory>

namespace firelight::gui {

/**
 * Three pads in the first three slots, told apart by profile id, behind the locator the model reads from
 */
class ControllerListModelTest : public testing::Test {
protected:
  emulation::FakeInputService inputService;

  void SetUp() override {
    for (int slot = 0; slot < 3; ++slot) {
      auto pad = std::make_shared<input::TestGamepad>(slot + 1);
      pad->setProfile(std::make_shared<input::GamepadProfile>(profileIdFor(slot)));
      pad->setPlayerIndex(slot);
      inputService.playerSlots[slot] = pad;
    }

    ServiceAccessor::setInputService(&inputService);
  }

  void TearDown() override { ServiceAccessor::setInputService(nullptr); }

  static int profileIdFor(const int slot) { return 100 + slot; }

  static QList<int> roleValues(const QAbstractItemModel &model, const char *roleName) {
    const auto role = model.roleNames().key(roleName);
    QList<int> out;
    for (int row = 0; row < model.rowCount({}); ++row) {
      out.append(model.data(model.index(row, 0), role).toInt());
    }

    return out;
  }
};

TEST_F(ControllerListModelTest, ShowsTheFirstFourSlotsFromTheService) {
  const ControllerListModel model;

  EXPECT_EQ(roleValues(model, "profile_id"), (QList<int>{100, 101, 102, -1}));
  EXPECT_EQ(roleValues(model, "player_index"), (QList<int>{0, 1, 2, 3}));
}

TEST_F(ControllerListModelTest, ASingleMoveRotatesTheRowsWithoutAReset) {
  ControllerListModel model;
  QAbstractItemModelTester tester(&model, QAbstractItemModelTester::FailureReportingMode::Fatal);
  QSignalSpy moved(&model, &QAbstractItemModel::rowsMoved);
  QSignalSpy reset(&model, &QAbstractItemModel::modelReset);

  EventDispatcher::instance().publish(input::GamepadOrderChangedEvent{.from = 0, .to = 2});

  EXPECT_EQ(roleValues(model, "profile_id"), (QList<int>{101, 102, 100, -1}));
  EXPECT_EQ(roleValues(model, "player_index"), (QList<int>{0, 1, 2, 3}));
  EXPECT_EQ(moved.count(), 1);
  EXPECT_EQ(reset.count(), 0);
}

TEST_F(ControllerListModelTest, AMoveBackwardsRotatesTheOtherWay) {
  ControllerListModel model;
  QAbstractItemModelTester tester(&model, QAbstractItemModelTester::FailureReportingMode::Fatal);

  EventDispatcher::instance().publish(input::GamepadOrderChangedEvent{.from = 2, .to = 0});

  EXPECT_EQ(roleValues(model, "profile_id"), (QList<int>{102, 100, 101, -1}));
  EXPECT_EQ(roleValues(model, "player_index"), (QList<int>{0, 1, 2, 3}));
}

TEST_F(ControllerListModelTest, AnUnspecifiedOrderChangeReloadsFromTheService) {
  ControllerListModel model;
  QSignalSpy reset(&model, &QAbstractItemModel::modelReset);
  std::swap(inputService.playerSlots[0], inputService.playerSlots[2]);

  EventDispatcher::instance().publish(input::GamepadOrderChangedEvent{});
  QCoreApplication::processEvents();

  EXPECT_EQ(roleValues(model, "profile_id"), (QList<int>{102, 101, 100, -1}));
  EXPECT_EQ(reset.count(), 1);
}

TEST_F(ControllerListModelTest, AMoveOutsideTheShownRowsReloads) {
  ControllerListModel model;
  QSignalSpy moved(&model, &QAbstractItemModel::rowsMoved);
  QSignalSpy reset(&model, &QAbstractItemModel::modelReset);

  EventDispatcher::instance().publish(input::GamepadOrderChangedEvent{.from = 0, .to = 7});

  EXPECT_EQ(roleValues(model, "profile_id"), (QList<int>{100, 101, 102, -1}));
  EXPECT_EQ(moved.count(), 0);
  EXPECT_EQ(reset.count(), 1);
}

} // namespace firelight::gui

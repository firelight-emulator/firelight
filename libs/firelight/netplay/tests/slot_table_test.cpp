#include <firelight/netplay/slot_table.hpp>

#include <gtest/gtest.h>

namespace firelight::netplay {

TEST(SlotTableTest, AssignClearAndBounds) {
  SlotTable table;
  EXPECT_TRUE(table.assign(0, {.memberId = 1, .displayName = "A"}));
  EXPECT_TRUE(table.assign(7, {.memberId = 2, .displayName = "B"}));
  EXPECT_FALSE(table.assign(8, {.memberId = 3}));
  EXPECT_FALSE(table.assign(-1, {.memberId = 3}));
  EXPECT_FALSE(table.slot(8).has_value());

  EXPECT_EQ(table.occupiedCount(), 2);
  table.clear(0);
  EXPECT_FALSE(table.slot(0).has_value());
  EXPECT_EQ(table.occupiedCount(), 1);
}

TEST(SlotTableTest, MemberCanHoldMultipleSlots) {
  SlotTable table;
  table.assign(0, {.memberId = 1, .localPadIndex = 0});
  table.assign(1, {.memberId = 1, .localPadIndex = 1});
  table.assign(2, {.memberId = 2, .localPadIndex = 0});

  EXPECT_EQ(table.slotsFor(1), (std::vector<int>{0, 1}));

  table.setReady(1, true);
  EXPECT_TRUE(table.slot(0)->ready);
  EXPECT_TRUE(table.slot(1)->ready);
  EXPECT_FALSE(table.slot(2)->ready);

  table.clearMember(1);
  EXPECT_FALSE(table.slot(0).has_value());
  EXPECT_FALSE(table.slot(1).has_value());
  EXPECT_TRUE(table.slot(2).has_value());
}

TEST(SlotTableTest, RemoteOccupantDetection) {
  SlotTable table;
  const PlayerId hostId = 10;
  table.assign(0, {.memberId = hostId});
  EXPECT_FALSE(table.anyRemoteOccupant(hostId));
  table.assign(1, {.memberId = 20});
  EXPECT_TRUE(table.anyRemoteOccupant(hostId));
}

} // namespace firelight::netplay

// TODO: NEEDS REVIEW
#include "gui/selection_group.hpp"

#include <QSignalSpy>
#include <gtest/gtest.h>
#include <vector>

// What a click, a shift-click and a clear do to a selection, and what the view is told afterwards
namespace firelight::gui {

class SelectionGroupTest : public testing::Test {
protected:
  SelectionGroup m_group;

  static constexpr int NONE = Qt::NoModifier;
  static constexpr int SHIFT = Qt::ShiftModifier;
  static constexpr int CONTROL = Qt::ControlModifier;

  // TODO
  // The selected indices in order, read the way a delegate reads them
  [[nodiscard]] std::vector<int> selected(const int upTo = 14) const {
    std::vector<int> indices;

    for (auto i = 0; i < upTo; ++i) {
      if (m_group.isSelected(i)) {
        indices.push_back(i);
      }
    }

    return indices;
  }
};

TEST_F(SelectionGroupTest, NothingIsSelectedToStart) {
  EXPECT_TRUE(selected().empty());
  EXPECT_FALSE(m_group.isSelected(0));
  EXPECT_FALSE(m_group.areMultipleSelected());
  EXPECT_EQ(m_group.getRangeStart(), -1);
  EXPECT_EQ(m_group.getRangeEnd(), -1);
  EXPECT_FALSE(m_group.isEditingRange());
}

TEST_F(SelectionGroupTest, SelectingAnIndexSelectsIt) {
  m_group.select(3, NONE);

  EXPECT_EQ(selected(), std::vector{3});
}

TEST_F(SelectionGroupTest, SelectingTheSameIndexAgainDeselectsIt) {
  m_group.select(3, NONE);
  m_group.select(3, NONE);

  EXPECT_TRUE(selected().empty());
}

TEST_F(SelectionGroupTest, SelectionsAccumulate) {
  m_group.select(3, NONE);
  m_group.select(7, NONE);

  EXPECT_EQ(selected(), (std::vector{3, 7}));
}

// TODO
// Plain clicking already toggles, so the modifier has nothing left to add
TEST_F(SelectionGroupTest, ControlBehavesLikeAPlainClick) {
  m_group.select(3, CONTROL);
  m_group.select(7, CONTROL);
  m_group.select(3, CONTROL);

  EXPECT_EQ(selected(), std::vector{7});
}

TEST_F(SelectionGroupTest, TheMapIsKeyedByIndexAsAString) {
  m_group.select(3, NONE);

  const auto map = m_group.getSelected();
  EXPECT_TRUE(map.value("3").toBool());
  EXPECT_FALSE(map.contains("4"));
}

TEST_F(SelectionGroupTest, TheMapFollowsTheSelection) {
  m_group.select(3, NONE);
  ASSERT_TRUE(m_group.getSelected().contains("3"));

  m_group.select(3, NONE);
  EXPECT_FALSE(m_group.getSelected().contains("3"));

  m_group.select(9, NONE);
  EXPECT_TRUE(m_group.getSelected().contains("9"));
}

TEST_F(SelectionGroupTest, MultipleSelectedNeedsMoreThanOne) {
  EXPECT_FALSE(m_group.areMultipleSelected());

  m_group.select(3, NONE);
  EXPECT_FALSE(m_group.areMultipleSelected());

  m_group.select(4, NONE);
  EXPECT_TRUE(m_group.areMultipleSelected());
}

//****************
// ranges
//****************

TEST_F(SelectionGroupTest, ShiftWithoutAnAnchorSelectsOneIndex) {
  m_group.select(4, SHIFT);

  EXPECT_EQ(selected(), std::vector{4});
}

TEST_F(SelectionGroupTest, ShiftSelectsTheRangeFromTheAnchor) {
  m_group.select(5, NONE);
  m_group.select(10, SHIFT);

  EXPECT_EQ(selected(), (std::vector{5, 6, 7, 8, 9, 10}));
}

TEST_F(SelectionGroupTest, ShiftBackwardsSelectsTheSameRange) {
  m_group.select(10, NONE);
  m_group.select(5, SHIFT);

  EXPECT_EQ(selected(), (std::vector{5, 6, 7, 8, 9, 10}));
}

// TODO
// The endpoint can be pulled back, which is what the baseline behind paintTo is for
TEST_F(SelectionGroupTest, PullingTheRangeBackReleasesWhatItPassed) {
  m_group.select(5, NONE);
  m_group.select(10, SHIFT);
  m_group.select(7, SHIFT);

  EXPECT_EQ(selected(), (std::vector{5, 6, 7}));
}

TEST_F(SelectionGroupTest, ASelectionMadeBeforeTheAnchorSurvivesTheRange) {
  m_group.select(2, NONE);
  m_group.select(5, NONE);
  m_group.select(10, SHIFT);
  m_group.select(7, SHIFT);

  EXPECT_EQ(selected(), (std::vector{2, 5, 6, 7}));
}

// TODO
// A click that turns its index off makes the range remove rather than add
TEST_F(SelectionGroupTest, AnAnchorTurnedOffRemovesTheRange) {
  m_group.select(5, NONE);
  m_group.select(6, NONE);
  m_group.select(7, NONE);
  ASSERT_EQ(selected(), (std::vector{5, 6, 7}));

  m_group.select(5, NONE);
  m_group.select(7, SHIFT);

  EXPECT_TRUE(selected().empty());
}

TEST_F(SelectionGroupTest, ShiftingTwiceIsIdempotent) {
  m_group.select(5, NONE);
  m_group.select(10, SHIFT);
  const auto once = selected();

  m_group.select(10, SHIFT);

  EXPECT_EQ(selected(), once);
}

TEST_F(SelectionGroupTest, TheRangeIsReported) {
  m_group.select(5, NONE);
  m_group.select(10, SHIFT);

  EXPECT_EQ(m_group.getRangeStart(), 5);
  EXPECT_EQ(m_group.getRangeEnd(), 10);
  EXPECT_TRUE(m_group.isEditingRange());
}

TEST_F(SelectionGroupTest, APlainSelectionClearsTheReportedRange) {
  m_group.select(5, NONE);
  m_group.select(10, SHIFT);
  m_group.select(1, NONE);

  EXPECT_EQ(m_group.getRangeStart(), -1);
  EXPECT_EQ(m_group.getRangeEnd(), -1);
  EXPECT_FALSE(m_group.isEditingRange());
}

//****************
// clearing
//****************

TEST_F(SelectionGroupTest, ClearingEmptiesTheSelection) {
  m_group.select(5, NONE);
  m_group.select(10, SHIFT);

  m_group.clearSelection();

  EXPECT_TRUE(selected().empty());
}

// TODO
// A shift after a clear has no anchor to paint from, so it selects only what it names
TEST_F(SelectionGroupTest, ClearingResetsTheAnchorSoAShiftStartsFresh) {
  m_group.select(5, NONE);
  m_group.select(10, SHIFT);
  m_group.clearSelection();

  m_group.select(3, SHIFT);

  EXPECT_EQ(selected(), std::vector{3});
}

TEST_F(SelectionGroupTest, ClearingClearsTheReportedRange) {
  m_group.select(5, NONE);
  m_group.select(10, SHIFT);

  m_group.clearSelection();

  EXPECT_EQ(m_group.getRangeStart(), -1);
  EXPECT_EQ(m_group.getRangeEnd(), -1);
  EXPECT_FALSE(m_group.isEditingRange());
}

// TODO
// A caller can be mid-gesture with nothing selected, and clearing has to end that too
TEST_F(SelectionGroupTest, ClearingEndsARangeGestureWithNothingSelected) {
  m_group.setEditingRange(true);

  QSignalSpy spy(&m_group, &SelectionGroup::selectedChanged);
  m_group.clearSelection();

  EXPECT_FALSE(m_group.isEditingRange());
  EXPECT_EQ(spy.count(), 1);
}

//****************
// notifications
//****************

TEST_F(SelectionGroupTest, SelectingAnnouncesTheChange) {
  QSignalSpy spy(&m_group, &SelectionGroup::selectedChanged);

  m_group.select(3, NONE);
  EXPECT_EQ(spy.count(), 1);

  m_group.select(6, SHIFT);
  EXPECT_EQ(spy.count(), 2);

  m_group.clearSelection();
  EXPECT_EQ(spy.count(), 3);
}

TEST_F(SelectionGroupTest, ClearingNothingAnnouncesNothing) {
  QSignalSpy spy(&m_group, &SelectionGroup::selectedChanged);

  m_group.clearSelection();

  EXPECT_EQ(spy.count(), 0);
}

// TODO
// Leaving selection mode drops everything, so a later mode change cannot bring a stale set back
TEST_F(SelectionGroupTest, LeavingSelectionModeClearsTheSelection) {
  m_group.setActive(true);
  m_group.select(5, NONE);
  m_group.select(10, SHIFT);
  ASSERT_FALSE(selected().empty());

  m_group.setActive(false);

  EXPECT_TRUE(selected().empty());
  EXPECT_EQ(m_group.getRangeStart(), -1);
  EXPECT_EQ(m_group.getRangeEnd(), -1);
  EXPECT_FALSE(m_group.isEditingRange());
}

// TODO
// A gesture the caller never ended is stranded until something clears it
TEST_F(SelectionGroupTest, LeavingSelectionModeEndsAnUnfinishedRangeGesture) {
  m_group.setActive(true);
  m_group.setEditingRange(true);

  m_group.setActive(false);

  EXPECT_FALSE(m_group.isEditingRange());
}

TEST_F(SelectionGroupTest, EnteringSelectionModeKeepsWhatIsSelected) {
  m_group.select(5, NONE);

  m_group.setActive(true);

  EXPECT_EQ(selected(), std::vector{5});
}

TEST_F(SelectionGroupTest, LeavingSelectionModeAnnouncesTheModeAndTheSelection) {
  m_group.setActive(true);
  m_group.select(5, NONE);

  QSignalSpy activeSpy(&m_group, &SelectionGroup::activeChanged);
  QSignalSpy selectedSpy(&m_group, &SelectionGroup::selectedChanged);

  m_group.setActive(false);

  EXPECT_EQ(activeSpy.count(), 1);
  EXPECT_EQ(selectedSpy.count(), 1);
}

// TODO
// Turning the mode off when it is already off touches nothing
TEST_F(SelectionGroupTest, LeavingSelectionModeTwiceClearsOnlyOnce) {
  m_group.setActive(true);
  m_group.select(5, NONE);
  m_group.setActive(false);

  m_group.select(7, NONE);
  m_group.setActive(false);

  EXPECT_EQ(selected(), std::vector{7});
}

TEST_F(SelectionGroupTest, ActiveAnnouncesOnlyOnChange) {
  QSignalSpy spy(&m_group, &SelectionGroup::activeChanged);
  EXPECT_FALSE(m_group.isActive());

  m_group.setActive(true);
  EXPECT_TRUE(m_group.isActive());
  EXPECT_EQ(spy.count(), 1);

  m_group.setActive(true);
  EXPECT_EQ(spy.count(), 1);

  m_group.setActive(false);
  EXPECT_EQ(spy.count(), 2);
}

} // namespace firelight::gui

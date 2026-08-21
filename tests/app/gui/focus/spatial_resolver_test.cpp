#include "gui/focus/spatial_resolver.hpp"

#include <gtest/gtest.h>

namespace firelight::gui {

namespace {
// A 4-wide grid of 100x100 tiles at 10px spacing, indices reading left to right
QRectF tile(const int index, const int columns = 4) {
  const auto column = index % columns;
  const auto row = index / columns;

  return {column * 110.0, row * 110.0, 100.0, 100.0};
}

std::vector<QRectF> grid(const int count, const int columns = 4) {
  std::vector<QRectF> rects;

  for (auto i = 0; i < count; ++i) {
    rects.push_back(tile(i, columns));
  }

  return rects;
}

int pick(const QRectF &origin, const Direction direction, const std::vector<QRectF> &candidates) {
  return SpatialResolver::pick(origin, direction, candidates);
}
} // namespace

TEST(SpatialResolverTest, NothingToMoveToIsNone) {
  EXPECT_EQ(pick(tile(0), Direction::Down, {}), SpatialResolver::NONE);
}

TEST(SpatialResolverTest, TheOriginIsNeverItsOwnAnswer) {
  const std::vector<QRectF> only = {tile(0)};
  EXPECT_EQ(pick(tile(0), Direction::Down, only), SpatialResolver::NONE);
  EXPECT_EQ(pick(tile(0), Direction::Right, only), SpatialResolver::NONE);
}

TEST(SpatialResolverTest, MovesToTheTileDirectlyBelow) {
  const auto rects = grid(12);
  EXPECT_EQ(pick(tile(1), Direction::Down, rects), 5);
  EXPECT_EQ(pick(tile(5), Direction::Down, rects), 9);
}

TEST(SpatialResolverTest, MovesToTheTileDirectlyAbove) {
  const auto rects = grid(12);
  EXPECT_EQ(pick(tile(9), Direction::Up, rects), 5);
  EXPECT_EQ(pick(tile(5), Direction::Up, rects), 1);
}

TEST(SpatialResolverTest, MovesToTheNeighbouringColumns) {
  const auto rects = grid(12);
  EXPECT_EQ(pick(tile(5), Direction::Right, rects), 6);
  EXPECT_EQ(pick(tile(5), Direction::Left, rects), 4);
}

TEST(SpatialResolverTest, TheEdgesOfTheGridHaveNowhereToGo) {
  const auto rects = grid(12);
  EXPECT_EQ(pick(tile(0), Direction::Up, rects), SpatialResolver::NONE);
  EXPECT_EQ(pick(tile(0), Direction::Left, rects), SpatialResolver::NONE);
  EXPECT_EQ(pick(tile(11), Direction::Down, rects), SpatialResolver::NONE);
  EXPECT_EQ(pick(tile(11), Direction::Right, rects), SpatialResolver::NONE);
}

// The rule that makes a grid behave like a grid: straight ahead always wins,
// even when something diagonal is closer as the crow flies
TEST(SpatialResolverTest, StraightAheadBeatsDiagonal) {
  const std::vector<QRectF> rects = {
      // Diagonal and very close
      {120.0, 105.0, 100.0, 100.0},
      // Directly below but further away
      {0.0, 300.0, 100.0, 100.0},
  };

  EXPECT_EQ(pick({0.0, 0.0, 100.0, 100.0}, Direction::Down, rects), 1);
}

TEST(SpatialResolverTest, PartialOverlapStillCountsAsAhead) {
  const std::vector<QRectF> rects = {
      // Overlaps the origin's right quarter
      {75.0, 200.0, 100.0, 100.0},
      // No overlap at all, and nearer
      {200.0, 150.0, 100.0, 100.0},
  };

  EXPECT_EQ(pick({0.0, 0.0, 100.0, 100.0}, Direction::Down, rects), 0);
}

// A badly aligned candidate should lose to a well aligned one a row further off
TEST(SpatialResolverTest, AlignmentOutweighsASmallDistanceAdvantage) {
  const std::vector<QRectF> rects = {
      // Only 10% overlapping, slightly nearer
      {90.0, 150.0, 100.0, 100.0},
      // Perfectly aligned, slightly further
      {0.0, 200.0, 100.0, 100.0},
  };

  EXPECT_EQ(pick({0.0, 0.0, 100.0, 100.0}, Direction::Down, rects), 1);
}

// A press reaches what it lines up with and nothing else. Without this a column
// of buttons hands Down to a grid it shares no width with, several rows down and
// most of a screen to the right
TEST(SpatialResolverTest, NothingOffToTheSideIsReachable) {
  const std::vector<QRectF> rects = {
      {400.0, 200.0, 100.0, 100.0},
      {200.0, 250.0, 100.0, 100.0},
  };

  EXPECT_EQ(pick({0.0, 0.0, 100.0, 100.0}, Direction::Down, rects), SpatialResolver::NONE);
}

// The library's icon rail, running out of buttons at the bottom
TEST(SpatialResolverTest, ThePressAtTheEndOfARailDoesNotEnterTheGridBesideIt) {
  const QRectF bottomButton = {16.0, 262.0, 32.0, 32.0};
  const std::vector<QRectF> tiles = {
      {75.0, 407.0, 149.0, 148.0},
      {234.0, 407.0, 149.0, 148.0},
      {75.0, 566.0, 149.0, 148.0},
  };

  EXPECT_EQ(pick(bottomButton, Direction::Down, tiles), SpatialResolver::NONE);
}

// The toolbar-to-grid case: a short button moving into tall tiles should land on
// the one nearest its own centre, not simply the first
TEST(SpatialResolverTest, ASmallItemEnteringALargeColumnLandsNearestItsCentre) {
  const std::vector<QRectF> rects = {
      {200.0, 0.0, 200.0, 200.0},
      {200.0, 210.0, 200.0, 200.0},
      {200.0, 420.0, 200.0, 200.0},
  };

  EXPECT_EQ(pick({0.0, 250.0, 72.0, 36.0}, Direction::Right, rects), 1);
}

// And back again, which is the return path that does not exist today. The
// buttons are centred at y 18, 64 and 110, so the answer is whichever is nearest
// the origin's own centre
TEST(SpatialResolverTest, LeavingLandsOnTheButtonNearestTheOriginsCentre) {
  const std::vector<QRectF> rects = {
      {0.0, 0.0, 72.0, 36.0},
      {0.0, 46.0, 72.0, 36.0},
      {0.0, 92.0, 72.0, 36.0},
  };

  // A button at the same height as the middle one
  EXPECT_EQ(pick({200.0, 50.0, 72.0, 36.0}, Direction::Left, rects), 1);

  // A tall tile centred at y 160, which is nearest the lowest button
  EXPECT_EQ(pick({200.0, 60.0, 200.0, 200.0}, Direction::Left, rects), 2);
}

TEST(SpatialResolverTest, AWideHeaderIsReachableFromAnyButtonBeneathIt) {
  const std::vector<QRectF> header = {{0.0, 0.0, 800.0, 60.0}};

  EXPECT_EQ(pick({10.0, 100.0, 100.0, 40.0}, Direction::Up, header), 0);
  EXPECT_EQ(pick({400.0, 100.0, 100.0, 40.0}, Direction::Up, header), 0);
  EXPECT_EQ(pick({700.0, 100.0, 100.0, 40.0}, Direction::Up, header), 0);
}

TEST(SpatialResolverTest, AnExactTieGoesToWhicheverCameFirst) {
  const std::vector<QRectF> rects = {
      {0.0, 200.0, 100.0, 100.0},
      {0.0, 200.0, 100.0, 100.0},
  };

  EXPECT_EQ(pick({0.0, 0.0, 100.0, 100.0}, Direction::Down, rects), 0);
}

// Sitting inside something is not a reason to move to it, unless its bulk really
// is in the direction pressed
TEST(SpatialResolverTest, AnEnclosingItemCountsOnlyWhenItIsMostlyAhead) {
  const QRectF origin = {100.0, 100.0, 50.0, 50.0};

  const std::vector<QRectF> centred = {{0.0, 0.0, 400.0, 400.0}};
  EXPECT_EQ(pick(origin, Direction::Down, centred), 0);

  const std::vector<QRectF> above = {{0.0, 0.0, 400.0, 160.0}};
  EXPECT_EQ(pick(origin, Direction::Down, above), SpatialResolver::NONE);
}

// Two things overlapping along the direction of travel is the case a grid never
// produces and a real screen does constantly
TEST(SpatialResolverTest, ThingsThatOverlapAreReachableFromBothSides) {
  // A bar, and a panel below it that starts higher than the bar ends
  const QRectF bar = {100.0, 120.0, 680.0, 40.0};
  const QRectF panel = {300.0, 100.0, 400.0, 600.0};

  ASSERT_EQ(pick(bar, Direction::Down, {panel}), 0);
  EXPECT_EQ(pick(panel, Direction::Up, {bar}), 0);
}

// The library's rail sits level with the top row of tiles, far off to the left,
// so nothing above a tile is ever the rail
TEST(SpatialResolverTest, SomethingOffToTheSideIsNotAbove) {
  const QRectF topRightTile = {873.0, 88.0, 149.0, 148.0};
  const std::vector<QRectF> rail = {
      {16.0, 65.0, 32.0, 32.0},
      {16.0, 113.0, 32.0, 32.0},
      {16.0, 161.0, 32.0, 32.0},
  };

  EXPECT_EQ(pick(topRightTile, Direction::Up, rail), SpatialResolver::NONE);

  // Still not above once it is clear of the tile: it shares none of its width
  const std::vector<QRectF> higher = {{16.0, 20.0, 32.0, 32.0}};
  EXPECT_EQ(pick(topRightTile, Direction::Up, higher), SpatialResolver::NONE);
}

// Sideways the lane is the vertical span, so the rail is reachable from a tile
// level with it, and from nothing else
TEST(SpatialResolverTest, TheRailIsReachedFromWhatIsLevelWithIt) {
  const std::vector<QRectF> rail = {
      {16.0, 65.0, 32.0, 32.0},
      {16.0, 113.0, 32.0, 32.0},
      {16.0, 161.0, 32.0, 32.0},
  };

  // Level with the rail. Geometry answers with the button nearest the tile's own
  // centre; getting back to the one the cursor arrived from is the navigator's job
  EXPECT_EQ(pick({75.0, 88.0, 149.0, 148.0}, Direction::Left, rail), 2);

  // Far below the whole rail: to the left of it, but level with none of it
  EXPECT_EQ(pick({75.0, 900.0, 149.0, 148.0}, Direction::Left, rail), SpatialResolver::NONE);
}

//****************
// entering a scrolling view
//****************

// A scrolled grid shows whatever rows happen to be on screen, so the row a press
// should land on may line up with nothing
TEST(SpatialResolverTest, EnteringTakesTheRowNearestTheLane) {
  const QRectF topButton = {16.0, 65.0, 32.0, 32.0};
  const std::vector<QRectF> tiles = {
      {75.0, 120.0, 149.0, 148.0},
      {234.0, 120.0, 149.0, 148.0},
      {75.0, 279.0, 149.0, 148.0},
      {234.0, 279.0, 149.0, 148.0},
  };

  // Nothing shares the button's 32px lane, so the ordinary rule refuses
  ASSERT_EQ(pick(topButton, Direction::Right, tiles), SpatialResolver::NONE);

  // Entering takes the nearest row, then the nearest of that row
  EXPECT_EQ(SpatialResolver::pickNearestLane(topButton, Direction::Right, tiles), 0);
}

TEST(SpatialResolverTest, EnteringPrefersARowItDoesLineUpWith) {
  const QRectF button = {16.0, 200.0, 32.0, 32.0};
  const std::vector<QRectF> tiles = {
      {75.0, 20.0, 149.0, 148.0},
      {75.0, 190.0, 149.0, 148.0},
  };

  EXPECT_EQ(SpatialResolver::pickNearestLane(button, Direction::Right, tiles), 1);
}

TEST(SpatialResolverTest, EnteringStillOnlyLooksTheWayThePressPoints) {
  const QRectF button = {500.0, 65.0, 32.0, 32.0};
  const std::vector<QRectF> behind = {{75.0, 120.0, 149.0, 148.0}};

  EXPECT_EQ(SpatialResolver::pickNearestLane(button, Direction::Right, behind), SpatialResolver::NONE);
  EXPECT_EQ(SpatialResolver::pickNearestLane(button, Direction::Left, behind), 0);
}

// Whatever the layout, a press that lands somewhere must leave the way back
// open, or the cursor strands itself where no key gets it out
TEST(SpatialResolverTest, NoMoveLeadsSomewhereWithNoWayBack) {
  const std::vector<Direction> directions = {Direction::Up, Direction::Down, Direction::Left, Direction::Right};
  const std::vector<Direction> opposites = {Direction::Down, Direction::Up, Direction::Right, Direction::Left};

  // Overlapping, enclosing and ragged all at once, which nothing above covers
  const std::vector<QRectF> layered = {
      {620.0, 540.0, 400.0, 40.0}, {360.0, 200.0, 800.0, 500.0}, {420.0, 580.0, 100.0, 40.0},
      {0.0, 0.0, 1280.0, 60.0},    {40.0, 90.0, 48.0, 400.0},    {900.0, 20.0, 200.0, 200.0},
      {-100.0, 300.0, 90.0, 90.0}, {700.0, 690.0, 300.0, 120.0},
  };

  for (size_t i = 0; i < layered.size(); ++i) {
    for (size_t d = 0; d < directions.size(); ++d) {
      const auto landed = pick(layered[i], directions[d], layered);

      if (landed == SpatialResolver::NONE) {
        continue;
      }

      EXPECT_NE(pick(layered[landed], opposites[d], layered), SpatialResolver::NONE)
          << "from " << i << " direction " << d << " landed on " << landed << " with no way back";
    }
  }
}

TEST(SpatialResolverTest, SomethingBarelyOffsetIsNotAhead) {
  const std::vector<QRectF> rects = {{110.0, 0.5, 100.0, 100.0}};
  EXPECT_EQ(pick({0.0, 0.0, 100.0, 100.0}, Direction::Down, rects), SpatialResolver::NONE);
}

TEST(SpatialResolverTest, EmptyRectanglesAreIgnored) {
  const std::vector<QRectF> rects = {
      {0.0, 200.0, 0.0, 0.0},
      {0.0, 300.0, 100.0, 100.0},
  };

  EXPECT_EQ(pick({0.0, 0.0, 100.0, 100.0}, Direction::Down, rects), 1);
  EXPECT_EQ(pick({0.0, 0.0, 0.0, 0.0}, Direction::Down, rects), SpatialResolver::NONE);
}

// Every direction has to behave the same way, so the same layout is walked
// symmetrically rather than one axis being subtly better than the other
TEST(SpatialResolverTest, WalkingTheGridInEitherDirectionIsSymmetric) {
  const auto rects = grid(12);

  for (auto i = 0; i < 8; ++i) {
    const auto down = pick(tile(i), Direction::Down, rects);
    ASSERT_NE(down, SpatialResolver::NONE) << "index " << i;
    EXPECT_EQ(pick(tile(down), Direction::Up, rects), i) << "index " << i;
  }

  for (auto i = 0; i < 11; ++i) {
    if (i % 4 == 3) {
      continue;
    }

    const auto right = pick(tile(i), Direction::Right, rects);
    ASSERT_NE(right, SpatialResolver::NONE) << "index " << i;
    EXPECT_EQ(pick(tile(right), Direction::Left, rects), i) << "index " << i;
  }
}

// A tile in a short final row is reachable from the one above it in its own
// column. Reaching it from a column that runs off the end of the row is the
// grid's own stepping to answer, not this: Nav.gridStep clamps into the short
// row, and nothing here shares a lane to cross by
TEST(SpatialResolverTest, AShortFinalRowIsReachableColumnByColumn) {
  const auto rects = grid(10);

  EXPECT_EQ(pick(tile(4), Direction::Down, rects), 8);
  EXPECT_EQ(pick(tile(5), Direction::Down, rects), 9);
  EXPECT_EQ(pick(tile(6), Direction::Down, rects), SpatialResolver::NONE);
  EXPECT_EQ(pick(tile(7), Direction::Down, rects), SpatialResolver::NONE);
}

TEST(SpatialResolverTest, ASingleColumnWalksVertically) {
  const auto rects = grid(4, 1);

  EXPECT_EQ(pick(tile(0, 1), Direction::Down, rects), 1);
  EXPECT_EQ(pick(tile(3, 1), Direction::Down, rects), SpatialResolver::NONE);
  EXPECT_EQ(pick(tile(0, 1), Direction::Right, rects), SpatialResolver::NONE);
}

TEST(SpatialResolverTest, ASingleRowWalksHorizontally) {
  const auto rects = grid(4, 4);

  EXPECT_EQ(pick(tile(0), Direction::Right, rects), 1);
  EXPECT_EQ(pick(tile(3), Direction::Right, rects), SpatialResolver::NONE);
  EXPECT_EQ(pick(tile(0), Direction::Down, rects), SpatialResolver::NONE);
}

// Off-screen is not the resolver's business: an item scrolled just out of view is
// still where it is, and the ring scrolls to whatever gets focused
TEST(SpatialResolverTest, SomethingPastTheViewportEdgeIsStillReachable) {
  const std::vector<QRectF> rects = {{0.0, 900.0, 100.0, 100.0}};
  EXPECT_EQ(pick({0.0, 0.0, 100.0, 100.0}, Direction::Down, rects), 0);
}

TEST(SpatialResolverTest, NegativeCoordinatesBehaveTheSame) {
  const std::vector<QRectF> rects = {
      {-500.0, -300.0, 100.0, 100.0},
      {-500.0, -100.0, 100.0, 100.0},
  };

  EXPECT_EQ(pick({-500.0, -500.0, 100.0, 100.0}, Direction::Down, rects), 0);
  EXPECT_EQ(pick({-500.0, -100.0, 100.0, 100.0}, Direction::Up, rects), 0);
}

} // namespace firelight::gui

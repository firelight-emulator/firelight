#include "gui/focus/focus_navigator.hpp"

#include "gui/focus_info.hpp"

#include <gtest/gtest.h>

#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickItem>

#include <memory>

namespace firelight::gui {

namespace {
QQuickItem *item(QQuickItem *parent, const qreal x, const qreal y, const qreal width, const qreal height,
                 const Qt::FocusPolicy policy = Qt::StrongFocus) {
  auto *created = new QQuickItem(parent);
  created->setPosition({x, y});
  created->setSize(QSizeF(width, height));
  created->setFocusPolicy(policy);

  return created;
}

QQuickItem *container(QQuickItem *parent, const qreal x = 0.0, const qreal y = 0.0, const qreal width = 1000.0,
                      const qreal height = 1000.0) {
  return item(parent, x, y, width, height, Qt::NoFocus);
}

FocusInfo *info(QObject *object) {
  return qobject_cast<FocusInfo *>(qmlAttachedPropertiesObject<FocusInfo>(object, true));
}

bool heldBack(QQuickItem *from, QQuickItem *to, const Direction direction, const bool isAutoRepeat = true) {
  return FocusNavigator::isHeldBack(from, to, direction, isAutoRepeat);
}
} // namespace

//****************
// cadence
//****************

TEST(RepeatGovernorTest, AFreshPressIsNeverThinned) {
  RepeatGovernor governor;

  EXPECT_TRUE(governor.allows(Direction::Down, false, 1000));
  EXPECT_TRUE(governor.allows(Direction::Down, false, 1001));
  EXPECT_TRUE(governor.allows(Direction::Down, false, 1002));
}

TEST(RepeatGovernorTest, AHeldDirectionIsThinnedToTheInterval) {
  RepeatGovernor governor;

  ASSERT_TRUE(governor.allows(Direction::Down, false, 1000));

  EXPECT_FALSE(governor.allows(Direction::Down, true, 1030));
  EXPECT_FALSE(governor.allows(Direction::Down, true, 1000 + RepeatGovernor::INTERVAL_MS - 1));
  EXPECT_TRUE(governor.allows(Direction::Down, true, 1000 + RepeatGovernor::INTERVAL_MS));

  // The clock restarts from the move that was allowed, not from the first press
  EXPECT_FALSE(governor.allows(Direction::Down, true, 1000 + RepeatGovernor::INTERVAL_MS + 30));
}

TEST(RepeatGovernorTest, TurningIsImmediate) {
  RepeatGovernor governor;

  ASSERT_TRUE(governor.allows(Direction::Down, false, 1000));
  EXPECT_TRUE(governor.allows(Direction::Right, true, 1005));
  EXPECT_FALSE(governor.allows(Direction::Right, true, 1010));
}

TEST(RepeatGovernorTest, AHoldThatBeganBeforeAnythingMovedIsNotThinned) {
  RepeatGovernor governor;

  EXPECT_TRUE(governor.allows(Direction::Down, true, 1000));
}

TEST(RepeatGovernorTest, ResetMakesTheNextPressFresh) {
  RepeatGovernor governor;

  ASSERT_TRUE(governor.allows(Direction::Down, false, 1000));
  ASSERT_FALSE(governor.allows(Direction::Down, true, 1010));

  governor.reset();

  EXPECT_TRUE(governor.allows(Direction::Down, true, 1011));
}

//****************
// held edges
//****************

TEST(FocusNavigatorTest, WithNothingDeclaredAHeldDirectionCrossesFreely) {
  QQuickItem root;
  auto *grid = container(&root);
  auto *tile = item(grid, 0.0, 0.0, 100.0, 100.0);
  auto *header = item(&root, 0.0, 200.0, 100.0, 40.0);

  EXPECT_FALSE(heldBack(tile, header, Direction::Up));
}

TEST(FocusNavigatorTest, AHeldDirectionStopsAtADeclaredEdge) {
  QQuickItem root;
  auto *grid = container(&root);
  auto *tile = item(grid, 0.0, 0.0, 100.0, 100.0);
  auto *header = item(&root, 0.0, 200.0, 100.0, 40.0);

  info(grid)->setHoldEdges(FocusInfo::Vertical);

  EXPECT_TRUE(heldBack(tile, header, Direction::Up));
  EXPECT_TRUE(heldBack(tile, header, Direction::Down));
}

TEST(FocusNavigatorTest, PressingAgainCrossesTheEdgeAHoldStoppedAt) {
  QQuickItem root;
  auto *grid = container(&root);
  auto *tile = item(grid, 0.0, 0.0, 100.0, 100.0);
  auto *header = item(&root, 0.0, 200.0, 100.0, 40.0);

  info(grid)->setHoldEdges(FocusInfo::Vertical);

  EXPECT_FALSE(heldBack(tile, header, Direction::Up, false));
}

TEST(FocusNavigatorTest, AnUndeclaredDirectionCrossesEvenWhileHeld) {
  QQuickItem root;
  auto *grid = container(&root);
  auto *tile = item(grid, 0.0, 0.0, 100.0, 100.0);
  auto *toolbar = item(&root, 200.0, 0.0, 40.0, 40.0);

  info(grid)->setHoldEdges(FocusInfo::Vertical);

  EXPECT_FALSE(heldBack(tile, toolbar, Direction::Left));
  EXPECT_FALSE(heldBack(tile, toolbar, Direction::Right));
}

TEST(FocusNavigatorTest, MovingWithinTheDeclaringContainerIsNotStopped) {
  QQuickItem root;
  auto *grid = container(&root);
  auto *first = item(grid, 0.0, 0.0, 100.0, 100.0);
  auto *second = item(grid, 0.0, 110.0, 100.0, 100.0);

  info(grid)->setHoldEdges(FocusInfo::Vertical);

  EXPECT_FALSE(heldBack(first, second, Direction::Down));
}

// Every container up the chain gets a say, so an inner one holding a different
// direction does not excuse leaving an outer one that holds this direction
TEST(FocusNavigatorTest, ARowInsideAListInsideAPageAnswersToBoth) {
  QQuickItem root;
  auto *page = container(&root);
  auto *list = container(page, 0.0, 0.0, 300.0, 300.0);
  auto *row = item(list, 0.0, 0.0, 100.0, 40.0);
  auto *elsewhere = item(&root, 0.0, 500.0, 100.0, 40.0);

  info(page)->setHoldEdges(FocusInfo::Vertical);
  info(list)->setHoldEdges(FocusInfo::Horizontal);

  EXPECT_TRUE(heldBack(row, elsewhere, Direction::Down));
  EXPECT_FALSE(heldBack(row, elsewhere, Direction::Down, false));
}

// The nearest container holding this direction is the edge a hold stops at, so a
// list inside a page stops at the list's own end rather than the page's
TEST(FocusNavigatorTest, AHoldStopsAtTheNearestEdgeThatHoldsIt) {
  QQuickItem root;
  auto *page = container(&root);
  auto *list = container(page, 0.0, 0.0, 300.0, 300.0);
  auto *row = item(list, 0.0, 0.0, 100.0, 40.0);
  auto *nextRow = item(list, 0.0, 50.0, 100.0, 40.0);
  auto *stillOnThePage = item(page, 0.0, 350.0, 100.0, 40.0);

  info(page)->setHoldEdges(FocusInfo::Vertical);
  info(list)->setHoldEdges(FocusInfo::Vertical);

  EXPECT_FALSE(heldBack(row, nextRow, Direction::Down));
  EXPECT_TRUE(heldBack(row, stillOnThePage, Direction::Down));
}

TEST(FocusNavigatorTest, SomethingCanHoldItsOwnEdges) {
  QQuickItem root;
  auto *field = item(&root, 0.0, 0.0, 200.0, 40.0);
  auto *elsewhere = item(&root, 0.0, 100.0, 200.0, 40.0);

  info(field)->setHoldEdges(FocusInfo::AllEdges);

  EXPECT_TRUE(heldBack(field, elsewhere, Direction::Down));
  EXPECT_FALSE(heldBack(field, elsewhere, Direction::Down, false));
}

//****************
// moving
//****************

TEST(FocusNavigatorTest, AKeyThatIsNotADirectionDoesNothing) {
  FocusNavigator navigator;
  QQuickItem root;
  auto *button = item(&root, 0.0, 0.0, 72.0, 36.0);

  EXPECT_EQ(navigator.move(button, Qt::Key_Select, false), FocusNavigator::NoTarget);
  EXPECT_EQ(navigator.move(nullptr, Qt::Key_Down, false), FocusNavigator::NoTarget);
}

TEST(FocusNavigatorTest, MovesToWhatLiesThatWay) {
  FocusNavigator navigator;
  QQuickItem root;
  auto *first = item(&root, 0.0, 0.0, 100.0, 100.0);
  auto *second = item(&root, 0.0, 110.0, 100.0, 100.0);

  EXPECT_EQ(navigator.move(first, Qt::Key_Down, false), FocusNavigator::Moved);
  EXPECT_TRUE(second->hasFocus());
}

TEST(FocusNavigatorTest, WithNothingThatWayThePressGoesNowhere) {
  FocusNavigator navigator;
  QQuickItem root;
  auto *first = item(&root, 0.0, 0.0, 100.0, 100.0);
  item(&root, 0.0, 110.0, 100.0, 100.0);

  EXPECT_EQ(navigator.move(first, Qt::Key_Up, false), FocusNavigator::NoTarget);
}

// Key focus can sit below the thing the cursor is drawn around, and the move has
// to start from what is on screen
TEST(FocusNavigatorTest, TheMoveStartsFromTheThingTheCursorIsOn) {
  FocusNavigator navigator;
  QQuickItem root;
  auto *button = item(&root, 0.0, 0.0, 100.0, 100.0);
  auto *label = item(button, 0.0, 0.0, 20.0, 20.0, Qt::NoFocus);
  auto *below = item(&root, 0.0, 110.0, 100.0, 100.0);

  EXPECT_EQ(navigator.move(label, Qt::Key_Down, false), FocusNavigator::Moved);
  EXPECT_TRUE(below->hasFocus());
}

// A held direction is thinned rather than refused, so the caller never mistakes
// the pause for having run out of room
TEST(FocusNavigatorTest, AThinnedRepeatReportsAMoveWithoutMakingOne) {
  FocusNavigator navigator;
  QQuickItem root;
  auto *first = item(&root, 0.0, 0.0, 100.0, 100.0);
  auto *second = item(&root, 0.0, 110.0, 100.0, 100.0);
  auto *third = item(&root, 0.0, 220.0, 100.0, 100.0);

  ASSERT_EQ(navigator.move(first, Qt::Key_Down, false), FocusNavigator::Moved);
  ASSERT_TRUE(second->hasFocus());

  EXPECT_EQ(navigator.move(second, Qt::Key_Down, true), FocusNavigator::Moved);
  EXPECT_FALSE(third->hasFocus());
}

//****************
// the way back
//****************

// Leaving a short button for a tall tile means leaving from the tile's middle,
// and the button nearest that middle is not the one it started on — so geometry
// alone can never answer "back where I came from"
TEST(FocusNavigatorTest, ReversingAMoveReturnsToWhereItStarted) {
  FocusNavigator navigator;
  QQuickItem root;
  auto *top = item(&root, 0.0, 0.0, 40.0, 40.0);
  auto *middle = item(&root, 0.0, 60.0, 40.0, 40.0);
  auto *tile = item(&root, 60.0, 0.0, 200.0, 200.0);

  ASSERT_EQ(navigator.move(top, Qt::Key_Right, false), FocusNavigator::Moved);
  ASSERT_TRUE(tile->hasFocus());

  EXPECT_EQ(navigator.move(tile, Qt::Key_Left, false), FocusNavigator::Moved);
  EXPECT_TRUE(top->hasFocus());
  EXPECT_FALSE(middle->hasFocus()) << "geometry alone would have picked the nearest button";
}

TEST(FocusNavigatorTest, OnlyTheOppositeDirectionComesBack) {
  FocusNavigator navigator;
  QQuickItem root;
  auto *top = item(&root, 0.0, 0.0, 40.0, 40.0);
  auto *tile = item(&root, 60.0, 0.0, 200.0, 200.0);
  auto *below = item(&root, 60.0, 210.0, 200.0, 200.0);

  ASSERT_EQ(navigator.move(top, Qt::Key_Right, false), FocusNavigator::Moved);

  EXPECT_EQ(navigator.move(tile, Qt::Key_Down, false), FocusNavigator::Moved);
  EXPECT_TRUE(below->hasFocus());
}

TEST(FocusNavigatorTest, TheWayBackIsOnlyGoodFromWhereTheMoveLanded) {
  FocusNavigator navigator;
  QQuickItem root;
  auto *top = item(&root, 0.0, 0.0, 40.0, 40.0);
  auto *tile = item(&root, 60.0, 0.0, 200.0, 200.0);
  auto *below = item(&root, 60.0, 210.0, 200.0, 200.0);
  auto *beside = item(&root, 0.0, 240.0, 40.0, 40.0);

  ASSERT_EQ(navigator.move(top, Qt::Key_Right, false), FocusNavigator::Moved);

  // Pressing Left from somewhere the move never landed is an ordinary press
  EXPECT_EQ(navigator.move(below, Qt::Key_Left, false), FocusNavigator::Moved);
  EXPECT_TRUE(beside->hasFocus());
  EXPECT_FALSE(top->hasFocus());
  EXPECT_FALSE(tile->hasFocus());
}

TEST(FocusNavigatorTest, ForgettingTheWayBackLeavesGeometry) {
  FocusNavigator navigator;
  QQuickItem root;
  auto *top = item(&root, 0.0, 0.0, 40.0, 40.0);
  auto *middle = item(&root, 0.0, 60.0, 40.0, 40.0);
  auto *tile = item(&root, 60.0, 0.0, 200.0, 200.0);

  ASSERT_EQ(navigator.move(top, Qt::Key_Right, false), FocusNavigator::Moved);
  navigator.forget();

  EXPECT_EQ(navigator.move(tile, Qt::Key_Left, false), FocusNavigator::Moved);
  EXPECT_TRUE(middle->hasFocus());
}

TEST(FocusNavigatorTest, AHeldDirectionNeverComesBack) {
  FocusNavigator navigator;
  QQuickItem root;
  auto *top = item(&root, 0.0, 0.0, 40.0, 40.0);
  auto *middle = item(&root, 0.0, 60.0, 40.0, 40.0);
  auto *tile = item(&root, 60.0, 0.0, 200.0, 200.0);

  ASSERT_EQ(navigator.move(top, Qt::Key_Right, false), FocusNavigator::Moved);

  EXPECT_EQ(navigator.move(tile, Qt::Key_Left, true), FocusNavigator::Moved);
  EXPECT_TRUE(middle->hasFocus());
}

//****************
// entering a container
//****************

// A container answers for what it holds: a tall column is reachable from beside
// any part of it, and lands on whichever of its children is nearest the press
TEST(FocusNavigatorTest, ATallContainerIsReachedFromBesideAnyPartOfIt) {
  FocusNavigator navigator;
  QQuickItem root;
  auto *column = container(&root, 0.0, 0.0, 40.0, 800.0);
  auto *topButton = item(column, 0.0, 0.0, 40.0, 40.0);
  auto *lowButton = item(column, 0.0, 60.0, 40.0, 40.0);
  auto *tile = item(&root, 100.0, 600.0, 200.0, 200.0);

  info(column)->setContainer(true);

  // Neither button is level with the tile, but the column running past it is
  EXPECT_EQ(navigator.move(tile, Qt::Key_Left, false), FocusNavigator::Moved);
  EXPECT_TRUE(lowButton->hasFocus()) << "should be the button nearest the press";
  EXPECT_FALSE(topButton->hasFocus());
}

// Sharing a lane is still required: a column beside the grid is not above it
TEST(FocusNavigatorTest, AContainerSharingNoLaneIsStillRefused) {
  FocusNavigator navigator;
  QQuickItem root;
  auto *column = container(&root, 0.0, 0.0, 40.0, 800.0);
  item(column, 0.0, 0.0, 40.0, 40.0);
  auto *tile = item(&root, 100.0, 600.0, 200.0, 200.0);

  info(column)->setContainer(true);

  EXPECT_EQ(navigator.move(tile, Qt::Key_Up, false), FocusNavigator::NoTarget);
}

// A container that refused a press must not have it answered by re-entering
// that same container. Here the next item down is offset far enough that it
// shares none of the origin's lane, so only re-entry could reach it
TEST(FocusNavigatorTest, APressCannotReEnterTheContainerItCameFrom) {
  FocusNavigator navigator;
  QQuickItem root;
  auto *column = container(&root, 0.0, 0.0, 200.0, 800.0);
  auto *first = item(column, 0.0, 0.0, 40.0, 40.0);
  auto *offset = item(column, 100.0, 60.0, 40.0, 40.0);

  info(column)->setContainer(true);

  EXPECT_EQ(navigator.move(first, Qt::Key_Down, false), FocusNavigator::NoTarget);
  EXPECT_FALSE(offset->hasFocus());
}

// A scrolled view shows whatever rows happen to be on screen, so a press aimed
// at it may line up with none of them. It should still enter, at the nearest row
// that is wholly on display
TEST(FocusNavigatorTest, APressEntersAScrolledViewAtTheNearestRowOnDisplay) {
  FocusNavigator navigator;
  QQmlEngine engine;
  QQmlComponent component(&engine);
  component.setData(R"(
        import QtQuick
        import Firelight 1.0
        Item {
            width: 600; height: 400
            property Item button: theButton
            property Item grid: theGrid

            Item {
                id: theButton
                objectName: "button"
                width: 40; height: 32
                focusPolicy: Qt.StrongFocus
            }

            GridView {
                id: theGrid
                objectName: "grid"
                FLFocus.container: true
                x: 100; width: 400; height: 400
                cellWidth: 200; cellHeight: 200
                model: 40
                delegate: Item {
                    objectName: "tile" + index
                    width: 190; height: 190
                    focusPolicy: Qt.StrongFocus
                }
            }
        }
    )",
                    QUrl());

  std::unique_ptr<QObject> created(component.create());
  ASSERT_NE(created, nullptr) << component.errorString().toStdString();

  auto *button = created->property("button").value<QQuickItem *>();
  auto *grid = created->property("grid").value<QQuickItem *>();
  ASSERT_NE(button, nullptr);
  ASSERT_NE(grid, nullptr);

  // Scrolled so the top row is cut off and the second row is the first wholly shown
  grid->setProperty("contentY", 100.0);
  QMetaObject::invokeMethod(grid, "forceLayout");

  EXPECT_EQ(navigator.move(button, Qt::Key_Right, false), FocusNavigator::Moved);

  auto focused = QString();

  for (auto *item : grid->childItems()) {
    for (auto *delegate : item->childItems()) {
      if (delegate->hasFocus()) {
        focused = delegate->objectName();
      }
    }
  }

  EXPECT_EQ(focused, "tile2") << "should be the leftmost tile of the first wholly visible row";
}

// The rule that stopped a rail falling into the grid beside it still holds: the
// view itself has to lie the way the press points
TEST(FocusNavigatorTest, EnteringDoesNotReachAContainerThePressDoesNotPointAt) {
  FocusNavigator navigator;
  QQmlEngine engine;
  QQmlComponent component(&engine);
  component.setData(R"(
        import QtQuick
        import Firelight 1.0
        Item {
            width: 600; height: 800
            property Item button: theButton

            Item {
                id: theButton
                objectName: "button"
                y: 260
                width: 32; height: 32
                focusPolicy: Qt.StrongFocus
            }

            GridView {
                objectName: "grid"
                FLFocus.container: true
                x: 100; width: 400; height: 700
                cellWidth: 200; cellHeight: 200
                model: 40
                delegate: Item {
                    objectName: "tile" + index
                    width: 190; height: 190
                    focusPolicy: Qt.StrongFocus
                }
            }
        }
    )",
                    QUrl());

  std::unique_ptr<QObject> created(component.create());
  ASSERT_NE(created, nullptr) << component.errorString().toStdString();

  auto *button = created->property("button").value<QQuickItem *>();
  ASSERT_NE(button, nullptr);

  EXPECT_EQ(navigator.move(button, Qt::Key_Down, false), FocusNavigator::NoTarget);
}

TEST(FocusNavigatorTest, ThePressStaysInsideAnOpenPanel) {
  FocusNavigator navigator;
  QQuickItem root;
  auto *panel = container(&root, 0.0, 0.0, 300.0, 300.0);
  auto *inside = item(panel, 0.0, 0.0, 100.0, 40.0);
  auto *outside = item(&root, 0.0, 400.0, 100.0, 40.0);

  info(panel)->setBarrier(true);

  EXPECT_EQ(navigator.move(inside, Qt::Key_Down, false), FocusNavigator::NoTarget);
  EXPECT_FALSE(outside->hasFocus());
}

// The cursor may be drawn around something other than the focused item, and the
// move has to be measured by the same shape
TEST(FocusNavigatorTest, AStandInShapeIsWhatTheMoveIsMeasuredBy) {
  FocusNavigator navigator;
  QQuickItem root;
  auto *button = item(&root, 0.0, 0.0, 10.0, 10.0);
  auto *shape = item(&root, 500.0, 500.0, 100.0, 100.0, Qt::NoFocus);
  auto *below = item(&root, 500.0, 700.0, 100.0, 100.0);

  info(button)->setProxy(shape);

  EXPECT_EQ(navigator.move(button, Qt::Key_Down, false), FocusNavigator::Moved);
  EXPECT_TRUE(below->hasFocus());
}

TEST(FocusNavigatorTest, AHeldDirectionStopsAtAContainersEdgeRatherThanLeavingIt) {
  FocusNavigator navigator;
  QQuickItem root;
  auto *grid = container(&root, 0.0, 100.0, 300.0, 300.0);
  auto *tile = item(grid, 0.0, 0.0, 100.0, 100.0);
  auto *header = item(&root, 0.0, 0.0, 300.0, 60.0);

  info(grid)->setHoldEdges(FocusInfo::Vertical);

  EXPECT_EQ(navigator.move(tile, Qt::Key_Up, true), FocusNavigator::NoTarget);
  EXPECT_FALSE(header->hasFocus());

  EXPECT_EQ(navigator.move(tile, Qt::Key_Up, false), FocusNavigator::Moved);
  EXPECT_TRUE(header->hasFocus());
}

} // namespace firelight::gui

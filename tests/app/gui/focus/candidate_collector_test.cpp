#include "gui/focus/candidate_collector.hpp"

#include "gui/focus_info.hpp"

#include <gtest/gtest.h>

#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickItem>

namespace firelight::gui {

namespace {
// A focusable leaf, since that is what most of these trees are made of
QQuickItem *item(QQuickItem *parent, const qreal x, const qreal y, const qreal width, const qreal height,
                 const Qt::FocusPolicy policy = Qt::StrongFocus) {
  auto *created = new QQuickItem(parent);
  created->setPosition({x, y});
  created->setSize(QSizeF(width, height));
  created->setFocusPolicy(policy);

  return created;
}

// A container that holds things without being one itself
QQuickItem *container(QQuickItem *parent, const qreal x = 0.0, const qreal y = 0.0, const qreal width = 1000.0,
                      const qreal height = 1000.0) {
  return item(parent, x, y, width, height, Qt::NoFocus);
}

FocusInfo *info(QObject *object) {
  return qobject_cast<FocusInfo *>(qmlAttachedPropertiesObject<FocusInfo>(object, true));
}

std::vector<QQuickItem *> itemsIn(const std::vector<FocusCandidate> &candidates) {
  std::vector<QQuickItem *> items;

  for (const auto &candidate : candidates) {
    items.push_back(candidate.item);
  }

  return items;
}
} // namespace

TEST(CandidateCollectorTest, NothingIsFoundBelowNothing) {
  EXPECT_TRUE(CandidateCollector::collect(nullptr).empty());

  QQuickItem root;
  EXPECT_TRUE(CandidateCollector::collect(&root).empty());
}

TEST(CandidateCollectorTest, FindsAFocusableChildAndWhereItIs) {
  QQuickItem root;
  auto *panel = container(&root, 100.0, 50.0);
  auto *button = item(panel, 10.0, 20.0, 72.0, 36.0);

  const auto found = CandidateCollector::collect(&root);

  ASSERT_EQ(found.size(), 1u);
  EXPECT_EQ(found[0].item, button);
  EXPECT_EQ(found[0].rect, QRectF(110.0, 70.0, 72.0, 36.0));
}

TEST(CandidateCollectorTest, SomethingThatCannotTakeFocusIsPassedThroughNotOver) {
  QQuickItem root;
  auto *layout = container(&root);
  auto *button = item(layout, 0.0, 0.0, 72.0, 36.0);

  EXPECT_EQ(itemsIn(CandidateCollector::collect(&root)), std::vector<QQuickItem *>{button});
}

TEST(CandidateCollectorTest, WhatIsInsideATargetIsNotOfferedSeparately) {
  QQuickItem root;
  auto *button = item(&root, 0.0, 0.0, 72.0, 36.0);
  item(button, 0.0, 0.0, 20.0, 20.0);

  EXPECT_EQ(itemsIn(CandidateCollector::collect(&root)), std::vector<QQuickItem *>{button});
}

TEST(CandidateCollectorTest, AHiddenBranchIsNotSearched) {
  QQuickItem root;
  auto *page = container(&root);
  item(page, 0.0, 0.0, 72.0, 36.0);
  page->setVisible(false);

  EXPECT_TRUE(CandidateCollector::collect(&root).empty());
}

TEST(CandidateCollectorTest, ADisabledBranchIsNotSearched) {
  QQuickItem root;
  auto *page = container(&root);
  item(page, 0.0, 0.0, 72.0, 36.0);
  page->setEnabled(false);

  EXPECT_TRUE(CandidateCollector::collect(&root).empty());
}

TEST(CandidateCollectorTest, AFullyFadedBranchIsNotSearched) {
  QQuickItem root;
  auto *page = container(&root);
  item(page, 0.0, 0.0, 72.0, 36.0);
  page->setOpacity(0.0);

  EXPECT_TRUE(CandidateCollector::collect(&root).empty());

  page->setOpacity(0.01);
  EXPECT_EQ(CandidateCollector::collect(&root).size(), 1u);
}

// Items do not grow to fit their children in QML, so a container with no size of its own is
// ordinary rather than absent
TEST(CandidateCollectorTest, AContainerWithNoSizeStillHoldsItsChildren) {
  QQuickItem root;
  auto *wrapper = item(&root, 40.0, 40.0, 0.0, 0.0, Qt::NoFocus);
  auto *button = item(wrapper, 0.0, 0.0, 72.0, 36.0);

  const auto found = CandidateCollector::collect(&root);

  ASSERT_EQ(found.size(), 1u);
  EXPECT_EQ(found[0].item, button);
}

TEST(CandidateCollectorTest, SomethingWithNoSizeIsNotATarget) {
  QQuickItem root;
  item(&root, 0.0, 0.0, 0.0, 0.0);

  EXPECT_TRUE(CandidateCollector::collect(&root).empty());
}

// A tile wraps its button in a focus scope; the cursor should find the button
TEST(CandidateCollectorTest, AFocusScopeOffersWhatIsInsideItInstead) {
  QQuickItem root;
  auto *scope = item(&root, 0.0, 0.0, 200.0, 200.0);
  scope->setFlag(QQuickItem::ItemIsFocusScope);
  auto *button = item(scope, 10.0, 10.0, 72.0, 36.0);

  EXPECT_EQ(itemsIn(CandidateCollector::collect(&root)), std::vector<QQuickItem *>{button});
}

TEST(CandidateCollectorTest, AFocusScopeCanAskToBeTheTargetItself) {
  QQuickItem root;
  auto *scope = item(&root, 0.0, 0.0, 200.0, 200.0);
  scope->setFlag(QQuickItem::ItemIsFocusScope);
  item(scope, 10.0, 10.0, 72.0, 36.0);

  info(scope)->setMode(FocusInfo::Stop);

  EXPECT_EQ(itemsIn(CandidateCollector::collect(&root)), std::vector<QQuickItem *>{scope});
}

TEST(CandidateCollectorTest, GroupOffersWhatIsInsideSomethingFocusable) {
  QQuickItem root;
  auto *group = item(&root, 0.0, 0.0, 200.0, 200.0);
  auto *first = item(group, 0.0, 0.0, 200.0, 40.0);
  auto *second = item(group, 0.0, 50.0, 200.0, 40.0);

  EXPECT_EQ(itemsIn(CandidateCollector::collect(&root)), std::vector<QQuickItem *>{group});

  info(group)->setMode(FocusInfo::Group);

  EXPECT_EQ(itemsIn(CandidateCollector::collect(&root)), (std::vector<QQuickItem *>{first, second}));
}

TEST(CandidateCollectorTest, StopKeepsWhatIsInsideOutOfReach) {
  QQuickItem root;
  auto *panel = container(&root);
  auto *outer = item(panel, 0.0, 0.0, 200.0, 200.0);
  item(outer, 0.0, 0.0, 50.0, 50.0);

  info(outer)->setMode(FocusInfo::Stop);

  EXPECT_EQ(itemsIn(CandidateCollector::collect(&root)), std::vector<QQuickItem *>{outer});
}

TEST(CandidateCollectorTest, StopOnSomethingUnfocusableLeavesNothingReachable) {
  QQuickItem root;
  auto *wrapper = container(&root);
  item(wrapper, 0.0, 0.0, 72.0, 36.0);

  info(wrapper)->setMode(FocusInfo::Stop);

  EXPECT_TRUE(CandidateCollector::collect(&root).empty());
}

TEST(CandidateCollectorTest, SkipRemovesTheWholeBranch) {
  QQuickItem root;
  auto *hidden = container(&root);
  item(hidden, 0.0, 0.0, 72.0, 36.0);
  auto *shown = item(&root, 0.0, 100.0, 72.0, 36.0);

  info(hidden)->setMode(FocusInfo::Skip);

  EXPECT_EQ(itemsIn(CandidateCollector::collect(&root)), std::vector<QQuickItem *>{shown});
}

TEST(CandidateCollectorTest, SkipOnATargetRemovesItToo) {
  QQuickItem root;
  auto *button = item(&root, 0.0, 0.0, 72.0, 36.0);
  info(button)->setMode(FocusInfo::Skip);

  EXPECT_TRUE(CandidateCollector::collect(&root).empty());
}

TEST(CandidateCollectorTest, WhatAClippingParentCutsOffIsOutOfReach) {
  QQuickItem root;
  auto *frame = container(&root, 0.0, 0.0, 200.0, 200.0);
  frame->setClip(true);

  auto *inside = item(frame, 10.0, 10.0, 72.0, 36.0);
  item(frame, 10.0, 400.0, 72.0, 36.0);

  EXPECT_EQ(itemsIn(CandidateCollector::collect(&root)), std::vector<QQuickItem *>{inside});
}

TEST(CandidateCollectorTest, ClippingNarrowsRatherThanReplaces) {
  QQuickItem root;
  auto *outer = container(&root, 0.0, 0.0, 200.0, 200.0);
  outer->setClip(true);

  auto *inner = container(outer, 0.0, 0.0, 400.0, 400.0);
  inner->setClip(true);

  auto *inside = item(inner, 10.0, 10.0, 72.0, 36.0);
  item(inner, 10.0, 300.0, 72.0, 36.0);

  EXPECT_EQ(itemsIn(CandidateCollector::collect(&root)), std::vector<QQuickItem *>{inside});
}

// The root is where the search starts, so its own clipping still applies
TEST(CandidateCollectorTest, TheRootsOwnClippingApplies) {
  QQuickItem root;
  root.setSize(QSizeF(200.0, 200.0));
  root.setClip(true);

  auto *inside = item(&root, 0.0, 0.0, 72.0, 36.0);
  item(&root, 0.0, 400.0, 72.0, 36.0);

  EXPECT_EQ(itemsIn(CandidateCollector::collect(&root)), std::vector<QQuickItem *>{inside});
}

// A scrolling view clips, but anything in it can be brought into view, so its
// bounds must not decide what is reachable
TEST(CandidateCollectorTest, WhatAScrollingViewHasScrolledPastIsStillReachable) {
  QQmlEngine engine;
  QQmlComponent component(&engine);
  component.setData(R"(
        import QtQuick
        Flickable {
            width: 200; height: 200; clip: true
            Item { objectName: "visible"; width: 72; height: 36; focusPolicy: Qt.StrongFocus }
            Item { objectName: "scrolled"; y: 4000; width: 72; height: 36; focusPolicy: Qt.StrongFocus }
        }
    )",
                    QUrl());

  std::unique_ptr<QObject> created(component.create());
  ASSERT_NE(created, nullptr) << component.errorString().toStdString();

  auto *view = qobject_cast<QQuickItem *>(created.get());
  ASSERT_NE(view, nullptr);
  ASSERT_TRUE(view->inherits("QQuickFlickable"));

  const auto found = CandidateCollector::collect(view);

  ASSERT_EQ(found.size(), 2u);
  EXPECT_EQ(found[0].item->objectName(), "visible");
  EXPECT_EQ(found[1].item->objectName(), "scrolled");
}

// A view keeps delegates alive outside what it shows — recycled, buffered, or just the current one
// left behind — so what it shows is the limit of what can be moved to
TEST(CandidateCollectorTest, WhatAnItemViewHoldsButDoesNotShowIsOutOfReach) {
  QQmlEngine engine;
  QQmlComponent component(&engine);
  component.setData(R"(
        import QtQuick
        GridView {
            width: 400; height: 400
            cellWidth: 200; cellHeight: 200
            model: 200
            reuseItems: true
            currentIndex: 0
            cacheBuffer: 400
            displayMarginBeginning: 100
            delegate: Item { width: 190; height: 190; focusPolicy: Qt.StrongFocus }
        }
    )",
                    QUrl());

  std::unique_ptr<QObject> created(component.create());
  ASSERT_NE(created, nullptr) << component.errorString().toStdString();

  auto *view = qobject_cast<QQuickItem *>(created.get());
  ASSERT_NE(view, nullptr);

  QMetaObject::invokeMethod(view, "forceLayout");
  ASSERT_FALSE(CandidateCollector::collect(view).empty()) << "the view realized nothing to begin with";

  view->setProperty("contentY", 4000.0);
  QMetaObject::invokeMethod(view, "forceLayout");

  const auto viewport = view->mapRectToScene(QRectF(0.0, 0.0, view->width(), view->height()));
  const auto found = CandidateCollector::collect(view);

  ASSERT_FALSE(found.empty());

  for (const auto &candidate : found) {
    EXPECT_TRUE(viewport.contains(candidate.rect))
        << "offered something at " << candidate.rect.x() << "," << candidate.rect.y() << " the view does not fully show";
  }
}

// Landing on a row that is only half on screen drags the whole container along to
// finish showing it, which is not what moving into it should do
TEST(CandidateCollectorTest, SomethingOnlyPartlyOnDisplayIsNotOfferedTo) {
  QQuickItem root;
  auto *frame = container(&root, 0.0, 0.0, 200.0, 200.0);
  frame->setClip(true);

  auto *whole = item(frame, 10.0, 10.0, 72.0, 36.0);
  auto *half = item(frame, 10.0, 180.0, 72.0, 36.0);

  EXPECT_EQ(itemsIn(CandidateCollector::collect(&root)), std::vector<QQuickItem *>{whole});

  // Once it fits, it is offered like anything else
  half->setY(150.0);
  EXPECT_EQ(CandidateCollector::collect(&root).size(), 2u);
}

// An item exactly filling what holds it is on display, whatever the arithmetic
// left behind
TEST(CandidateCollectorTest, SomethingFillingItsContainerExactlyIsOnDisplay) {
  QQuickItem root;
  auto *frame = container(&root, 0.0, 0.0, 200.0, 200.0);
  frame->setClip(true);

  auto *exact = item(frame, 0.0, 0.0, 200.0, 200.0);

  EXPECT_EQ(itemsIn(CandidateCollector::collect(&root)), std::vector<QQuickItem *>{exact});
}

// Exact geometric ties are broken by walk order, so it has to be the order
// things were declared in
TEST(CandidateCollectorTest, WalkOrderFollowsTheTree) {
  QQuickItem root;
  auto *first = item(&root, 0.0, 0.0, 10.0, 10.0);
  auto *branch = container(&root);
  auto *second = item(branch, 0.0, 0.0, 10.0, 10.0);
  auto *third = item(branch, 0.0, 20.0, 10.0, 10.0);
  auto *fourth = item(&root, 0.0, 40.0, 10.0, 10.0);

  EXPECT_EQ(itemsIn(CandidateCollector::collect(&root)), (std::vector<QQuickItem *>{first, second, third, fourth}));
}

TEST(CandidateCollectorTest, WithNoBarrierTheSearchFallsBackToTheWholeScreen) {
  QQuickItem fallback;
  QQuickItem root;
  auto *button = item(&root, 0.0, 0.0, 72.0, 36.0);

  EXPECT_EQ(CandidateCollector::scopeFor(button, &fallback), &fallback);
  EXPECT_EQ(CandidateCollector::scopeFor(nullptr, &fallback), &fallback);
}

TEST(CandidateCollectorTest, TheSearchStopsAtTheNearestBarrier) {
  QQuickItem fallback;
  QQuickItem root;
  auto *popup = container(&root);
  auto *inner = container(popup);
  auto *button = item(inner, 0.0, 0.0, 72.0, 36.0);

  info(popup)->setBarrier(true);
  EXPECT_EQ(CandidateCollector::scopeFor(button, &fallback), popup);

  info(inner)->setBarrier(true);
  EXPECT_EQ(CandidateCollector::scopeFor(button, &fallback), inner);
}

TEST(CandidateCollectorTest, SomethingIsItsOwnBarrier) {
  QQuickItem fallback;
  QQuickItem root;
  auto *popup = container(&root);

  info(popup)->setBarrier(true);

  EXPECT_EQ(CandidateCollector::scopeFor(popup, &fallback), popup);
}

// Key focus can sit deeper than the thing the cursor is drawn around, so the two
// have to agree on where "here" is
TEST(CandidateCollectorTest, WhereTheCursorIsIsTheNearestTargetAtOrAboveTheFocusedItem) {
  QQuickItem root;
  auto *button = item(&root, 0.0, 0.0, 72.0, 36.0);
  auto *label = item(button, 0.0, 0.0, 40.0, 20.0, Qt::NoFocus);
  auto *glyph = item(label, 0.0, 0.0, 10.0, 10.0, Qt::NoFocus);

  EXPECT_EQ(CandidateCollector::candidateFor(glyph), button);
  EXPECT_EQ(CandidateCollector::candidateFor(button), button);
}

TEST(CandidateCollectorTest, AFocusScopeIsNotWhereTheCursorIs) {
  QQuickItem root;
  auto *scope = item(&root, 0.0, 0.0, 200.0, 200.0);
  scope->setFlag(QQuickItem::ItemIsFocusScope);
  auto *button = item(scope, 0.0, 0.0, 72.0, 36.0);

  EXPECT_EQ(CandidateCollector::candidateFor(button), button);
  EXPECT_EQ(CandidateCollector::candidateFor(scope), nullptr);
}

TEST(CandidateCollectorTest, NothingAboveASkippedItemCountsAsWhereTheCursorIs) {
  QQuickItem root;
  auto *button = item(&root, 0.0, 0.0, 72.0, 36.0);
  auto *label = item(button, 0.0, 0.0, 40.0, 20.0, Qt::NoFocus);

  info(button)->setMode(FocusInfo::Skip);

  EXPECT_EQ(CandidateCollector::candidateFor(label), nullptr);
  EXPECT_EQ(CandidateCollector::candidateFor(nullptr), nullptr);
}

} // namespace firelight::gui

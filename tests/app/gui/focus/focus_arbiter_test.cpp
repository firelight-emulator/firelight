// TODO: NEEDS REVIEW
#include "gui/focus/focus_arbiter.hpp"

#include "gui/focus_info.hpp"

#include <QCoreApplication>
#include <QQuickItem>
#include <QQuickWindow>
#include <gtest/gtest.h>

namespace firelight::gui {

namespace {
QQuickItem *item(QQuickItem *parent, const Qt::FocusPolicy policy = Qt::StrongFocus) {
  auto *created = new QQuickItem(parent);
  created->setSize(QSizeF(100.0, 40.0));
  created->setFocusPolicy(policy);

  return created;
}

FocusInfo *info(QObject *object) {
  return qobject_cast<FocusInfo *>(qmlAttachedPropertiesObject<FocusInfo>(object, true));
}

/**
 * A surface with one focusable leaf inside it, which is also its entry
 */
struct Surface {
  QQuickItem *root = nullptr;
  QQuickItem *leaf = nullptr;
  FocusInfo *declaration = nullptr;
};

Surface surface(QQuickItem *parent, const int layer, const bool isActive = true) {
  Surface made;
  made.root = item(parent, Qt::NoFocus);
  made.leaf = item(made.root);
  made.declaration = info(made.root);
  made.declaration->setLayer(layer);
  made.declaration->setSurfaceActive(isActive);
  made.declaration->setEntry(made.leaf);
  made.declaration->setSurface(true);

  return made;
}

/**
 * An arbiter whose landings plainly focus the item and are counted
 */
struct Harness {
  QQuickWindow window;
  FocusArbiter arbiter;
  int landings = 0;

  // TODO
  // An active window's root holds focus; a window that was never shown has to be given it
  Harness() {
    window.contentItem()->setFocus(true);
    arbiter.setLander([this](QQuickItem *to) {
      ++landings;
      to->forceActiveFocus();
    });
  }

  void pass() { arbiter.enforceNow(&window); }

  QQuickItem *root() { return window.contentItem(); }
};
} // namespace

//****************
// ownership
//****************

TEST(FocusArbiterTest, TheHighestActiveLayerOwns) {
  Harness harness;
  const auto low = surface(harness.root(), 0);
  const auto high = surface(harness.root(), 10);

  EXPECT_EQ(harness.arbiter.getOwner(&harness.window), high.root);
  EXPECT_EQ(harness.arbiter.getOwnerInfo(&harness.window), high.declaration);
  (void)low;
}

TEST(FocusArbiterTest, AnInactiveSurfaceNeverOwns) {
  Harness harness;
  const auto low = surface(harness.root(), 0);
  const auto high = surface(harness.root(), 10, false);

  EXPECT_EQ(harness.arbiter.getOwner(&harness.window), low.root);
  (void)high;
}

TEST(FocusArbiterTest, OnATieTheLatestActivationOwns) {
  Harness harness;
  const auto first = surface(harness.root(), 30);
  const auto second = surface(harness.root(), 30);

  EXPECT_EQ(harness.arbiter.getOwner(&harness.window), second.root);

  first.declaration->setSurfaceActive(false);
  first.declaration->setSurfaceActive(true);

  EXPECT_EQ(harness.arbiter.getOwner(&harness.window), first.root);

  // Staying active is not an activation
  first.declaration->setSurfaceActive(true);
  second.declaration->setSurfaceActive(false);
  second.declaration->setSurfaceActive(true);

  EXPECT_EQ(harness.arbiter.getOwner(&harness.window), second.root);
}

TEST(FocusArbiterTest, EachWindowHasItsOwnOwner) {
  Harness harness;
  QQuickWindow other;
  const auto here = surface(harness.root(), 0);
  const auto there = surface(other.contentItem(), 50);

  EXPECT_EQ(harness.arbiter.getOwner(&harness.window), here.root);
  EXPECT_EQ(harness.arbiter.getOwner(&other), there.root);
}

TEST(FocusArbiterTest, ADestroyedSurfaceStopsOwning) {
  Harness harness;
  const auto low = surface(harness.root(), 0);
  auto high = surface(harness.root(), 10);

  delete high.root;

  EXPECT_EQ(harness.arbiter.getOwner(&harness.window), low.root);
}

//****************
// enforcement
//****************

TEST(FocusArbiterTest, FocusOutsideTheOwnerIsLandedOnTheEntry) {
  Harness harness;
  const auto owner = surface(harness.root(), 10);
  auto *stray = item(harness.root());
  stray->forceActiveFocus();

  harness.pass();

  EXPECT_EQ(harness.window.activeFocusItem(), owner.leaf);
  EXPECT_EQ(harness.landings, 1);
}

TEST(FocusArbiterTest, FocusInsideTheOwnerIsLeftAlone) {
  Harness harness;
  const auto owner = surface(harness.root(), 10);
  auto *sibling = item(owner.root);
  sibling->forceActiveFocus();

  harness.pass();

  EXPECT_EQ(harness.window.activeFocusItem(), sibling);
  EXPECT_EQ(harness.landings, 0);
}

TEST(FocusArbiterTest, FocusRestingOnANonTargetInsideTheOwnerIsLandedOnTheEntry) {
  Harness harness;
  const auto owner = surface(harness.root(), 30);
  auto *scope = item(owner.root, Qt::NoFocus);
  scope->setFlag(QQuickItem::ItemIsFocusScope);
  scope->forceActiveFocus();

  ASSERT_EQ(harness.window.activeFocusItem(), scope);

  harness.pass();

  EXPECT_EQ(harness.window.activeFocusItem(), owner.leaf);
  EXPECT_EQ(harness.landings, 1);
}

TEST(FocusArbiterTest, NoFocusAtAllIsLanded) {
  Harness harness;
  const auto owner = surface(harness.root(), 10);

  harness.pass();

  EXPECT_EQ(harness.window.activeFocusItem(), owner.leaf);
}

TEST(FocusArbiterTest, FocusOnAHiddenItemIsLanded) {
  Harness harness;
  const auto owner = surface(harness.root(), 10);
  auto *hidden = item(owner.root);
  hidden->forceActiveFocus();
  hidden->setVisible(false);

  harness.pass();

  EXPECT_EQ(harness.window.activeFocusItem(), owner.leaf);
}

TEST(FocusArbiterTest, FocusInsideAnExclusiveRegionMustBeInsideTheEntry) {
  Harness harness;
  const auto owner = surface(harness.root(), 0);
  auto *stack = item(owner.root, Qt::NoFocus);
  auto *page = item(stack);
  auto *outgoingPage = item(stack);
  owner.declaration->setEntry(page);
  auto exclusive = owner.declaration->getExclusive();
  exclusive.append(&exclusive, stack);

  outgoingPage->forceActiveFocus();
  harness.pass();

  EXPECT_EQ(harness.window.activeFocusItem(), page);

  auto *titleBarButton = item(owner.root);
  titleBarButton->forceActiveFocus();
  harness.pass();

  EXPECT_EQ(harness.window.activeFocusItem(), titleBarButton);
}

TEST(FocusArbiterTest, AnEntryChangeLandsOnTheNewEntryEvenFromInsideTheOwner) {
  Harness harness;
  const auto owner = surface(harness.root(), 0);
  auto *elsewhere = item(owner.root);
  auto *newEntry = item(owner.root);
  elsewhere->forceActiveFocus();

  owner.declaration->setEntry(newEntry);
  QCoreApplication::processEvents();

  EXPECT_EQ(harness.window.activeFocusItem(), newEntry);

  // Focus already inside the new entry stays where it is
  auto *inside = item(newEntry);
  inside->forceActiveFocus();
  owner.declaration->setEntry(nullptr);
  owner.declaration->setEntry(newEntry);
  QCoreApplication::processEvents();

  EXPECT_EQ(harness.window.activeFocusItem(), inside);
}

TEST(FocusArbiterTest, ABurstOfChangesLandsOnce) {
  Harness harness;
  const auto owner = surface(harness.root(), 10);
  auto *strayA = item(harness.root());
  auto *strayB = item(harness.root());
  auto *strayC = item(harness.root());

  QObject::connect(&harness.window, &QQuickWindow::activeFocusItemChanged, &harness.arbiter,
                   [&harness] { harness.arbiter.request(&harness.window); });

  strayA->forceActiveFocus();
  strayB->forceActiveFocus();
  strayC->forceActiveFocus();

  EXPECT_EQ(harness.landings, 0);

  QCoreApplication::processEvents();
  QCoreApplication::processEvents();

  EXPECT_EQ(harness.window.activeFocusItem(), owner.leaf);
  EXPECT_EQ(harness.landings, 1);
}

TEST(FocusArbiterTest, AWindowWithoutKeyboardFocusIsLeftAlone) {
  Harness harness;
  const auto owner = surface(harness.root(), 10);
  auto *stray = item(harness.root());
  stray->forceActiveFocus();
  harness.root()->setFocus(false);

  harness.pass();

  EXPECT_EQ(harness.landings, 0);

  harness.root()->setFocus(true);
  harness.pass();

  EXPECT_EQ(harness.window.activeFocusItem(), owner.leaf);
}

TEST(FocusArbiterTest, AHiddenOwnerIsRetriedWhenItShows) {
  Harness harness;
  const auto owner = surface(harness.root(), 10);
  owner.root->setVisible(false);
  auto *stray = item(harness.root());
  stray->forceActiveFocus();

  harness.pass();

  EXPECT_EQ(harness.window.activeFocusItem(), stray);

  owner.root->setVisible(true);
  QCoreApplication::processEvents();

  EXPECT_EQ(harness.window.activeFocusItem(), owner.leaf);
}

TEST(FocusArbiterTest, AnOwnerChangeIsAnnouncedOncePerWindow) {
  Harness harness;
  const auto low = surface(harness.root(), 0);
  const auto high = surface(harness.root(), 10, false);
  int announcements = 0;
  QObject::connect(&harness.arbiter, &FocusArbiter::ownerChanged, &harness.arbiter,
                   [&announcements] { ++announcements; });

  harness.pass();
  harness.pass();
  EXPECT_EQ(announcements, 1);

  high.declaration->setSurfaceActive(true);
  harness.pass();
  EXPECT_EQ(announcements, 2);
  EXPECT_EQ(harness.arbiter.getOwner(&harness.window), high.root);
  (void)low;
}

} // namespace firelight::gui

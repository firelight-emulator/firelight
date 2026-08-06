#include "gui/focus_info.hpp"

#include <QObject>
#include <QQmlComponent>
#include <QQmlEngine>
#include <gtest/gtest.h>
#include <memory>

namespace firelight::gui {

namespace {
// Attaches metadata the way QML does on first use, so a test can set it up
FocusInfo *attach(QObject *object) {
  return qobject_cast<FocusInfo *>(qmlAttachedPropertiesObject<FocusInfo>(object, true));
}

FocusAction *addAction(FocusInfo *info, const QList<int> &keys, const bool enabled = true) {
  auto *action = new FocusAction(info);
  action->setKeys(keys);
  action->setEnabled(enabled);

  auto list = info->getActions();
  list.append(&list, action);

  return action;
}
} // namespace

TEST(FocusInfoTest, FindReturnsNullBeforeAnythingIsDeclared) {
  QObject object;
  EXPECT_EQ(FocusInfo::find(&object), nullptr);
}

TEST(FocusInfoTest, FindReturnsNullForNull) { EXPECT_EQ(FocusInfo::find(nullptr), nullptr); }

TEST(FocusInfoTest, FindReturnsTheSameInstanceOnceAttached) {
  QObject object;
  auto *attached = attach(&object);

  ASSERT_NE(attached, nullptr);
  EXPECT_EQ(FocusInfo::find(&object), attached);
}

// The cursor is opt-in, so an item that says nothing gets no ring
TEST(FocusInfoTest, DefaultsToNoCursor) {
  QObject object;
  const auto *info = attach(&object);

  EXPECT_FALSE(info->showsCursor());
  EXPECT_EQ(info->getProxy(), nullptr);
  EXPECT_EQ(info->getFocusSound(), nullptr);
  EXPECT_EQ(info->getMode(), FocusInfo::Normal);
  EXPECT_FALSE(info->isBarrier());
  EXPECT_EQ(info->getHoldEdges(), FocusInfo::NoEdges);
}

// Unset has to be distinguishable from any real value, since spacing is
// legitimately negative
TEST(FocusInfoTest, SpacingAndRadiusStartUnset) {
  QObject object;
  const auto *info = attach(&object);

  EXPECT_TRUE(qIsNaN(info->getSpacing()));
  EXPECT_TRUE(qIsNaN(info->getRadius()));
}

TEST(FocusInfoTest, SpacingAcceptsNegativeValues) {
  QObject object;
  auto *info = attach(&object);
  info->setSpacing(-8.0);

  EXPECT_FALSE(qIsNaN(info->getSpacing()));
  EXPECT_DOUBLE_EQ(info->getSpacing(), -8.0);
}

TEST(FocusInfoTest, HoldEdgesCombine) {
  QObject object;
  auto *info = attach(&object);
  info->setHoldEdges(FocusInfo::Vertical);

  EXPECT_TRUE(info->getHoldEdges().testFlag(FocusInfo::Up));
  EXPECT_TRUE(info->getHoldEdges().testFlag(FocusInfo::Down));
  EXPECT_FALSE(info->getHoldEdges().testFlag(FocusInfo::Left));
  EXPECT_FALSE(info->getHoldEdges().testFlag(FocusInfo::Right));
}

TEST(FocusInfoTest, NoActionsByDefault) {
  QObject object;
  const auto *info = attach(&object);

  EXPECT_EQ(info->getActionCount(), 0);
  EXPECT_EQ(info->getActionFor(Qt::Key_Select), nullptr);
}

TEST(FocusInfoTest, FindsAnActionByAnyOfItsKeys) {
  QObject object;
  auto *info = attach(&object);
  auto *play = addAction(info, {Qt::Key_Select, Qt::Key_Return, Qt::Key_Enter});

  EXPECT_EQ(info->getActionFor(Qt::Key_Select), play);
  EXPECT_EQ(info->getActionFor(Qt::Key_Return), play);
  EXPECT_EQ(info->getActionFor(Qt::Key_Enter), play);
  EXPECT_EQ(info->getActionFor(Qt::Key_Menu), nullptr);
}

// The point of the whole type: one item, two buttons, two outcomes
TEST(FocusInfoTest, SeparateKeysReachSeparateActions) {
  QObject object;
  auto *info = attach(&object);
  auto *play = addAction(info, {Qt::Key_Select});
  auto *options = addAction(info, {Qt::Key_Menu});

  EXPECT_EQ(info->getActionCount(), 2);
  EXPECT_EQ(info->getActionFor(Qt::Key_Select), play);
  EXPECT_EQ(info->getActionFor(Qt::Key_Menu), options);
}

TEST(FocusInfoTest, DisabledActionsAreSkipped) {
  QObject object;
  auto *info = attach(&object);
  addAction(info, {Qt::Key_Select}, false);

  EXPECT_EQ(info->getActionFor(Qt::Key_Select), nullptr);
  EXPECT_EQ(info->getActionCount(), 1);
}

TEST(FocusInfoTest, DisabledActionDoesNotShadowALaterEnabledOne) {
  QObject object;
  auto *info = attach(&object);
  addAction(info, {Qt::Key_Select}, false);
  auto *enabled = addAction(info, {Qt::Key_Select});

  EXPECT_EQ(info->getActionFor(Qt::Key_Select), enabled);
}

TEST(FocusInfoTest, FirstDeclaredActionWinsWhenKeysOverlap) {
  QObject object;
  auto *info = attach(&object);
  auto *first = addAction(info, {Qt::Key_Select});
  addAction(info, {Qt::Key_Select});

  EXPECT_EQ(info->getActionFor(Qt::Key_Select), first);
}

TEST(FocusInfoTest, ActionsAreReadableByIndexInDeclarationOrder) {
  QObject object;
  auto *info = attach(&object);
  auto *first = addAction(info, {Qt::Key_Select});
  auto *second = addAction(info, {Qt::Key_Menu});

  EXPECT_EQ(info->getAction(0), first);
  EXPECT_EQ(info->getAction(1), second);
  EXPECT_EQ(info->getAction(2), nullptr);
  EXPECT_EQ(info->getAction(-1), nullptr);
}

// The dispatcher fires this; nothing works if a QML handler cannot hear it
TEST(FocusInfoTest, TriggeringAnActionNotifies) {
  QObject object;
  auto *info = attach(&object);
  auto *action = addAction(info, {Qt::Key_Select});

  auto fired = 0;
  QObject::connect(action, &FocusAction::triggered, [&fired] { fired++; });

  info->getActionFor(Qt::Key_Select)->triggered();

  EXPECT_EQ(fired, 1);
}

// A popup declares its own barrier on content it was handed rather than
// declared, which only works if writing the attachment from script does
TEST(FocusInfoTest, MetadataCanBeDeclaredFromScript) {
  QQmlEngine engine;
  QQmlComponent component(&engine);
  component.setData(R"(
        import QtQuick
        import Firelight 1.0
        Item {
            id: root
            property Item content: Item {}
            Component.onCompleted: {
                content.FLFocus.barrier = true;
                content.FLFocus.holdEdges = FLFocus.Vertical;
            }
        }
    )",
                    QUrl());

  std::unique_ptr<QObject> created(component.create());
  ASSERT_NE(created, nullptr) << component.errorString().toStdString();

  auto *content = created->property("content").value<QObject *>();
  ASSERT_NE(content, nullptr);

  auto *info = FocusInfo::find(content);
  ASSERT_NE(info, nullptr);
  EXPECT_TRUE(info->isBarrier());
  EXPECT_EQ(info->getHoldEdges(), FocusInfo::Vertical);
}

TEST(FocusInfoTest, MetadataDiesWithTheObjectItIsAttachedTo) {
  QPointer<FocusInfo> info;

  {
    QObject object;
    info = attach(&object);
    ASSERT_FALSE(info.isNull());
  }

  EXPECT_TRUE(info.isNull());
}

} // namespace firelight::gui

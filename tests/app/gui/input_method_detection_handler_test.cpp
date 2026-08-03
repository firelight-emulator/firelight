#include "gui/eventhandlers/input_method_detection_handler.hpp"

#include <gtest/gtest.h>

#include <QCoreApplication>
#include <QKeyEvent>
#include <QObject>

namespace firelight::gui {

namespace {
// Delivers a key through the filter the way the window does, before any item
// would handle it
void sendKey(QObject &target, const QEvent::Type type, const bool autoRepeat) {
  QKeyEvent event(type, Qt::Key_Down, Qt::NoModifier, QString(), autoRepeat);
  QCoreApplication::sendEvent(&target, &event);
}

void sendMouseMove(QObject &target) {
  QMouseEvent event(QEvent::MouseMove, QPointF(), QPointF(), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
  QCoreApplication::sendEvent(&target, &event);
}

// Counts the keys that made it past the filter to the object itself
struct RecordingObject : QObject {
  int keyPresses = 0;

  bool event(QEvent *incoming) override {
    if (incoming->type() == QEvent::KeyPress) {
      keyPresses++;
    }

    return QObject::event(incoming);
  }
};

// A window with the handler filtering it, as main.cpp sets up
struct Filtered {
  RecordingObject window;
  InputMethodDetectionHandler handler;

  Filtered() { window.installEventFilter(&handler); }
};
} // namespace

TEST(InputMethodDetectionHandlerTest, StartsNotRepeating) {
  const Filtered filtered;
  EXPECT_FALSE(filtered.handler.isKeyRepeating());
}

TEST(InputMethodDetectionHandlerTest, AFreshPressIsNotRepeating) {
  Filtered filtered;
  sendKey(filtered.window, QEvent::KeyPress, false);

  EXPECT_FALSE(filtered.handler.isKeyRepeating());
}

TEST(InputMethodDetectionHandlerTest, ARepeatedPressIsRepeating) {
  Filtered filtered;
  sendKey(filtered.window, QEvent::KeyPress, false);
  sendKey(filtered.window, QEvent::KeyPress, true);

  EXPECT_TRUE(filtered.handler.isKeyRepeating());
}

// Ending the hold has to clear it, or every later press reads as a repeat
TEST(InputMethodDetectionHandlerTest, ReleasingTheHoldStopsRepeating) {
  Filtered filtered;
  sendKey(filtered.window, QEvent::KeyPress, true);
  ASSERT_TRUE(filtered.handler.isKeyRepeating());

  sendKey(filtered.window, QEvent::KeyRelease, false);

  EXPECT_FALSE(filtered.handler.isKeyRepeating());
}

// A held key delivers its own releases between repeats; those are not the end
TEST(InputMethodDetectionHandlerTest, ARepeatedReleaseDoesNotEndTheHold) {
  Filtered filtered;
  sendKey(filtered.window, QEvent::KeyPress, true);
  sendKey(filtered.window, QEvent::KeyRelease, true);

  EXPECT_TRUE(filtered.handler.isKeyRepeating());
}

TEST(InputMethodDetectionHandlerTest, AFreshPressAfterAHoldIsNotRepeating) {
  Filtered filtered;
  sendKey(filtered.window, QEvent::KeyPress, true);
  sendKey(filtered.window, QEvent::KeyPress, false);

  EXPECT_FALSE(filtered.handler.isKeyRepeating());
}

TEST(InputMethodDetectionHandlerTest, RepeatingChangesAreAnnouncedOnce) {
  Filtered filtered;
  auto changes = 0;
  QObject::connect(&filtered.handler, &InputMethodDetectionHandler::keyRepeatingChanged,
                   [&changes] { changes++; });

  sendKey(filtered.window, QEvent::KeyPress, true);
  sendKey(filtered.window, QEvent::KeyPress, true);

  EXPECT_EQ(changes, 1);
}

// Key input still means the mouse stopped driving, repeat or not
TEST(InputMethodDetectionHandlerTest, KeyInputStillClearsUsingMouse) {
  Filtered filtered;
  sendMouseMove(filtered.window);
  ASSERT_TRUE(filtered.handler.usingMouse());

  sendKey(filtered.window, QEvent::KeyPress, true);

  EXPECT_FALSE(filtered.handler.usingMouse());
}

// Observing must not consume; the focused item still has to receive the key
TEST(InputMethodDetectionHandlerTest, KeysStillReachTheirTarget) {
  Filtered filtered;
  sendKey(filtered.window, QEvent::KeyPress, false);
  sendKey(filtered.window, QEvent::KeyPress, true);

  EXPECT_EQ(filtered.window.keyPresses, 2);
}

} // namespace firelight::gui

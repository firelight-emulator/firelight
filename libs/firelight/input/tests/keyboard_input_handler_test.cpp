#include <firelight/event_dispatcher.hpp>
#include <firelight/input/gamepad_key_event.hpp>
#include <firelight/input/input_service.hpp>
#include <firelight/input/keyboard_input_handler.hpp>

#include <QCoreApplication>
#include <QKeyEvent>
#include <gtest/gtest.h>
#include <vector>

namespace firelight::input {

namespace {
// Keeps every keyboard event published while it is alive
class Recorder {
public:
  Recorder() {
    m_connection = EventDispatcher::instance().subscribe<KeyboardKeyEvent>(
        [this](const KeyboardKeyEvent &event) { m_events.push_back(event); });
  }

  [[nodiscard]] const std::vector<KeyboardKeyEvent> &getEvents() const { return m_events; }

private:
  ScopedConnection m_connection;
  std::vector<KeyboardKeyEvent> m_events;
};

// Sends event the way the window does, through the filter the handler installs on it
void deliver(KeyboardInputHandler &handler, QEvent &event) {
  QObject target;
  target.installEventFilter(&handler);
  QCoreApplication::sendEvent(&target, &event);
}
} // namespace

TEST(GamepadKeyEventTest, TellsItselfApartFromATypedKey) {
  const GamepadKeyEvent synthesized(QEvent::KeyPress, Qt::Key_Return, false);
  const QKeyEvent typed(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);

  EXPECT_TRUE(GamepadKeyEvent::isGamepad(&synthesized));
  EXPECT_FALSE(GamepadKeyEvent::isGamepad(&typed));
}

// It stands for a controller button, which holds no modifiers. Anything else and every action
// declaring one would answer a press that never held it
TEST(GamepadKeyEventTest, HoldsNoModifiers) {
  const GamepadKeyEvent synthesized(QEvent::KeyPress, Qt::Key_Return, false);

  EXPECT_EQ(synthesized.modifiers(), Qt::NoModifier);
}

// A controller press already reaches the shortcut engine as a gamepad input; publishing it again as
// a keyboard key would count every press twice
TEST(KeyboardInputHandlerTest, IgnoresAKeySynthesizedFromAController) {
  KeyboardInputHandler handler;
  const Recorder recorder;

  GamepadKeyEvent press(QEvent::KeyPress, Qt::Key_Return, false);
  GamepadKeyEvent release(QEvent::KeyRelease, Qt::Key_Return, false);
  deliver(handler, press);
  deliver(handler, release);

  EXPECT_TRUE(recorder.getEvents().empty());
}

TEST(KeyboardInputHandlerTest, PublishesATypedKey) {
  KeyboardInputHandler handler;
  const Recorder recorder;

  QKeyEvent press(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
  QKeyEvent release(QEvent::KeyRelease, Qt::Key_Return, Qt::NoModifier);
  deliver(handler, press);
  deliver(handler, release);

  ASSERT_EQ(recorder.getEvents().size(), 2);
  EXPECT_EQ(recorder.getEvents().at(0).key, Qt::Key_Return);
  EXPECT_TRUE(recorder.getEvents().at(0).pressed);
  EXPECT_FALSE(recorder.getEvents().at(1).pressed);
}

} // namespace firelight::input

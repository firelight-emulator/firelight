#include <firelight/event_dispatcher.hpp>

#include <gtest/gtest.h>

namespace {
// TU-local event types so nothing else can perturb these buses
struct FooEvent {
  int value;
};

struct BarEvent {};

TEST(EventDispatcherTest, DeliversToSubscriber) {
  EventDispatcher bus;
  int received = 0;
  auto conn = bus.subscribe<FooEvent>([&](const FooEvent &e) { received += e.value; });

  bus.publish(FooEvent{.value = 5});
  bus.publish(FooEvent{.value = 3});

  EXPECT_EQ(received, 8);
}

TEST(EventDispatcherTest, ScopedConnectionUnsubscribes) {
  EventDispatcher bus;
  int received = 0;
  {
    auto conn = bus.subscribe<BarEvent>([&](const BarEvent &) { received++; });
    bus.publish(BarEvent{});
    EXPECT_EQ(received, 1);
  }

  bus.publish(BarEvent{});
  EXPECT_EQ(received, 1);
}

// The point of making the dispatcher injectable: a locally-owned bus (as a test
// or a CLI app would supply) is isolated from any other instance
TEST(EventDispatcherTest, InstancesAreIndependent) {
  EventDispatcher a;
  EventDispatcher b;
  int aCount = 0;
  auto conn = a.subscribe<FooEvent>([&](const FooEvent &) { aCount++; });

  b.publish(FooEvent{.value = 1});
  EXPECT_EQ(aCount, 0);

  a.publish(FooEvent{.value = 1});
  EXPECT_EQ(aCount, 1);
}
} // namespace

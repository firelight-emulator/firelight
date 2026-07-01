#include "test_gamepad.hpp"

#include <firelight/event_dispatcher.hpp>
#include <firelight/input/gamepad_profile.hpp>
#include <firelight/input/shortcut_engine.hpp>
#include <firelight/input/shortcut_mapping.hpp>
#include <firelight/input/shortcut_registry.hpp>

#include <gtest/gtest.h>

namespace firelight::input {

namespace {
InputSource btn(const int code, std::vector<int> modifiers = {}) {
  InputSource source;
  source.type = SourceType::Button;
  source.code = code;
  source.modifiers = std::move(modifiers);
  return source;
}
} // namespace

class ShortcutTest : public testing::Test {
protected:
  void SetUp() override { ShortcutRegistry::instance().clear(); }
  void TearDown() override { ShortcutRegistry::instance().clear(); }

  static void addAction(const char *id, const ActivationType activation,
                        const int scope) {
    ShortcutAction action;
    action.id = id;
    action.activation = activation;
    action.scope = scope;
    ShortcutRegistry::instance().registerAction(action);
  }

  static std::shared_ptr<GamepadProfile> makeProfile() {
    auto profile = std::make_shared<GamepadProfile>(1);
    profile->setShortcutMapping(std::make_shared<ShortcutMapping>());
    return profile;
  }
};

TEST_F(ShortcutTest, ShortcutMappingJsonRoundTrip) {
  ShortcutMapping mapping;
  const auto combo = btn(GamepadInput::Select, {GamepadInput::LeftTrigger});
  const auto key = btn(0x20); // space-ish
  mapping.setBindings("fast_forward", {combo, key});
  mapping.setBindings("save_state", {combo});

  ShortcutMapping restored;
  restored.deserialize(mapping.serialize());

  ASSERT_EQ(restored.getBindings("fast_forward").size(), 2u);
  EXPECT_EQ(restored.getBindings("fast_forward")[0], combo);
  EXPECT_EQ(restored.getBindings("save_state").size(), 1u);
  EXPECT_TRUE(restored.getBindings("nonexistent").empty());
}

TEST_F(ShortcutTest, PressFiresOnRisingEdgeOnly) {
  addAction("ss", ActivationType::Press, ScopeInGame);
  auto profile = makeProfile();
  profile->getShortcutMapping()->setBindings("ss", {btn(GamepadInput::Select)});
  TestGamepad pad;
  pad.setProfile(profile);

  ShortcutEngine engine;
  engine.setContext(ScopeInGame);
  std::vector<ShortcutEvent> events;
  const auto conn = EventDispatcher::instance().subscribe<ShortcutEvent>(
      [&events](const ShortcutEvent &e) { events.push_back(e); });

  engine.onInput(0, &pad, GamepadInput::Select, true);
  engine.onInput(0, &pad, GamepadInput::Select, false);

  ASSERT_EQ(events.size(), 1u);
  EXPECT_EQ(events[0].id, "ss");
  EXPECT_EQ(events[0].phase, ShortcutPhase::Started);
}

TEST_F(ShortcutTest, HoldFiresStartedAndEnded) {
  addAction("ff", ActivationType::Hold, ScopeInGame);
  auto profile = makeProfile();
  profile->getShortcutMapping()->setBindings("ff", {btn(GamepadInput::Start)});
  TestGamepad pad;
  pad.setProfile(profile);

  ShortcutEngine engine;
  engine.setContext(ScopeInGame);
  std::vector<ShortcutEvent> events;
  const auto conn = EventDispatcher::instance().subscribe<ShortcutEvent>(
      [&events](const ShortcutEvent &e) { events.push_back(e); });

  engine.onInput(0, &pad, GamepadInput::Start, true);
  engine.onInput(0, &pad, GamepadInput::Start, false);

  ASSERT_EQ(events.size(), 2u);
  EXPECT_EQ(events[0].phase, ShortcutPhase::Started);
  EXPECT_EQ(events[1].phase, ShortcutPhase::Ended);
}

TEST_F(ShortcutTest, ToggleFlipsStateOnEachPress) {
  addAction("mute", ActivationType::Toggle, ScopeInGame);
  auto profile = makeProfile();
  profile->getShortcutMapping()->setBindings("mute", {btn(GamepadInput::Home)});
  TestGamepad pad;
  pad.setProfile(profile);

  ShortcutEngine engine;
  engine.setContext(ScopeInGame);
  std::vector<ShortcutEvent> events;
  const auto conn = EventDispatcher::instance().subscribe<ShortcutEvent>(
      [&events](const ShortcutEvent &e) { events.push_back(e); });

  engine.onInput(0, &pad, GamepadInput::Home, true);
  engine.onInput(0, &pad, GamepadInput::Home, false);
  engine.onInput(0, &pad, GamepadInput::Home, true);

  ASSERT_EQ(events.size(), 2u);
  EXPECT_TRUE(events[0].toggledState);
  EXPECT_FALSE(events[1].toggledState);
}

TEST_F(ShortcutTest, ScopeGatesActivation) {
  addAction("menu_only", ActivationType::Press, ScopeInMenu);
  auto profile = makeProfile();
  profile->getShortcutMapping()->setBindings("menu_only",
                                             {btn(GamepadInput::WestFace)});
  TestGamepad pad;
  pad.setProfile(profile);

  ShortcutEngine engine;
  engine.setContext(ScopeInGame);
  std::vector<ShortcutEvent> events;
  const auto conn = EventDispatcher::instance().subscribe<ShortcutEvent>(
      [&events](const ShortcutEvent &e) { events.push_back(e); });

  // Out of scope: nothing fires.
  engine.onInput(0, &pad, GamepadInput::WestFace, true);
  engine.onInput(0, &pad, GamepadInput::WestFace, false);
  EXPECT_TRUE(events.empty());

  // In scope: fires.
  engine.setContext(ScopeInMenu);
  engine.onInput(0, &pad, GamepadInput::WestFace, true);
  ASSERT_EQ(events.size(), 1u);
  EXPECT_EQ(events[0].id, "menu_only");
}

TEST_F(ShortcutTest, ComboRequiresModifierHeld) {
  addAction("ff", ActivationType::Press, ScopeInGame);
  auto profile = makeProfile();
  profile->getShortcutMapping()->setBindings(
      "ff", {btn(GamepadInput::Start, {GamepadInput::LeftTrigger})});
  TestGamepad pad;
  pad.setProfile(profile);

  ShortcutEngine engine;
  engine.setContext(ScopeInGame);
  std::vector<ShortcutEvent> events;
  const auto conn = EventDispatcher::instance().subscribe<ShortcutEvent>(
      [&events](const ShortcutEvent &e) { events.push_back(e); });

  // Start alone (no modifier) does nothing.
  engine.onInput(0, &pad, GamepadInput::Start, true);
  EXPECT_TRUE(events.empty());
  engine.onInput(0, &pad, GamepadInput::Start, false);

  // Hold the modifier, then press Start.
  engine.onInput(0, &pad, GamepadInput::LeftTrigger, true);
  engine.onInput(0, &pad, GamepadInput::Start, true);
  ASSERT_EQ(events.size(), 1u);
  EXPECT_EQ(events[0].id, "ff");
}

} // namespace firelight::input

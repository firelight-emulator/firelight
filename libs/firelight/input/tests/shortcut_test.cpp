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

// An action that flips something global (mute, pause) reports an edge and lets
// the consumer own the value. The engine must not track that value itself: it
// only ever sees one device, so two devices sharing a profile would each keep
// their own copy and drift apart
TEST_F(ShortcutTest, EveryPressFiresOnEveryDevice) {
  addAction("mute", ActivationType::Press, ScopeInGame);
  auto profile = makeProfile();
  profile->getShortcutMapping()->setBindings("mute", {btn(GamepadInput::Home)});

  // One profile, two pads — what two identical controllers look like
  TestGamepad padOne;
  TestGamepad padTwo;
  padOne.setProfile(profile);
  padTwo.setProfile(profile);

  ShortcutEngine engine;
  engine.setContext(ScopeInGame);
  std::vector<ShortcutEvent> events;
  const auto conn = EventDispatcher::instance().subscribe<ShortcutEvent>(
      [&events](const ShortcutEvent &e) { events.push_back(e); });

  engine.onInput(0, &padOne, GamepadInput::Home, true);
  engine.onInput(0, &padOne, GamepadInput::Home, false);
  engine.onInput(1, &padTwo, GamepadInput::Home, true);
  engine.onInput(1, &padTwo, GamepadInput::Home, false);
  engine.onInput(0, &padOne, GamepadInput::Home, true);

  ASSERT_EQ(events.size(), 3u);
  EXPECT_EQ(events[0].playerIndex, 0);
  EXPECT_EQ(events[1].playerIndex, 1);
  EXPECT_EQ(events[2].playerIndex, 0);
  for (const auto &event : events) {
    EXPECT_EQ(event.phase, ShortcutPhase::Started);
  }
}

// A bare press still reaches the game; the same button as part of a combo does
// not. That conditional is only possible because the modifier disambiguates the
// two intents at the trigger's rising edge — with no lookahead needed
TEST_F(ShortcutTest, ComboWithholdsItsTriggerButABarePressStillReachesTheGame) {
  addAction("save_state", ActivationType::Press, ScopeInGame);
  auto profile = makeProfile();
  profile->getShortcutMapping()->setBindings(
      "save_state",
      {btn(GamepadInput::EastFace, {GamepadInput::Select})});
  TestGamepad pad;
  pad.setProfile(profile);

  ShortcutEngine engine;
  engine.setContext(ScopeInGame);

  // Pressed on its own: no shortcut fires, so the game gets it
  engine.onInput(0, &pad, GamepadInput::EastFace, true);
  EXPECT_FALSE(pad.suppressor().isSuppressed(GamepadInput::EastFace));
  engine.onInput(0, &pad, GamepadInput::EastFace, false);

  // Held with the modifier: the shortcut fires and the game must not see it
  engine.onInput(0, &pad, GamepadInput::Select, true);
  engine.onInput(0, &pad, GamepadInput::EastFace, true);
  EXPECT_TRUE(pad.suppressor().isSuppressed(GamepadInput::EastFace));

  // The modifier leaks on purpose: it was already down, so masking it now would
  // hand the core a release it never got
  EXPECT_FALSE(pad.suppressor().isSuppressed(GamepadInput::Select));

  // Letting the modifier go first must not hand the core a spurious press —
  // the trigger stays withheld until it is itself released
  engine.onInput(0, &pad, GamepadInput::Select, false);
  EXPECT_TRUE(pad.suppressor().isSuppressed(GamepadInput::EastFace));

  engine.onInput(0, &pad, GamepadInput::EastFace, false);
  EXPECT_FALSE(pad.suppressor().isSuppressed(GamepadInput::EastFace));
}

TEST_F(ShortcutTest, LeavingGameplayDropsEveryMask) {
  addAction("screenshot", ActivationType::Press, ScopeAlways);
  auto profile = makeProfile();
  profile->getShortcutMapping()->setBindings(
      "screenshot", {btn(GamepadInput::WestFace, {GamepadInput::Select})});
  TestGamepad pad;
  pad.setProfile(profile);

  ShortcutEngine engine;
  engine.setContext(ScopeInGame);
  engine.onInput(0, &pad, GamepadInput::Select, true);
  engine.onInput(0, &pad, GamepadInput::WestFace, true);
  ASSERT_TRUE(pad.suppressor().isSuppressed(GamepadInput::WestFace));

  // A ScopeAlways action can fire in a menu, where nothing reads the mask; it
  // must not still be set when the game comes back
  engine.setContext(ScopeInMenu);
  EXPECT_FALSE(pad.suppressor().isSuppressed(GamepadInput::WestFace));
}

TEST_F(ShortcutTest, ForgettingADeviceDropsItsMasks) {
  addAction("pause", ActivationType::Press, ScopeInGame);
  auto profile = makeProfile();
  profile->getShortcutMapping()->setBindings(
      "pause", {btn(GamepadInput::SouthFace, {GamepadInput::Select})});
  TestGamepad pad;
  pad.setProfile(profile);

  ShortcutEngine engine;
  engine.setContext(ScopeInGame);
  engine.onInput(0, &pad, GamepadInput::Select, true);
  engine.onInput(0, &pad, GamepadInput::SouthFace, true);
  ASSERT_TRUE(pad.suppressor().isSuppressed(GamepadInput::SouthFace));

  // Unplugged mid-combo: the release never arrives, so the mask has to go with
  // the device or it would wedge that button forever
  engine.forgetDevice(&pad);
  EXPECT_FALSE(pad.suppressor().isSuppressed(GamepadInput::SouthFace));
}

// Turning hotkeys off hands the whole device to the game — which is what a core
// that wants the entire keyboard (DOS, Amiga, MSX) needs, and what no amount of
// per-binding cleverness can give it
TEST_F(ShortcutTest, HotkeysOffSilencesEverythingButTheWayBack) {
  addAction("save_state", ActivationType::Press, ScopeInGame);
  addAction(TOGGLE_HOTKEYS_ID, ActivationType::Press, ScopeAlways);
  auto profile = makeProfile();
  profile->getShortcutMapping()->setBindings("save_state",
                                            {btn(GamepadInput::EastFace)});
  profile->getShortcutMapping()->setBindings(TOGGLE_HOTKEYS_ID,
                                            {btn(GamepadInput::Home)});
  TestGamepad pad;
  pad.setProfile(profile);

  ShortcutEngine engine;
  engine.setContext(ScopeInGame);
  std::vector<ShortcutEvent> events;
  const auto conn = EventDispatcher::instance().subscribe<ShortcutEvent>(
      [&events](const ShortcutEvent &e) { events.push_back(e); });

  EXPECT_TRUE(engine.hotkeysEnabled(&pad));
  engine.onInput(0, &pad, GamepadInput::EastFace, true);
  EXPECT_EQ(events.size(), 1u);
  engine.onInput(0, &pad, GamepadInput::EastFace, false);

  engine.onInput(0, &pad, GamepadInput::Home, true);
  engine.onInput(0, &pad, GamepadInput::Home, false);
  EXPECT_FALSE(engine.hotkeysEnabled(&pad));

  // Off: the game gets this, the shortcut doesn't fire, and nothing is withheld
  events.clear();
  engine.onInput(0, &pad, GamepadInput::EastFace, true);
  EXPECT_TRUE(events.empty());
  EXPECT_FALSE(pad.suppressor().isSuppressed(GamepadInput::EastFace));
  engine.onInput(0, &pad, GamepadInput::EastFace, false);

  // The toggle itself still works, or there'd be no way back
  engine.onInput(0, &pad, GamepadInput::Home, true);
  EXPECT_TRUE(engine.hotkeysEnabled(&pad));
  engine.onInput(0, &pad, GamepadInput::Home, false);

  events.clear();
  engine.onInput(0, &pad, GamepadInput::EastFace, true);
  EXPECT_EQ(events.size(), 1u) << "hotkeys come back on";
}

TEST_F(ShortcutTest, HotkeysOffAppliesOnlyToTheDeviceThatDidIt) {
  addAction("save_state", ActivationType::Press, ScopeInGame);
  auto profile = makeProfile();
  profile->getShortcutMapping()->setBindings("save_state",
                                            {btn(GamepadInput::EastFace)});
  // One profile, two pads: turning them off on one must not touch the other
  TestGamepad padOne;
  TestGamepad padTwo;
  padOne.setProfile(profile);
  padTwo.setProfile(profile);

  ShortcutEngine engine;
  engine.setContext(ScopeInGame);
  std::vector<ShortcutEvent> events;
  const auto conn = EventDispatcher::instance().subscribe<ShortcutEvent>(
      [&events](const ShortcutEvent &e) { events.push_back(e); });

  engine.setHotkeysEnabled(&padOne, false);
  EXPECT_FALSE(engine.hotkeysEnabled(&padOne));
  EXPECT_TRUE(engine.hotkeysEnabled(&padTwo));

  engine.onInput(0, &padOne, GamepadInput::EastFace, true);
  EXPECT_TRUE(events.empty());

  engine.onInput(1, &padTwo, GamepadInput::EastFace, true);
  ASSERT_EQ(events.size(), 1u);
  EXPECT_EQ(events[0].playerIndex, 1);
}

TEST_F(ShortcutTest, TurningHotkeysOffHandsBackAnythingWithheld) {
  addAction("save_state", ActivationType::Press, ScopeInGame);
  auto profile = makeProfile();
  profile->getShortcutMapping()->setBindings(
      "save_state", {btn(GamepadInput::EastFace, {GamepadInput::Select})});
  TestGamepad pad;
  pad.setProfile(profile);

  ShortcutEngine engine;
  engine.setContext(ScopeInGame);
  engine.onInput(0, &pad, GamepadInput::Select, true);
  engine.onInput(0, &pad, GamepadInput::EastFace, true);
  ASSERT_TRUE(pad.suppressor().isSuppressed(GamepadInput::EastFace));

  // Giving the game its inputs back is the whole point, so a mask can't outlive
  // the switch being thrown
  engine.setHotkeysEnabled(&pad, false);
  EXPECT_FALSE(pad.suppressor().isSuppressed(GamepadInput::EastFace));
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

  // Out of scope: nothing fires
  engine.onInput(0, &pad, GamepadInput::WestFace, true);
  engine.onInput(0, &pad, GamepadInput::WestFace, false);
  EXPECT_TRUE(events.empty());

  // In scope: fires
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

  // Start alone (no modifier) does nothing
  engine.onInput(0, &pad, GamepadInput::Start, true);
  EXPECT_TRUE(events.empty());
  engine.onInput(0, &pad, GamepadInput::Start, false);

  // Hold the modifier, then press Start
  engine.onInput(0, &pad, GamepadInput::LeftTrigger, true);
  engine.onInput(0, &pad, GamepadInput::Start, true);
  ASSERT_EQ(events.size(), 1u);
  EXPECT_EQ(events[0].id, "ff");
}

TEST_F(ShortcutTest, ShortcutMappingAddRemoveSet) {
  ShortcutMapping mapping;
  mapping.addBinding("ff", btn(GamepadInput::Start));
  mapping.addBinding("ff", btn(GamepadInput::Select));
  ASSERT_EQ(mapping.getBindings("ff").size(), 2u);

  mapping.removeBinding("ff", 0);
  ASSERT_EQ(mapping.getBindings("ff").size(), 1u);
  EXPECT_EQ(mapping.getBindings("ff").front().code, GamepadInput::Select);

  mapping.setBindings("ff", {});
  EXPECT_TRUE(mapping.getBindings("ff").empty());
  EXPECT_TRUE(mapping.getAll().empty());
}

TEST_F(ShortcutTest, AlternateSourcesEitherFires) {
  addAction("ss", ActivationType::Press, ScopeInGame);
  auto profile = makeProfile();
  profile->getShortcutMapping()->setBindings(
      "ss", {btn(GamepadInput::Select), btn(GamepadInput::Start)});
  TestGamepad pad;
  pad.setProfile(profile);

  ShortcutEngine engine;
  engine.setContext(ScopeInGame);
  std::vector<ShortcutEvent> events;
  const auto conn = EventDispatcher::instance().subscribe<ShortcutEvent>(
      [&events](const ShortcutEvent &e) { events.push_back(e); });

  // Either alternate fires the same action
  engine.onInput(0, &pad, GamepadInput::Start, true);
  ASSERT_EQ(events.size(), 1u);
  EXPECT_EQ(events[0].id, "ss");
}

TEST_F(ShortcutTest, HoldEndsEvenAfterScopeChange) {
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

  engine.onInput(0, &pad, GamepadInput::Start, true); // Started (in scope)
  engine.setContext(ScopeInMenu);                     // leave the scope
  engine.onInput(0, &pad, GamepadInput::Start, false);

  ASSERT_EQ(events.size(), 2u);
  EXPECT_EQ(events[0].phase, ShortcutPhase::Started);
  EXPECT_EQ(events[1].phase, ShortcutPhase::Ended);
}

TEST_F(ShortcutTest, ForgetDeviceResetsHoldState) {
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

  engine.onInput(0, &pad, GamepadInput::Start, true); // Started
  engine.forgetDevice(&pad);
  // After forgetting, the stale release should not emit an Ended
  engine.onInput(0, &pad, GamepadInput::Start, false);

  ASSERT_EQ(events.size(), 1u);
  EXPECT_EQ(events[0].phase, ShortcutPhase::Started);
}

TEST_F(ShortcutTest, NullProfileIsSafe) {
  addAction("ss", ActivationType::Press, ScopeInGame);
  TestGamepad pad; // no profile set
  ShortcutEngine engine;
  engine.setContext(ScopeInGame);
  std::vector<ShortcutEvent> events;
  const auto conn = EventDispatcher::instance().subscribe<ShortcutEvent>(
      [&events](const ShortcutEvent &e) { events.push_back(e); });

  EXPECT_NO_THROW(engine.onInput(0, &pad, GamepadInput::Select, true));
  EXPECT_TRUE(events.empty());
}

} // namespace firelight::input

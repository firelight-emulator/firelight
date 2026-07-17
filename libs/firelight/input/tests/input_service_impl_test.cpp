#include <firelight/event_dispatcher.hpp>
#include "test_gamepad.hpp"

#include <gtest/gtest.h>
#include <firelight/input/gamepad_profile.hpp>
#include <firelight/input/shortcut_mapping.hpp>
#include <firelight/input/shortcut_registry.hpp>
#include <firelight/input/sqlite_controller_repository.hpp>
#include <firelight/input/sdl_input_service.hpp>
#include <firelight/platforms/platform_service.hpp>

#include <atomic>
#include <thread>

namespace firelight::db {
class InputServiceImplTest : public testing::Test {
protected:
  InputServiceImplTest() {
    m_connectedHandler =
        EventDispatcher::instance().subscribe<input::GamepadConnectedEvent>(
            [this](const input::GamepadConnectedEvent &event) {
              m_connectedEvents.push_back(event);
            });

    m_disconnectedHandler =
        EventDispatcher::instance().subscribe<input::GamepadDisconnectedEvent>(
            [this](const input::GamepadDisconnectedEvent &event) {
              m_disconnectedEvents.push_back(event);
            });

    m_orderChangedHandler =
        EventDispatcher::instance().subscribe<input::GamepadOrderChangedEvent>(
            [this](const input::GamepadOrderChangedEvent &event) {
              m_orderChangedEvents.push_back(event);
            });
  }

  platforms::PlatformService m_platformService;
  input::SqliteControllerRepository m_repo =
      input::SqliteControllerRepository(":memory:", m_platformService);

  ScopedConnection m_connectedHandler;
  ScopedConnection m_disconnectedHandler;
  ScopedConnection m_orderChangedHandler;

  std::vector<input::GamepadConnectedEvent> m_connectedEvents;
  std::vector<input::GamepadDisconnectedEvent> m_disconnectedEvents;
  std::vector<input::GamepadOrderChangedEvent> m_orderChangedEvents;
};

TEST_F(InputServiceImplTest, AddGamepadTest) {
  input::SDLInputService inputService(m_repo);
  auto gamepad = std::make_shared<input::TestGamepad>();

  inputService.addGamepad(gamepad);
  ASSERT_EQ(gamepad->getPlayerIndex(), 0);

  auto retrieved = inputService.getPlayerGamepad(0);
  ASSERT_EQ(retrieved, gamepad);
}

TEST_F(InputServiceImplTest, PlatformPreferredControllerPromotedToPlayerOne) {
  input::SDLInputService inputService(m_repo);

  auto xboxPad = std::make_shared<input::TestGamepad>(1);
  xboxPad->setType(MICROSOFT_XBOX_ONE);
  auto n64Pad = std::make_shared<input::TestGamepad>(2);
  n64Pad->setType(NINTENDO_NSO_N64);

  inputService.addGamepad(xboxPad); // player 0
  inputService.addGamepad(n64Pad);  // player 1
  ASSERT_EQ(xboxPad->getPlayerIndex(), 0);
  ASSERT_EQ(n64Pad->getPlayerIndex(), 1);

  constexpr int platformId = 5;
  m_repo.setPlatformPreferredType(platformId, NINTENDO_NSO_N64);

  inputService.applyGameContext(std::nullopt, platformId);

  // The N64 controller should now be player one, swapping with the Xbox pad.
  ASSERT_EQ(n64Pad->getPlayerIndex(), 0);
  ASSERT_EQ(xboxPad->getPlayerIndex(), 1);
}

TEST_F(InputServiceImplTest, SessionPreferredControllerOverridesStoredPreference) {
  input::SDLInputService inputService(m_repo);

  auto xboxPad = std::make_shared<input::TestGamepad>(1);
  xboxPad->setType(MICROSOFT_XBOX_ONE);
  auto n64Pad = std::make_shared<input::TestGamepad>(2);
  n64Pad->setType(NINTENDO_NSO_N64);

  inputService.addGamepad(xboxPad); // player 0
  inputService.addGamepad(n64Pad);  // player 1

  constexpr int platformId = 5;
  // The stored preference points at the Xbox pad...
  m_repo.setPlatformPreferredType(platformId, MICROSOFT_XBOX_ONE);
  // ...but a CLI --controller session override for the N64 pad wins.
  inputService.setSessionPreferredControllerType(platformId, NINTENDO_NSO_N64);

  inputService.applyGameContext(std::nullopt, platformId);

  ASSERT_EQ(n64Pad->getPlayerIndex(), 0);
  ASSERT_EQ(xboxPad->getPlayerIndex(), 1);
}

TEST_F(InputServiceImplTest, AddKeyboardTest) {
  input::SDLInputService inputService(m_repo);
  auto keyboard = std::make_shared<input::TestGamepad>();

  inputService.setKeyboard(keyboard);
  ASSERT_EQ(keyboard->getPlayerIndex(), 0);

  auto retrieved = inputService.getPlayerGamepad(0);
  ASSERT_EQ(retrieved, keyboard);
}

TEST_F(InputServiceImplTest, AddKeyboardThenGamepadWithoutPreferGamepadTest) {
  input::SDLInputService inputService(m_repo);
  inputService.setPreferGamepadOverKeyboard(false);
  auto keyboard = std::make_shared<input::TestGamepad>();

  inputService.setKeyboard(keyboard);
  ASSERT_EQ(keyboard->getPlayerIndex(), 0);

  auto retrieved = inputService.getPlayerGamepad(0);
  ASSERT_EQ(retrieved, keyboard);

  auto gamepad = std::make_shared<input::TestGamepad>();
  inputService.addGamepad(gamepad);

  ASSERT_EQ(gamepad->getPlayerIndex(), 1);
  retrieved = inputService.getPlayerGamepad(1);
  ASSERT_EQ(retrieved, gamepad);

  ASSERT_EQ(keyboard->getPlayerIndex(), 0);

  ASSERT_EQ(inputService.listGamepads().size(), 2);
}

TEST_F(InputServiceImplTest, AddKeyboardThenGamepadWithPreferGamepadTest) {
  input::SDLInputService inputService(m_repo);
  inputService.setPreferGamepadOverKeyboard(true);
  auto keyboard = std::make_shared<input::TestGamepad>();

  inputService.setKeyboard(keyboard);
  ASSERT_EQ(keyboard->getPlayerIndex(), 0);

  auto retrieved = inputService.getPlayerGamepad(0);
  ASSERT_EQ(retrieved, keyboard);

  auto gamepad = std::make_shared<input::TestGamepad>();
  inputService.addGamepad(gamepad);

  // Should have two gamepads
  ASSERT_EQ(inputService.listGamepads().size(), 2);

  // Gamepad should be player 1 now
  ASSERT_EQ(gamepad->getPlayerIndex(), 0);
  retrieved = inputService.getPlayerGamepad(1);
  ASSERT_EQ(retrieved, keyboard);

  // Keyboard should be player 2
  ASSERT_EQ(keyboard->getPlayerIndex(), 1);

  // Remove player 1 (gamepad)
  inputService.removeGamepadByPlayerIndex(0);
  ASSERT_EQ(gamepad->getPlayerIndex(), -1);

  // Keyboard should still be player 2
  ASSERT_EQ(keyboard->getPlayerIndex(), 1);
  ASSERT_EQ(keyboard, inputService.getPlayerGamepad(1));

  // Plug in gamepad again, should be player 1 again
  inputService.addGamepad(gamepad);
  ASSERT_EQ(gamepad->getPlayerIndex(), 0);
  ASSERT_EQ(gamepad, inputService.getPlayerGamepad(0));
  ASSERT_EQ(keyboard, inputService.getPlayerGamepad(1));
}

TEST_F(InputServiceImplTest, RemoveGamepadByInstanceIdTest) {
  input::SDLInputService inputService(m_repo);
  auto gamepad = std::make_shared<input::TestGamepad>();

  inputService.addGamepad(gamepad);
  ASSERT_EQ(gamepad->getPlayerIndex(), 0);

  auto retrieved = inputService.getPlayerGamepad(0);
  ASSERT_EQ(retrieved, gamepad);

  inputService.removeGamepadByInstanceId(gamepad->getInstanceId());
  retrieved = inputService.getPlayerGamepad(0);
  ASSERT_EQ(retrieved, nullptr);
  ASSERT_EQ(0, inputService.listGamepads().size());
}

TEST_F(InputServiceImplTest, RemoveGamepadByPlayerIndex) {
  input::SDLInputService inputService(m_repo);
  auto gamepad = std::make_shared<input::TestGamepad>();

  inputService.addGamepad(gamepad);
  ASSERT_EQ(gamepad->getPlayerIndex(), 0);

  auto retrieved = inputService.getPlayerGamepad(0);
  ASSERT_EQ(retrieved, gamepad);

  inputService.removeGamepadByPlayerIndex(0);
  retrieved = inputService.getPlayerGamepad(0);
  ASSERT_EQ(retrieved, nullptr);
  ASSERT_EQ(0, inputService.listGamepads().size());
}

TEST_F(InputServiceImplTest, GetPlayerGamepadEmptySlotReturnsNull) {
  input::SDLInputService inputService(m_repo);
  EXPECT_EQ(inputService.getPlayerGamepad(3), nullptr);
}

TEST_F(InputServiceImplTest, ChangeGamepadOrderSwapsSlots) {
  input::SDLInputService inputService(m_repo);
  auto padA = std::make_shared<input::TestGamepad>(1);
  auto padB = std::make_shared<input::TestGamepad>(2);
  inputService.addGamepad(padA);
  inputService.addGamepad(padB);
  ASSERT_EQ(padA->getPlayerIndex(), 0);
  ASSERT_EQ(padB->getPlayerIndex(), 1);

  inputService.changeGamepadOrder({{0, 1}, {1, 0}});
  EXPECT_EQ(inputService.getPlayerGamepad(0), padB);
  EXPECT_EQ(inputService.getPlayerGamepad(1), padA);
}

TEST_F(InputServiceImplTest, GameContextOverrideSwapsProfileAndRestores) {
  input::SDLInputService inputService(m_repo);
  const auto overrideProfile = m_repo.createProfile("GameOverride");
  ASSERT_TRUE(overrideProfile);
  m_repo.setGameProfileOverride("hashX", overrideProfile->getId());

  auto pad = std::make_shared<input::TestGamepad>(1);
  inputService.addGamepad(pad);
  ASSERT_TRUE(pad->getProfile());
  const auto deviceDefaultId = pad->getProfile()->getId();

  // Applying the game context swaps the device to the override profile.
  inputService.applyGameContext("hashX", -1);
  ASSERT_TRUE(pad->getProfile());
  EXPECT_EQ(pad->getProfile()->getId(), overrideProfile->getId());

  // Clearing restores the device's default profile.
  inputService.clearGameContext();
  EXPECT_EQ(pad->getProfile()->getId(), deviceDefaultId);
}

// ---------------------------------------------------------------------------
// Device connect/disconnect lifecycle + emitted events. These exercise the
// public API that the SDL event loop drives on CONTROLLERDEVICEADDED/REMOVED.
// ---------------------------------------------------------------------------

TEST_F(InputServiceImplTest, AddGamepadPublishesConnectedEvent) {
  input::SDLInputService inputService(m_repo);
  auto pad = std::make_shared<input::TestGamepad>(1);

  inputService.addGamepad(pad);

  ASSERT_EQ(m_connectedEvents.size(), 1u);
  EXPECT_EQ(m_connectedEvents[0].gamepad, pad);
  EXPECT_TRUE(m_disconnectedEvents.empty());
}

TEST_F(InputServiceImplTest, RemoveByInstanceIdPublishesDisconnectedWithSlot) {
  input::SDLInputService inputService(m_repo);
  auto pad = std::make_shared<input::TestGamepad>(1);
  inputService.addGamepad(pad);
  ASSERT_EQ(pad->getPlayerIndex(), 0);

  inputService.removeGamepadByInstanceId(1);

  ASSERT_EQ(m_disconnectedEvents.size(), 1u);
  EXPECT_EQ(m_disconnectedEvents[0].playerIndex, 0);
  EXPECT_EQ(inputService.getPlayerGamepad(0), nullptr);
}

TEST_F(InputServiceImplTest, UnpluggingMiddleFreesSlotAndItIsReused) {
  input::SDLInputService inputService(m_repo);
  auto p0 = std::make_shared<input::TestGamepad>(1);
  auto p1 = std::make_shared<input::TestGamepad>(2);
  auto p2 = std::make_shared<input::TestGamepad>(3);
  inputService.addGamepad(p0);
  inputService.addGamepad(p1);
  inputService.addGamepad(p2);
  ASSERT_EQ(p1->getPlayerIndex(), 1);

  inputService.removeGamepadByInstanceId(2); // the player-2 device

  EXPECT_EQ(inputService.getPlayerGamepad(0), p0);
  EXPECT_EQ(inputService.getPlayerGamepad(1), nullptr); // slot freed
  EXPECT_EQ(inputService.getPlayerGamepad(2), p2);      // others keep slots

  // The next device fills the freed slot rather than a new one.
  auto p3 = std::make_shared<input::TestGamepad>(4);
  inputService.addGamepad(p3);
  EXPECT_EQ(p3->getPlayerIndex(), 1);
}

TEST_F(InputServiceImplTest, ReplugSameDeviceReusesItsProfile) {
  input::SDLInputService inputService(m_repo);
  auto first = std::make_shared<input::TestGamepad>(1);
  inputService.addGamepad(first);
  ASSERT_TRUE(first->getProfile());
  const auto profileId = first->getProfile()->getId();

  inputService.removeGamepadByInstanceId(1);

  // Same model (same vendor/product/version) reconnecting gets the same profile.
  auto again = std::make_shared<input::TestGamepad>(2);
  inputService.addGamepad(again);
  ASSERT_TRUE(again->getProfile());
  EXPECT_EQ(again->getProfile()->getId(), profileId);
}

TEST_F(InputServiceImplTest, RemoveUnknownInstanceIsNoOp) {
  input::SDLInputService inputService(m_repo);
  auto pad = std::make_shared<input::TestGamepad>(1);
  inputService.addGamepad(pad);
  m_disconnectedEvents.clear();

  inputService.removeGamepadByInstanceId(999); // never connected

  EXPECT_TRUE(m_disconnectedEvents.empty());
  EXPECT_EQ(inputService.getPlayerGamepad(0), pad); // untouched
}

TEST_F(InputServiceImplTest, RemoveEmptyPlayerIndexIsNoOp) {
  input::SDLInputService inputService(m_repo);
  inputService.removeGamepadByPlayerIndex(5);
  EXPECT_TRUE(m_disconnectedEvents.empty());
}

TEST_F(InputServiceImplTest, ChangeOrderPublishesOrderChangedEvent) {
  input::SDLInputService inputService(m_repo);
  inputService.addGamepad(std::make_shared<input::TestGamepad>(1));
  inputService.addGamepad(std::make_shared<input::TestGamepad>(2));

  inputService.changeGamepadOrder({{0, 1}, {1, 0}});

  EXPECT_FALSE(m_orderChangedEvents.empty());
}

TEST_F(InputServiceImplTest, ListGamepadsTracksConnectAndDisconnect) {
  input::SDLInputService inputService(m_repo);
  auto a = std::make_shared<input::TestGamepad>(1);
  auto b = std::make_shared<input::TestGamepad>(2);
  inputService.addGamepad(a);
  inputService.addGamepad(b);
  EXPECT_EQ(inputService.listGamepads().size(), 2u);

  inputService.removeGamepadByInstanceId(1);
  EXPECT_EQ(inputService.listGamepads().size(), 1u);
}

TEST_F(InputServiceImplTest, ExtraDeviceBeyondMaxPlayersGetsNoSlotButConnects) {
  input::SDLInputService inputService(m_repo);
  std::vector<std::shared_ptr<input::TestGamepad>> pads;
  for (int i = 1; i <= 16; ++i) {
    auto pad = std::make_shared<input::TestGamepad>(i);
    pads.push_back(pad);
    inputService.addGamepad(pad);
  }
  for (int i = 0; i < 16; ++i) {
    EXPECT_EQ(pads[i]->getPlayerIndex(), i);
  }

  auto overflow = std::make_shared<input::TestGamepad>(17);
  inputService.addGamepad(overflow);

  EXPECT_EQ(overflow->getPlayerIndex(), -1);       // no free slot
  EXPECT_EQ(m_connectedEvents.size(), 17u);        // still announced
  EXPECT_EQ(inputService.listGamepads().size(), 17u);
}

TEST_F(InputServiceImplTest, RapidConnectDisconnectCyclesKeepBalancedEvents) {
  input::SDLInputService inputService(m_repo);
  for (int i = 1; i <= 20; ++i) {
    auto pad = std::make_shared<input::TestGamepad>(i);
    inputService.addGamepad(pad);
    inputService.removeGamepadByInstanceId(i);
  }

  EXPECT_EQ(m_connectedEvents.size(), 20u);
  EXPECT_EQ(m_disconnectedEvents.size(), 20u);
  EXPECT_TRUE(inputService.listGamepads().empty());
  EXPECT_EQ(inputService.getPlayerGamepad(0), nullptr);
}

TEST_F(InputServiceImplTest, UnpluggingGamepadLeavesKeyboardConnected) {
  input::SDLInputService inputService(m_repo);
  inputService.setPreferGamepadOverKeyboard(true);
  auto keyboard = std::make_shared<input::TestGamepad>(-2);
  inputService.setKeyboard(keyboard);
  auto pad = std::make_shared<input::TestGamepad>(1);
  inputService.addGamepad(pad);

  // Gamepad preferred into player one, keyboard bumped.
  ASSERT_EQ(pad->getPlayerIndex(), 0);
  ASSERT_EQ(keyboard->getPlayerIndex(), 1);

  inputService.removeGamepadByInstanceId(1);

  // Keyboard remains connected in its slot.
  EXPECT_EQ(inputService.getPlayerGamepad(1), keyboard);
  bool keyboardStillListed = false;
  for (const auto &g : inputService.listGamepads()) {
    if (g == keyboard) {
      keyboardStillListed = true;
    }
  }
  EXPECT_TRUE(keyboardStillListed);
}

// ---------------------------------------------------------------------------
// Concurrency: the SDL event loop drives connect/disconnect on its own thread
// (QtConcurrent::run in the app) while the render/UI threads read player slots
// every frame. This hammers that pattern: one thread continuously reads while
// another churns the device set. It relies on the device mutex to avoid
// iterator invalidation / torn reads. (Reads treat handed-out gamepads as
// opaque and only touch immutable fields; the collections are what's raced.)
// ---------------------------------------------------------------------------
TEST_F(InputServiceImplTest, ConcurrentReadsDuringConnectDisconnectAreSafe) {
  input::SDLInputService inputService(m_repo);
  constexpr int SLOTS = 16;

  std::atomic<bool> stop{false};
  std::thread reader([&] {
    while (!stop.load(std::memory_order_relaxed)) {
      for (int i = 0; i < SLOTS; ++i) {
        // Non-null check only: don't read mutable per-device fields here.
        (void)(inputService.getPlayerGamepad(i) != nullptr);
        (void)inputService.getRetropadForPlayerIndex(i);
      }
      const auto all = inputService.listGamepads();
      for (const auto &g : all) {
        if (g) {
          (void)g->getInstanceId(); // instance id is immutable after creation
        }
      }
    }
  });

  // Churn a rotating set of four controllers many times.
  for (int round = 0; round < 500; ++round) {
    std::vector<std::shared_ptr<input::TestGamepad>> pads;
    for (int i = 0; i < 4; ++i) {
      auto pad = std::make_shared<input::TestGamepad>(round * 4 + i + 1);
      pads.push_back(pad);
      inputService.addGamepad(pad);
    }
    for (const auto &pad : pads) {
      inputService.removeGamepadByInstanceId(pad->getInstanceId());
    }
  }

  stop.store(true, std::memory_order_relaxed);
  reader.join();

  // After the storm every device is gone and the slots are consistent.
  EXPECT_TRUE(inputService.listGamepads().empty());
  for (int i = 0; i < SLOTS; ++i) {
    EXPECT_EQ(inputService.getPlayerGamepad(i), nullptr);
  }
}

// A reader observing player-one while a preferred controller is promoted (a
// GamepadOrderChangedEvent path) must never see a corrupt slot map.
TEST_F(InputServiceImplTest, ConcurrentReadsDuringOrderChangesAreSafe) {
  input::SDLInputService inputService(m_repo);
  auto a = std::make_shared<input::TestGamepad>(1);
  auto b = std::make_shared<input::TestGamepad>(2);
  inputService.addGamepad(a);
  inputService.addGamepad(b);

  std::atomic<bool> stop{false};
  std::thread reader([&] {
    while (!stop.load(std::memory_order_relaxed)) {
      (void)inputService.getPlayerGamepad(0);
      (void)inputService.getPlayerGamepad(1);
      (void)inputService.listGamepads();
    }
  });

  for (int i = 0; i < 2000; ++i) {
    inputService.changeGamepadOrder({{0, 1}, {1, 0}});
  }

  stop.store(true, std::memory_order_relaxed);
  reader.join();

  // Both controllers are still present, one per slot.
  EXPECT_EQ(inputService.listGamepads().size(), 2u);
  EXPECT_NE(inputService.getPlayerGamepad(0), nullptr);
  EXPECT_NE(inputService.getPlayerGamepad(1), nullptr);
  EXPECT_NE(inputService.getPlayerGamepad(0), inputService.getPlayerGamepad(1));
}

TEST_F(InputServiceImplTest, AxisDecodeSetsBothStickDirections) {
  using input::SDLInputService;

  const auto centered =
      SDLInputService::decodeAxisMotion(SDL_CONTROLLER_AXIS_LEFTX, 0);
  ASSERT_EQ(centered.size(), 2u);
  EXPECT_EQ(centered[0], std::make_pair(input::LeftStickLeft, false));
  EXPECT_EQ(centered[1], std::make_pair(input::LeftStickRight, false));

  const auto right =
      SDLInputService::decodeAxisMotion(SDL_CONTROLLER_AXIS_LEFTX, 20000);
  EXPECT_EQ(right[0], std::make_pair(input::LeftStickLeft, false));
  EXPECT_EQ(right[1], std::make_pair(input::LeftStickRight, true));

  // The opposite direction is reported as released rather than left alone, so
  // flicking extreme-to-extreme without passing through the deadzone doesn't
  // leave it stuck on.
  const auto left =
      SDLInputService::decodeAxisMotion(SDL_CONTROLLER_AXIS_LEFTX, -20000);
  EXPECT_EQ(left[0], std::make_pair(input::LeftStickLeft, true));
  EXPECT_EQ(left[1], std::make_pair(input::LeftStickRight, false));
}

TEST_F(InputServiceImplTest, AxisDecodeTreatsTriggersAsOneDirection) {
  using input::SDLInputService;

  const auto released =
      SDLInputService::decodeAxisMotion(SDL_CONTROLLER_AXIS_TRIGGERRIGHT, 0);
  ASSERT_EQ(released.size(), 1u);
  EXPECT_EQ(released[0], std::make_pair(input::RightTrigger, false));

  const auto pulled = SDLInputService::decodeAxisMotion(
      SDL_CONTROLLER_AXIS_TRIGGERRIGHT, 20000);
  ASSERT_EQ(pulled.size(), 1u);
  EXPECT_EQ(pulled[0], std::make_pair(input::RightTrigger, true));

  EXPECT_TRUE(SDLInputService::decodeAxisMotion(SDL_CONTROLLER_AXIS_MAX, 20000)
                  .empty());
}

// Triggers are axes, not buttons, so a shortcut bound to one only fires if the
// axis path reaches the shortcut engine.
TEST_F(InputServiceImplTest, TriggerAxisFiresShortcutBoundToIt) {
  input::ShortcutRegistry::instance().clear();
  input::ShortcutAction action;
  action.id = "fast_forward";
  action.activation = input::ActivationType::Press;
  action.scope = input::ScopeAlways;
  input::ShortcutRegistry::instance().registerAction(action);

  std::vector<input::ShortcutEvent> fired;
  auto handler = EventDispatcher::instance().subscribe<input::ShortcutEvent>(
      [&fired](const input::ShortcutEvent &event) { fired.push_back(event); });

  input::SDLInputService inputService(m_repo);
  auto gamepad = std::make_shared<input::TestGamepad>(7);
  inputService.addGamepad(gamepad);

  // After adding: addGamepad resolves the device's profile from the repository,
  // which would replace one set beforehand.
  auto profile = std::make_shared<input::GamepadProfile>(1);
  auto mapping = std::make_shared<input::ShortcutMapping>();
  input::InputSource trigger;
  trigger.type = input::SourceType::AxisPositive;
  trigger.code = input::RightTrigger;
  mapping->setBindings("fast_forward", {trigger});
  profile->setShortcutMapping(mapping);
  gamepad->setProfile(profile);

  inputService.handleAxisMotion(7, SDL_CONTROLLER_AXIS_TRIGGERRIGHT, 20000);
  ASSERT_EQ(fired.size(), 1u);
  EXPECT_EQ(fired[0].id, "fast_forward");
  EXPECT_EQ(fired[0].phase, input::ShortcutPhase::Started);

  // Still held: no second edge.
  inputService.handleAxisMotion(7, SDL_CONTROLLER_AXIS_TRIGGERRIGHT, 25000);
  EXPECT_EQ(fired.size(), 1u);

  inputService.handleAxisMotion(7, SDL_CONTROLLER_AXIS_TRIGGERRIGHT, 0);
  EXPECT_EQ(fired.size(), 1u);

  inputService.handleAxisMotion(7, SDL_CONTROLLER_AXIS_TRIGGERRIGHT, 20000);
  EXPECT_EQ(fired.size(), 2u);

  input::ShortcutRegistry::instance().clear();
}

} // namespace firelight::db

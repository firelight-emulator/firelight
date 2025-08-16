#include "event_dispatcher.hpp"
#include "test_gamepad.hpp"

#include <gtest/gtest.h>
#include <input/sqlite_controller_repository.hpp>
#include <input2/sdl/sdl_input_service.hpp>

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

  input::SqliteControllerRepository m_repo =
      input::SqliteControllerRepository(":memory:");

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

} // namespace firelight::db

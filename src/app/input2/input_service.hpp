#pragma once

#include "gamepad.hpp"
#include <firelight/libretro/retropad_provider.hpp>
#include <memory>

namespace firelight::input {

struct GamepadConnectedEvent {
  std::shared_ptr<IGamepad> gamepad;
};

struct GamepadDisconnectedEvent {
  std::shared_ptr<IGamepad> gamepad;
};

struct GamepadOrderChangedEvent {};

struct GamepadInputEvent {
  int playerIndex;
  GamepadInput input;
  bool pressed;
};

class InputService : public libretro::IRetropadProvider {
public:
  static void setInstance(InputService *instance) { s_instance = instance; }
  static InputService *instance() { return s_instance; }

  ~InputService() override = default;
  virtual std::vector<std::shared_ptr<IGamepad>> listGamepads() = 0;
  virtual std::shared_ptr<IGamepad> getPlayerGamepad(int playerNumber) = 0;

  virtual std::shared_ptr<GamepadProfile> getProfile(int id) = 0;

  virtual void changeGamepadOrder(const std::map<int, int> &oldToNewIndex) = 0;

private:
  static InputService *s_instance;
};

} // namespace firelight::input

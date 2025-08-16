#pragma once

#include "gamepad.hpp"
#include <firelight/libretro/retropad_provider.hpp>
#include <memory>

namespace firelight::input {

struct GamepadConnectedEvent {
  std::shared_ptr<IGamepad> gamepad;
};

struct GamepadDisconnectedEvent {
  int playerIndex;
};

struct GamepadOrderChangedEvent {};

struct GamepadInputEvent {
  int playerIndex;
  GamepadInput input;
  bool pressed;
};

struct ShortcutToggledEvent {
  int playerIndex;
  Shortcut shortcut;
};

class InputService : public libretro::IRetropadProvider {
public:
  static std::map<Shortcut, std::string> listShortcuts() {
    return {{OpenRewindMenu, "Open rewind menu"},
            {SpeedUp, "Speed up"},
            {SlowDown, "Slow down"}};
  }

  static void setInstance(InputService *instance) { s_instance = instance; }
  static InputService *instance() { return s_instance; }

  ~InputService() override = default;
  virtual int addGamepad(std::shared_ptr<IGamepad> gamepad) = 0;

  virtual bool removeGamepadByInstanceId(int instanceId) = 0;
  virtual bool removeGamepadByPlayerIndex(int playerIndex) = 0;

  virtual std::vector<std::shared_ptr<IGamepad>> listGamepads() = 0;
  virtual std::shared_ptr<IGamepad> getPlayerGamepad(int playerIndex) = 0;

  virtual std::shared_ptr<GamepadProfile> getProfile(int id) = 0;

  virtual void changeGamepadOrder(const std::map<int, int> &oldToNewIndex) = 0;

  virtual bool preferGamepadOverKeyboard() const = 0;
  virtual void setPreferGamepadOverKeyboard(bool prefer) = 0;

private:
  static InputService *s_instance;
};

} // namespace firelight::input

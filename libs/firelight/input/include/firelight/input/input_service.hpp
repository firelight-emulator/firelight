#pragma once

#include <firelight/input/igamepad.hpp>
#include <firelight/libretro/pointer_input_provider.hpp>
#include <firelight/libretro/retropad_provider.hpp>
#include <memory>
#include <optional>
#include <string>

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
  bool autoRepeat = false;
};

// Raw keyboard key transition, published by the keyboard handler so the shortcut
// engine can treat the keyboard like any other device.
struct KeyboardKeyEvent {
  int playerIndex;
  int key; // Qt::Key
  bool pressed;
};

class InputService : public libretro::IRetropadProvider,
                     public libretro::IPointerInputProvider {
public:
  ~InputService() override = default;
  virtual int addGamepad(std::shared_ptr<IGamepad> gamepad) = 0;

  virtual bool removeGamepadByInstanceId(int instanceId) = 0;
  virtual bool removeGamepadByPlayerIndex(int playerIndex) = 0;

  virtual std::vector<std::shared_ptr<IGamepad>> listGamepads() = 0;
  virtual std::shared_ptr<IGamepad> getPlayerGamepad(int playerIndex) = 0;

  virtual void changeGamepadOrder(const std::map<int, int> &oldToNewIndex) = 0;

  virtual bool preferGamepadOverKeyboard() const = 0;
  virtual void setPreferGamepadOverKeyboard(bool prefer) = 0;

  virtual void updateMouseState(double x, double y, bool mousePressed) = 0;
  virtual void updateMousePressed(bool mousePressed) = 0;

  // Applies the input context for a launched game: any per-game profile
  // override (by content hash) and the platform's preferred controller type
  // (promoting a matching controller to player 1). Pass an empty hash to skip
  // the override lookup.
  virtual void applyGameContext(std::optional<std::string> contentHash,
                                int platformId) = 0;
  // Clears the per-game override, restoring device-default profiles. Player
  // slot order is left as-is.
  virtual void clearGameContext() = 0;

  // Sets which shortcuts are currently active (ScopeInGame / ScopeInMenu).
  virtual void setShortcutContext(int scope) = 0;
};

} // namespace firelight::input

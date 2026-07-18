#pragma once

#include <firelight/event_dispatcher.hpp>
#include <firelight/input/controller_repository.hpp>
#include <firelight/input/input_service.hpp>
#include <firelight/input/sdl_controller.hpp>
#include <firelight/input/shortcut_engine.hpp>
#include <SDL.h>
#include <SDL_gamecontroller.h>
#include <atomic>
#include <functional>
#include <mutex>
#include <shared_mutex>

namespace firelight::input {

const static std::map<int, GamepadInput> sdlToGamepadInputs = {
    {SDL_CONTROLLER_BUTTON_A, SouthFace},
    {SDL_CONTROLLER_BUTTON_B, EastFace},
    {SDL_CONTROLLER_BUTTON_X, WestFace},
    {SDL_CONTROLLER_BUTTON_Y, NorthFace},
    {SDL_CONTROLLER_BUTTON_DPAD_UP, DpadUp},
    {SDL_CONTROLLER_BUTTON_DPAD_DOWN, DpadDown},
    {SDL_CONTROLLER_BUTTON_DPAD_LEFT, DpadLeft},
    {SDL_CONTROLLER_BUTTON_DPAD_RIGHT, DpadRight},
    {SDL_CONTROLLER_BUTTON_START, Start},
    {SDL_CONTROLLER_BUTTON_BACK, Select},
    {SDL_CONTROLLER_BUTTON_LEFTSHOULDER, LeftBumper},
    {SDL_CONTROLLER_BUTTON_RIGHTSHOULDER, RightBumper},
    {SDL_CONTROLLER_BUTTON_LEFTSTICK, L3},
    {SDL_CONTROLLER_BUTTON_RIGHTSTICK, R3},
    {SDL_CONTROLLER_BUTTON_GUIDE, Home}};

// TODO
// Threading: run() is a blocking SDL event loop on its own thread (device
// add/remove, button/axis updates). The emulation/render thread reads pad and
// pointer state each frame, and the GUI thread pushes mouse updates — shared
// state is guarded by the mutexes/atomics below
class SDLInputService final : public InputService {
public:
  explicit SDLInputService(IControllerRepository &gamepadRepository);
  ~SDLInputService() override;

  int addGamepad(std::shared_ptr<IGamepad> gamepad) override;
  bool removeGamepadByInstanceId(int instanceId) override;
  bool removeGamepadByPlayerIndex(int playerIndex) override;
  std::vector<std::shared_ptr<IGamepad>> listGamepads() override;
  std::shared_ptr<IGamepad> getPlayerGamepad(int playerIndex) override;

  std::shared_ptr<libretro::IRetroPad>
  getRetropadForPlayerIndex(int t_player) override;

  [[nodiscard]] std::pair<int16_t, int16_t> getPointerPosition() const override;
  [[nodiscard]] bool isPressed() const override;
  [[nodiscard]] std::pair<int16_t, int16_t> getRelativeMotion() override;
  [[nodiscard]] bool isMouseButtonPressed(int retroMouseButtonId) const override;
  [[nodiscard]] bool isPointerOffscreen() const override;
  void nudgeCursor(int dx, int dy) override;
  void updateMouseState(double x, double y, bool mousePressed) override;
  void updateMousePressed(bool mousePressed) override;
  void updateMouseButtons(bool left, bool right, bool middle) override;
  void updateMouseMotion(int dx, int dy) override;
  void updateMouseOffscreen(bool offscreen) override;

  void run();
  void stop();

  // Applies one SDL axis event to the device with this instance id: decodes it
  // to digital directions and records each edge
  void handleAxisMotion(int instanceId, int sdlAxis, int value);

  // TODO
  // The digital directions one SDL axis event implies, as (input, pressed)
  // pairs. Both directions of a stick axis are always returned, so a stick
  // flicked straight from one extreme to the other clears the direction it
  // left instead of leaving it stuck on
  static std::vector<std::pair<GamepadInput, bool>>
  decodeAxisMotion(int sdlAxis, int value);

  void changeGamepadOrder(const std::map<int, int> &oldToNewIndex) override;

  bool preferGamepadOverKeyboard() const override;
  void setPreferGamepadOverKeyboard(bool prefer) override;

  void applyGameContext(std::optional<std::string> contentHash,
                        int platformId) override;
  void clearGameContext() override;

  void setSessionPreferredControllerType(int platformId,
                                         int gamepadType) override;

  void setShortcutContext(int scope) override;
  void setHotkeysEnabled(bool enabled,
                         std::optional<DeviceType> only = {}) override;

  void setKeyboard(std::shared_ptr<IGamepad> keyboard);

private:
  static constexpr int MAX_PLAYERS = 16;

  void openSdlGamepad(int deviceIndex);
  // TODO
  // Records a digital edge for one gamepad input: feeds the shortcut engine and
  // publishes the nav event. Repeats are dropped, so callers can pass the
  // current state unconditionally rather than edge-detecting themselves
  void setGamepadInputState(int playerIndex, IGamepad *gamepad,
                            GamepadInput input, bool pressed);
  // Locks m_devicesMutex internally; safe to call without holding it
  std::shared_ptr<IGamepad> findGamepadByInstanceId(int instanceId);
  int getNextAvailablePlayerIndex() const;
  bool moveGamepadToPlayerIndex(int oldIndex, int newIndex);
  // Resolves the profile a gamepad should use: the active per-game override if
  // any, otherwise the device's stored default (creating one if needed)
  std::shared_ptr<GamepadProfile>
  resolveProfileForGamepad(const std::shared_ptr<IGamepad> &gamepad);
  // Places a connected device into a player slot, honoring the
  // prefer-gamepad-over-keyboard rule. Shared by gamepads and the keyboard
  void assignPlayerSlot(const std::shared_ptr<IGamepad> &gamepad);
  void assignToSlot(int slot, const std::shared_ptr<IGamepad> &gamepad);
  void publishConnected(const std::shared_ptr<IGamepad> &gamepad);
  void publishDisconnected(int playerIndex);
  // Re-resolves every connected gamepad's profile (used when the game context
  // changes). Moves a device into player one, displacing the current occupant
  void reapplyDeviceProfiles();
  // Returns true if the slot order actually changed (caller publishes the event
  // after releasing the lock)
  bool promoteDeviceToPlayerOne(const std::shared_ptr<IGamepad> &gamepad);

  IControllerRepository &m_gamepadRepository;

  // TODO
  // Guards the device collections below. run() mutates them from the SDL event
  // thread while the render/UI threads read them; every access goes through
  // this. It is a read-write lock: the hot readers (getPlayerGamepad /
  // getRetropadForPlayerIndex every frame, listGamepads) take a shared lock and
  // run concurrently; mutators take a unique lock. Connect/disconnect/order
  // events are published *outside* the lock because subscribers (e.g.
  // ControllerListModel) call back in to read slots
  std::shared_mutex m_devicesMutex;
  std::optional<int> m_gameProfileOverride;
  // CLI `--controller`: platformId -> gamepad type, wins over the stored
  // platform preference in applyGameContext. Guarded by m_devicesMutex
  std::map<int, int> m_sessionPreferredTypes;
  std::vector<std::shared_ptr<IGamepad>> m_gamepads;
  std::map<int, std::shared_ptr<IGamepad>> m_playerSlots;

  std::shared_ptr<IGamepad> m_keyboard;

  ShortcutEngine m_shortcutEngine;
  ScopedConnection m_keyboardKeyConnection;

  std::map<int, std::map<GamepadInput, bool>> m_gamepadLastStates;

  int m_sdlServices = SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC;
  std::atomic<bool> m_running{true};

  bool m_preferGamepadOverKeyboard = true;

  // TODO
  // Cursor position (±32767). Written absolutely by the UI thread on mouse
  // events and incrementally by the emulation thread's analog-stick glide
  // (nudgeCursor), read from the render thread — atomic so no lock is needed
  std::atomic<int16_t> m_mouseX{0};
  std::atomic<int16_t> m_mouseY{0};
  std::atomic<bool> m_mousePressed{false};
  // TODO
  // Right/middle buttons, relative motion, and off-screen state for
  // RETRO_DEVICE_MOUSE and the light gun. Written from the UI thread on mouse
  // events, read from the render thread via the pointer provider — atomic so
  // no lock is needed. Relative motion accumulates and is consumed per frame
  std::atomic<bool> m_mouseRightPressed{false};
  std::atomic<bool> m_mouseMiddlePressed{false};
  std::atomic<int> m_mouseRelX{0};
  std::atomic<int> m_mouseRelY{0};
  std::atomic<bool> m_mouseOffscreen{false};
};

} // namespace firelight::input

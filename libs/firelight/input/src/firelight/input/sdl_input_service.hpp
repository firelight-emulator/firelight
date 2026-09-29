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

const static std::map<int, GamepadInput> sdlToGamepadInputs = {{SDL_CONTROLLER_BUTTON_A, SouthFace},
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

// Run on its own thread to handle SDL events
class SDLInputService final : public InputService {
public:
  explicit SDLInputService(IControllerRepository &gamepadRepository);
  ~SDLInputService() override;

  int addGamepad(std::shared_ptr<IGamepad> gamepad) override;
  bool removeGamepadByInstanceId(int instanceId) override;
  bool removeGamepadByPlayerIndex(int playerIndex) override;
  std::vector<std::shared_ptr<IGamepad>> listGamepads() override;
  std::shared_ptr<IGamepad> getPlayerGamepad(int playerIndex) override;

  std::shared_ptr<libretro::IRetroPad> getRetropadForPlayerIndex(int t_player) override;

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

  /**
   * Decodes an axis motion event to digital directions and records each edge
   */
  void handleAxisMotion(int instanceId, int sdlAxis, int value);

  /**
   * The digital directions one SDL axis event implies, as (input, pressed) pairs. Both directions of a stick axis are
   * always returned, so a stick flicked straight from one extreme to the other clears the direction it left instead
   * of leaving it stuck on
   */
  static std::vector<std::pair<GamepadInput, bool>> decodeAxisMotion(int sdlAxis, int value);

  void changeGamepadOrder(const std::map<int, int> &oldToNewIndex) override;
  void swapGamepads(int firstIndex, int secondIndex) override;
  void moveGamepad(int from, int to) override;

  bool preferGamepadOverKeyboard() const override;
  void setPreferGamepadOverKeyboard(bool prefer) override;

  void applyGameContext(std::optional<std::string> contentHash, int platformId) override;
  void clearGameContext() override;

  void setSessionPreferredControllerType(int platformId, int gamepadType) override;

  void setShortcutContext(int scope) override;
  void setHotkeysEnabled(bool enabled, std::optional<DeviceType> only = {}) override;

  /** Scales rumble on the connected controllers using this profile */
  void setRumbleScale(int profileId, int percent) override;

  /** Sets the light color on the connected controllers using this profile */
  void setLightColor(int profileId, std::optional<uint32_t> rgb) override;

  void setKeyboard(std::shared_ptr<IGamepad> keyboard);

private:
  static constexpr int MAX_PLAYERS = 16;

  void openSdlGamepad(int deviceIndex);

  // Records a digital edge for one gamepad input: feeds the shortcut engine and publishes the nav event. Repeats are
  // dropped, so callers can pass the current state unconditionally rather than edge-detecting themselves
  void setGamepadInputState(int playerIndex, IGamepad *gamepad, GamepadInput input, bool pressed);

  std::shared_ptr<IGamepad> findGamepadByInstanceId(int instanceId);
  int getNextAvailablePlayerIndex() const;
  bool moveGamepadToPlayerIndex(int oldIndex, int newIndex);

  // Rebuilds the player slots from an old-to-new index map. A slot the map does not name ends up empty
  // The device mutex should be held by the caller
  void applyPlayerOrder(const std::map<int, int> &oldToNewIndex);

  // Resolves the profile a gamepad should use: the active per-game override if any, otherwise the device's stored
  // default (creating one if needed)
  std::shared_ptr<GamepadProfile> resolveProfileForGamepad(const std::shared_ptr<IGamepad> &gamepad);

  // Places a connected device into a player slot, honoring the prefer-gamepad-over-keyboard rule
  void assignPlayerSlot(const std::shared_ptr<IGamepad> &gamepad);
  void assignToSlot(int slot, const std::shared_ptr<IGamepad> &gamepad);
  void publishConnected(const std::shared_ptr<IGamepad> &gamepad);
  void publishDisconnected(int playerIndex);

  // Re-resolves every connected gamepad's profile (used when the game context changes). Moves a device into player
  // one, displacing the current occupant
  void reapplyDeviceProfiles();

  /**
   * @return true if the gamepad moved
   */
  bool promoteDeviceToPlayerOne(const std::shared_ptr<IGamepad> &gamepad);

  /** Calls apply on each connected controller using this profile, holding m_devicesMutex shared throughout */
  void forEachGamepadUsingProfile(int profileId, const std::function<void(IGamepad &)> &apply);

  IControllerRepository &m_gamepadRepository;

  // Mutex for manipulating gamepads and player slots, events emitted outside of lock
  std::shared_mutex m_devicesMutex;

  std::optional<int> m_gameProfileOverride;
  std::map<int, int> m_sessionPreferredTypes; // platformId -> gamepad type for CLI session override
  std::vector<std::shared_ptr<IGamepad>> m_gamepads;
  std::map<int, std::shared_ptr<IGamepad>> m_playerSlots;

  std::shared_ptr<IGamepad> m_keyboard;

  ShortcutEngine m_shortcutEngine;
  ScopedConnection m_keyboardKeyConnection;

  std::map<int, std::map<GamepadInput, bool>> m_gamepadLastStates;

  int m_sdlServices = SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC;
  std::atomic<bool> m_running{true};

  std::atomic<bool> m_preferGamepadOverKeyboard{true};

  std::atomic<int16_t> m_mouseX{0}; // Absolute mouse X position
  std::atomic<int16_t> m_mouseY{0}; // Absolute mouse Y position
  std::atomic<bool> m_mousePressed{false};
  std::atomic<bool> m_mouseRightPressed{false};
  std::atomic<bool> m_mouseMiddlePressed{false};
  std::atomic<int> m_mouseRelX{0}; // Relative motion since last consumed, consumed per frame
  std::atomic<int> m_mouseRelY{0}; // Relative motion since last consumed, consumed per frame
  std::atomic<bool> m_mouseOffscreen{false};
};

} // namespace firelight::input

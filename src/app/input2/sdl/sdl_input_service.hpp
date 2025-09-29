#pragma once

#include "../input_service.hpp"

#include "sdl_controller.hpp"
#include <SDL.h>
#include <SDL_gamecontroller.h>
#include <functional>
#include <input/controller_repository.hpp>

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

class SDLInputService final : public InputService {
public:
  explicit SDLInputService(IControllerRepository &gamepadRepository);
  ~SDLInputService() override;

  int addGamepad(std::shared_ptr<IGamepad> gamepad) override;
  bool removeGamepadByInstanceId(int instanceId) override;
  bool removeGamepadByPlayerIndex(int playerIndex) override;
  std::vector<std::shared_ptr<IGamepad>> listGamepads() override;
  std::shared_ptr<IGamepad> getPlayerGamepad(int playerIndex) override;

  std::shared_ptr<GamepadProfile> getProfile(int id) override;

  std::optional<libretro::IRetroPad *>
  getRetropadForPlayerIndex(int t_player) override;

  [[nodiscard]] std::pair<int16_t, int16_t> getPointerPosition() const override;
  [[nodiscard]] bool isPressed() const override;
  void updateMouseState(double x, double y, bool mousePressed) override;
  void updateMousePressed(bool mousePressed) override;

  void run();
  void stop();

  void changeGamepadOrder(const std::map<int, int> &oldToNewIndex) override;

  bool preferGamepadOverKeyboard() const override;
  void setPreferGamepadOverKeyboard(bool prefer) override;

  void setKeyboard(std::shared_ptr<IGamepad> keyboard);

private:
  static constexpr int MAX_PLAYERS = 16;

  void openSdlGamepad(int deviceIndex);
  int getNextAvailablePlayerIndex() const;
  bool moveGamepadToPlayerIndex(int oldIndex, int newIndex);

  IControllerRepository &m_gamepadRepository;
  std::vector<std::shared_ptr<IGamepad>> m_gamepads;
  std::map<int, std::shared_ptr<IGamepad>> m_playerSlots;

  std::shared_ptr<IGamepad> m_keyboard;

  std::map<int, std::map<GamepadInput, bool>> m_gamepadLastStates;

  int m_sdlServices = SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC;
  bool m_running = true;

  bool m_preferGamepadOverKeyboard = true;

  int16_t m_mouseX;
  int16_t m_mouseY;
  bool m_mousePressed = false;
};

} // namespace firelight::input
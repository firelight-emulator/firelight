//
// Created by alexs on 11/17/2023.
//

#ifndef FIRELIGHT_GAME_SCREEN_HPP
#define FIRELIGHT_GAME_SCREEN_HPP

#include "../lib/gui/screens/screen.hpp"
#include "controller/controller_manager.hpp"
#include "library/entry.hpp"
#include "library/game_library.hpp"
#include "libretro/core.hpp"
#include "saves/save_manager.hpp"
#include <filesystem>
namespace FL::GUI {

class GameScreen : public Screen {
public:
  explicit GameScreen(std::unique_ptr<ContainerWidget> container,
                      FL::Input::ControllerManager *manager,
                      FL::Graphics::Driver *driver, std::string romPath,
                      FL::Library::GameLibrary *library,
                      Library::Entry libEntry, SaveManager *saveMan);
  void enter() override;
  void exit() override;
  void update(float deltaTime) override;
  void render(const std::shared_ptr<WidgetPainter> &painter) override;
  ~GameScreen() override = default;
  bool handleEvent(Event &event) override;
  void setWindowArea(int width, int height) override;
  void forceStop() override;

private:
  std::unique_ptr<libretro::Core> core;
  std::string gameRomPath;
  FL::Input::ControllerManager *controllerManager;
  FL::Graphics::Driver *gfxDriver;
  FL::Library::GameLibrary *gameLibrary;
  Library::Entry libraryEntry;
  SaveManager *saveManager;
  float millisSinceLastSave = 0;
};

} // namespace FL::GUI

#endif // FIRELIGHT_GAME_SCREEN_HPP

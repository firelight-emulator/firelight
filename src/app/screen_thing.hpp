//
// Created by alexs on 11/14/2023.
//

#ifndef FIRELIGHT_SCREEN_THING_HPP
#define FIRELIGHT_SCREEN_THING_HPP

#include "../lib/gui/screens/screen.hpp"
#include "../lib/gui/screens/screen_manager.hpp"
#include "../lib/gui/vertical_box_layout_manager.hpp"
#include "../lib/gui/widget_factory.hpp"
#include "controller/controller_manager.hpp"
#include "game_screen.hpp"
#include "gui/themes.hpp"
#include "library/entry.hpp"
#include "library/game_library.hpp"
#include "saves/save_manager.hpp"
#include <memory>
namespace FL::GUI {

class ScreenThing {
private:
  std::unordered_map<std::string, std::unique_ptr<Screen>> screenCache;
  ScreenManager *screenManager;
  WidgetFactory *widgetFactory;
  FL::Input::ControllerManager *controllerManager;
  SaveManager *saveManager;
  Graphics::Driver *gfxDriver;

public:
  ScreenThing(ScreenManager *manager, WidgetFactory *factory);

  std::unique_ptr<Screen> getInitialScreen() {
    //    return std::move(screenCache["home"]);
    return std::move(screenCache["home"]);
  }

  void buildHomeScreen(FL::Library::GameLibrary *library) {
    auto container = std::make_unique<ContainerWidget>();
    container->box.xPx = 80;
    container->box.yPx = 100;
    container->box.widthPx = 1100;
    container->box.heightPx = 620;

    container->setLayoutManager(
        std::make_unique<VerticalBoxLayoutManager>(60, 0));

    auto screen = std::make_unique<Screen>(std::move(container));

    for (const auto &e : library->getAllGames()) {
      auto button = widgetFactory->createButton(e.gameName);
      button->onPressed.connect([e, library, this](Button *button) {
        auto container = std::make_unique<ContainerWidget>();
        container->box.xPx = 0;
        container->box.yPx = 0;
        container->box.widthPx = 1280;
        container->box.heightPx = 720;

        container->setLayoutManager(
            std::make_unique<VerticalBoxLayoutManager>(30, 10));

        auto screen = std::make_unique<GameScreen>(
            std::move(container), controllerManager, gfxDriver,
            e.romPath.generic_string(), library, e, saveManager);

        this->screenManager->pushScreen(std::move(screen));
      });

      screen->addWidget(std::move(button));
    }

    screenCache.emplace("home", std::move(screen));
  }

  void buildGameScreen(FL::Input::ControllerManager *manager,
                       FL::Graphics::Driver *driver, FL::SaveManager *saveMan) {
    this->controllerManager = manager;
    this->gfxDriver = driver;
    this->saveManager = saveMan;
    //
    //
    //
    //    screenCache.emplace("game", std::move(screen));
  }

  void buildQuickMenu() {
    auto container = std::make_unique<ContainerWidget>();
    container->box.xPx = 0;
    container->box.yPx = 0;
    container->box.widthPx = 1280;
    container->box.heightPx = 720;

    container->setLayoutManager(
        std::make_unique<VerticalBoxLayoutManager>(30, 10));

    auto screen = std::make_unique<Screen>(std::move(container));

    auto button = widgetFactory->createButton("resume");

    button->onPressed.connect(
        [this](Button *button) { this->screenManager->popScreen(); });

    screen->addWidget(std::move(button));

    screenCache.emplace("quick", std::move(screen));
  }
};

} // namespace FL::GUI

#endif // FIRELIGHT_SCREEN_THING_HPP

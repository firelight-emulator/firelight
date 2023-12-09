//
// Created by alexs on 11/17/2023.
//

#include "game_screen.hpp"
#include "patching/pm_star_rod_mod_patch.hpp"
#include "patching/yay_0_codec.hpp"

#include <fstream>
#include <iterator>
#include <utility>

namespace FL::GUI {
GameScreen::GameScreen(std::unique_ptr<ContainerWidget> container,
                       FL::Input::ControllerManager *manager,
                       FL::Graphics::Driver *driver, std::string romPath,
                       FL::Library::GameLibrary *library,
                       Library::Entry libEntry, SaveManager *saveMan)
    : Screen(std::move(container)), gameRomPath(std::move(romPath)),
      controllerManager(manager), gfxDriver(driver), gameLibrary(library),
      libraryEntry(std::move(libEntry)), saveManager(saveMan) {}

bool GameScreen::handleEvent(Event &event) {
  if (event.type == FL::GUI::Event::TEST) {
    auto data = core->getMemoryData(libretro::SAVE_RAM);
    saveManager->write(libraryEntry.gameId, data);
    return true;
  }

  return Screen::handleEvent(event);
}

void GameScreen::enter() {
  core = std::make_unique<libretro::Core>(libraryEntry.corePath, gfxDriver,
                                          controllerManager);

  core->setSystemDirectory(".");
  core->setSaveDirectory(".");
  core->init();

  if (libraryEntry.type == Library::ROMHACK) {
    auto sourceId = libraryEntry.sourceGameId;
    if (sourceId.empty()) {
      printf("whaaaaa\n");
      return;
    }

    auto gameEntry = gameLibrary->getEntryByGameId(sourceId);
    printf("going to use %s to patch %s\n", libraryEntry.gameName.c_str(),
           gameEntry->gameName.c_str());

    std::filesystem::path t = libraryEntry.romPath;
    auto size = file_size(t);
    auto data = new char[size];

    std::ifstream file(t, std::ios::binary);

    file.read(data, size);

    FL::Patching::Yay0Codec codec;

    auto result = codec.decompress(reinterpret_cast<uint8_t *>(data));

    FL::Patching::PMStarRodModPatch patch(result);

    std::filesystem::path gamePath = gameEntry->romPath;
    std::ifstream game(gamePath);

    auto gameSize = file_size(gamePath);
    std::vector<uint8_t> gameData(gameSize);

    game.read(reinterpret_cast<char *>(gameData.data()), gameSize);

    auto patchedGame = patch.patchRom(gameData);
    libretro::Game lrGame(patchedGame);

    core->loadGame(&lrGame);
    core->getVideo()->initialize(0, 0, 1280, 720);

    auto saveData = saveManager->read(libraryEntry.gameId);
    if (!saveData.empty()) {
      core->writeMemoryData(libretro::SAVE_RAM, saveData.data());
    }

    return;
  }

  libretro::Game game(gameRomPath);
  core->loadGame(&game);
  core->getVideo()->initialize(0, 0, 1280, 720);

  auto saveData = saveManager->read(libraryEntry.gameId);
  if (!saveData.empty()) {
    core->writeMemoryData(libretro::SAVE_RAM, saveData.data());
  }
}

void GameScreen::exit() { Screen::exit(); }
void GameScreen::update(float deltaTime) {
  millisSinceLastSave += deltaTime;

  if (millisSinceLastSave >= 5000) {
    millisSinceLastSave = 0;

    // TODO: Need to get IO out of the hot path
    auto data = core->getMemoryData(libretro::SAVE_RAM);
    saveManager->write(libraryEntry.gameId, data);
  }

  core->run(deltaTime);
}
void GameScreen::forceStop() {
  auto data = core->getMemoryData(libretro::SAVE_RAM);
  saveManager->write(libraryEntry.gameId, data);
}

void GameScreen::render(const std::shared_ptr<WidgetPainter> &painter) {
  core->getVideo()->draw();
}
void GameScreen::setWindowArea(int width, int height) {
  Screen::setWindowArea(width, height);
  if (core) {
    core->getVideo()->setScreenDimensions(0, 0, width, height);
  }
}

} // namespace FL::GUI

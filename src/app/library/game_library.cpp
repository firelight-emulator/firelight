//
// Created by alexs on 11/18/2023.
//

#include "game_library.hpp"
#include "../db/game_scanner.hpp"
#include <fstream>

namespace FL::Library {
Entry *GameLibrary::getEntryByGameId(const std::string &gameId) {
  for (auto &g : allGames) {
    if (g.gameId == gameId) {
      return &g;
    }
  }

  return nullptr;
}
void GameLibrary::addGame(Entry &entry) {
  for (auto &game : allGames) {
    if (game.gameId == entry.gameId) {
      return;
    }
  }
  allGames.emplace_back(entry);
}
std::vector<Entry> GameLibrary::getAllGames() { return allGames; }

GameLibrary::GameLibrary(std::filesystem::path libFile,
                         FL::DB::GameRepository *gameRepo)
    : libFile(libFile), gameRepository(gameRepo) {
  scanner = new FL::DB::GameScanner(gameRepo);
  std::filesystem::path path(libFile);
  if (!exists(path)) {
    std::filesystem::create_directories(libFile.parent_path());
    return;
  }

  std::ifstream file(path, std::ios::binary);

  auto size = file_size(path);
  char content[size + 1];

  file.read(content, size);
  file.close();

  content[size] = 0;

  std::string contentString(content);
  auto j = json::parse(contentString);

  auto p2 = j.template get<std::vector<FL::Library::Entry>>();
  for (auto &p : p2) {
    addGame(p);
  }
}

void GameLibrary::scanNow() {
  auto results = scanner->scanDirectory("./roms", true);
  for (auto r : results) {
    addGame(r);
  }
}

} // namespace FL::Library

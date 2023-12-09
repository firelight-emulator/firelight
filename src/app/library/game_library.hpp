//
// Created by alexs on 11/18/2023.
//

#ifndef FIRELIGHT_GAME_LIBRARY_HPP
#define FIRELIGHT_GAME_LIBRARY_HPP

#include "../db/game_repository.hpp"
#include "../db/game_scanner.hpp"
#include "entry.hpp"
#include "nlohmann/json.hpp"
#include <vector>
using namespace nlohmann;

namespace FL::Library {

class GameLibrary {
public:
  GameLibrary(std::filesystem::path libFile, FL::DB::GameRepository *gameRepo);
  void addGame(Entry &entry);
  Entry *getEntryByGameId(const std::string &gameId);
  std::vector<Entry> getAllGames();
  void scanNow();

private:
  FL::DB::GameScanner *scanner;
  FL::DB::GameRepository *gameRepository;
  std::filesystem::path libFile;
  std::vector<Entry> allGames;
};

} // namespace FL::Library

#endif // FIRELIGHT_GAME_LIBRARY_HPP

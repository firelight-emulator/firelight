//
// Created by alexs on 11/14/2023.
//

#ifndef FIRELIGHT_GAME_SCANNER_HPP
#define FIRELIGHT_GAME_SCANNER_HPP

#include "../library/entry.hpp"
#include "game_repository.hpp"
#include <string>

namespace FL::DB {

class GameScanner {
public:
  explicit GameScanner(GameRepository *repository);
  std::vector<FL::Library::Entry> scanDirectory(const std::string &path,
                                                bool recursive);

private:
  GameRepository *gameRepository;
};

} // namespace FL::DB

#endif // FIRELIGHT_GAME_SCANNER_HPP

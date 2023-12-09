//
// Created by alexs on 11/18/2023.
//

#ifndef FIRELIGHT_SAVE_MANAGER_HPP
#define FIRELIGHT_SAVE_MANAGER_HPP

#include "../db/game_repository.hpp"
#include <string>
#include <vector>
namespace FL {

class SaveManager {
public:
  std::vector<char> read(const std::string &gameId);
  void write(const std::string &gameId, std::vector<char> data);

private:
  std::string profileId = "defaultuser";
  DB::GameRepository *gameRepository;

  std::filesystem::path getSaveFilePath(const std::string &gameId);
  std::filesystem::path getSaveFileDirectory();
};

} // namespace FL

#endif // FIRELIGHT_SAVE_MANAGER_HPP

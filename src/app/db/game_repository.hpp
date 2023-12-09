//
// Created by alexs on 11/14/2023.
//

#ifndef FIRELIGHT_GAME_REPOSITORY_HPP
#define FIRELIGHT_GAME_REPOSITORY_HPP

#include "game_record.hpp"
#include "romhack_record.hpp"
#include <memory>
#include <string>

namespace FL::DB {

class GameRepository {
public:
  virtual std::shared_ptr<ROM> getGameByChecksum(std::string checksum) = 0;
  virtual std::shared_ptr<ROM> getGameById(std::string id) = 0;
  virtual std::shared_ptr<RomhackRecord>
  getRomhackByChecksum(std::string checksum) = 0;
};

} // namespace FL::DB

#endif // FIRELIGHT_GAME_REPOSITORY_HPP

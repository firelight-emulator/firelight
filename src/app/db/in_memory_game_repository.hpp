//
// Created by alexs on 11/15/2023.
//

#ifndef FIRELIGHT_IN_MEMORY_GAME_REPOSITORY_HPP
#define FIRELIGHT_IN_MEMORY_GAME_REPOSITORY_HPP

#include "game_repository.hpp"
#include "romhack_record.hpp"
namespace FL::DB {

class InMemoryGameRepository : public GameRepository {
public:
  InMemoryGameRepository(const std::string &filename,
                         const string &romhackFile);
  std::shared_ptr<ROM> getGameByChecksum(std::string checksum) override;
  std::shared_ptr<ROM> getGameById(std::string id) override;
  std::shared_ptr<RomhackRecord>
  getRomhackByChecksum(std::string checksum) override;

private:
  std::unordered_map<std::string, ROM> games;
  std::vector<RomhackRecord> romhacks;
};

} // namespace FL::DB

#endif // FIRELIGHT_IN_MEMORY_GAME_REPOSITORY_HPP

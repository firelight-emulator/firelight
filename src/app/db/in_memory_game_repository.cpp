//
// Created by alexs on 11/15/2023.
//

#include "in_memory_game_repository.hpp"
#include "romhack_record.hpp"
#include <fstream>

namespace FL::DB {

InMemoryGameRepository::InMemoryGameRepository(const string &gameFile,
                                               const string &romhackFile) {
  std::filesystem::path path(gameFile);
  std::ifstream file(path, std::ios::binary);

  auto size = file_size(path);
  printf("file size: %ju\n", size);
  char content[size + 1];

  //  memset(content, 0, size);

  file.read(content, size);
  file.close();

  content[size] = 0;

  string contentString(content);
  auto j = json::parse(contentString);

  auto p2 = j.template get<std::vector<ROM>>();
  for (const auto &p : p2) {
    if (p.checksums.at("md5").empty()) {
      printf("empty: %s\n", p.gameName.c_str());
      continue;
    }
    games.emplace(p.checksums.at("md5"), p);
  }

  // romhacks

  std::filesystem::path romhackPath(romhackFile);
  std::ifstream romhackF(romhackPath, std::ios::binary);

  auto romhackSize = file_size(romhackPath);
  char romhackContent[romhackSize];

  memset(romhackContent, 0, romhackSize);

  romhackF.read(romhackContent, romhackSize);
  romhackF.close();

  string romhackContentString(romhackContent);
  auto romhackJson = json::parse(romhackContentString);

  auto romhackList = romhackJson.template get<std::vector<RomhackRecord>>();
  for (const auto &p : romhackList) {
    romhacks.emplace_back(p);
  }
}

std::shared_ptr<ROM>
InMemoryGameRepository::getGameByChecksum(string checksum) {
  auto result = games.find(checksum);
  if (result != nullptr) {
    return std::make_shared<ROM>(result->second);
  }

  return nullptr;
}

std::shared_ptr<ROM> InMemoryGameRepository::getGameById(std::string id) {
  for (const auto &record : games) {
    if (record.second.id == id) {
      return std::make_shared<ROM>(record.second);
    }
  }

  return nullptr;
}

std::shared_ptr<RomhackRecord>
InMemoryGameRepository::getRomhackByChecksum(std::string checksum) {
  for (const auto &romhack : romhacks) {
    for (const auto &version : romhack.versions) {
      if (version.md5_checksum == checksum) {
        return std::make_shared<RomhackRecord>(romhack);
      }
    }
  }

  return nullptr;
}

} // namespace FL::DB
//
// Created by alexs on 11/14/2023.
//

#ifndef FIRELIGHT_ENTRY_HPP
#define FIRELIGHT_ENTRY_HPP

#include "nlohmann/json.hpp"
#include <filesystem>
#include <string>
using namespace nlohmann;
namespace FL::Library {

enum EntryType { NORMAL_GAME, ROMHACK };

struct Entry {
  std::string id;
  EntryType type;
  std::string gameName;
  std::string gameId;       // todo: change name?
  std::string sourceGameId; // todo
  std::string corePath;
  std::filesystem::path romPath; // todo: change name
};

static void to_json(json &j, const Entry &entry) {
  j = json{{"id", entry.id},
           {"type", entry.type},
           {"game_name", entry.gameName},
           {"game_id", entry.gameId},
           {"source_game_id", entry.sourceGameId},
           {"core_path", entry.corePath},
           {"rom_path", entry.romPath}};
}

static void from_json(const json &j, Entry &entry) {
  j.at("id").get_to(entry.id);
  j.at("type").get_to(entry.type);
  j.at("game_name").get_to(entry.gameName);
  j.at("game_id").get_to(entry.gameId);
  j.at("source_game_id").get_to(entry.sourceGameId);
  j.at("core_path").get_to(entry.corePath);
  j.at("rom_path").get_to(entry.romPath);
}

} // namespace FL::Library

#endif // FIRELIGHT_ENTRY_HPP

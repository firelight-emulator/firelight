//
// Created by alexs on 11/14/2023.
//

#ifndef FIRELIGHT_GAME_RECORD_HPP
#define FIRELIGHT_GAME_RECORD_HPP

#include "nlohmann/json.hpp"
#include <string>

using std::string;
using namespace nlohmann;

namespace FL::DB {

struct ROM {
  string id;
  string region;
  string platform;
  std::map<string, string> checksums;
  std::vector<string> knownFilenames;
  int filesizeBytes = 0;
  std::string gameName;
  std::string gameId;
  //  std::map<string, string> externalIds;
};

static void to_json(json &j, const ROM &rom) {
  j = json{{"id", rom.id},
           {"region", rom.region},
           {"platform", rom.platform},
           {"checksums", rom.checksums},
           {"known_filenames", rom.knownFilenames},
           {"filesize_bytes", rom.filesizeBytes},
           {"game_name", rom.gameName},
           {"game_id", rom.gameId}};
}

static void from_json(const json &j, ROM &rom) {
  j.at("id").get_to(rom.id);
  j.at("region").get_to(rom.region);
  j.at("platform").get_to(rom.platform);
  j.at("checksums").get_to(rom.checksums);
  j.at("known_filenames").get_to(rom.knownFilenames);
  j.at("filesize_bytes").get_to(rom.filesizeBytes);
  j.at("game_name").get_to(rom.gameName);
  j.at("game_id").get_to(rom.gameId);
}

struct GameRecord {
  string id;
  string name;
  string platform;
  string md5_checksum;
  std::vector<ROM> versions;
  std::map<string, string> externalIds;
};

static void to_json(json &j, const GameRecord &record) {
  j = json{{"id", record.id},
           {"name", record.name},
           {"platform", record.platform},
           {"md5_checksum", record.md5_checksum},
           {"versions", record.versions},
           {"external_ids", record.externalIds}};
}

static void from_json(const json &j, GameRecord &record) {
  j.at("id").get_to(record.id);
  j.at("name").get_to(record.name);
  j.at("platform").get_to(record.platform);
  j.at("md5_checksum").get_to(record.md5_checksum);
  if (j.contains("versions")) {
    j.at("versions").get_to(record.versions);
    j.at("external_ids").get_to(record.externalIds);
  }
}

} // namespace FL::DB

#endif // FIRELIGHT_GAME_RECORD_HPP

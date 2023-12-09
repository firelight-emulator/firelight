//
// Created by alexs on 11/27/2023.
//

#ifndef FIRELIGHT_ROMHACK_RECORD_HPP
#define FIRELIGHT_ROMHACK_RECORD_HPP

#include "nlohmann/json.hpp"
#include "romhack_version_record.hpp"
#include <string>

using std::string;
using namespace nlohmann;

namespace FL::DB {

struct RomhackRecord {
  string id;
  string name;
  string platform;
  string sourceGameId;
  std::vector<RomhackVersionRecord> versions;
};

static void to_json(json &j, const RomhackRecord &record) {
  j = json{{"id", record.id},
           {"name", record.name},
           {"platform", record.platform},
           {"source_game_id", record.sourceGameId},
           {"versions", record.versions}};
}

static void from_json(const json &j, RomhackRecord &record) {
  j.at("id").get_to(record.id);
  j.at("name").get_to(record.name);
  j.at("platform").get_to(record.platform);
  j.at("source_game_id").get_to(record.sourceGameId);
  j.at("versions").get_to(record.versions);
}

} // namespace FL::DB

#endif // FIRELIGHT_ROMHACK_RECORD_HPP

//
// Created by alexs on 11/27/2023.
//

#ifndef FIRELIGHT_ROMHACK_VERSION_RECORD_HPP
#define FIRELIGHT_ROMHACK_VERSION_RECORD_HPP

#include "nlohmann/json.hpp"
#include <string>

using std::string;
using namespace nlohmann;

namespace FL::DB {

struct RomhackVersionRecord {
  std::string version;
  std::string md5_checksum;
};

static void to_json(json &j, const RomhackVersionRecord &record) {
  j = json{{"version", record.version}, {"md5_checksum", record.md5_checksum}};
}

static void from_json(const json &j, RomhackVersionRecord &record) {
  j.at("version").get_to(record.version);
  j.at("md5_checksum").get_to(record.md5_checksum);
}

} // namespace FL::DB

#endif // FIRELIGHT_ROMHACK_VERSION_RECORD_HPP

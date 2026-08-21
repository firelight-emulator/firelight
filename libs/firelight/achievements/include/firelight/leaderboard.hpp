#pragma once

#include <nlohmann/json.hpp>
#include <string>

namespace firelight::achievements {
struct Leaderboard {
  unsigned id;
  unsigned achievementSetId;
  std::string memAddr;
  std::string format;
  bool lowerIsBetter;
  std::string title;
  std::string description;
  bool hidden;
  int displayOrder;
};

inline void from_json(const nlohmann::json &j, Leaderboard &l) {
  j.at("ID").get_to(l.id);

  if (j.contains("Mem") && !j.at("Mem").is_null()) {
    j.at("Mem").get_to(l.memAddr);
  }

  if (j.contains("Format") && !j.at("Format").is_null()) {
    j.at("Format").get_to(l.format);
  }

  if (j.contains("LowerIsBetter") && !j.at("LowerIsBetter").is_null()) {
    j.at("LowerIsBetter").get_to(l.lowerIsBetter);
  }

  if (j.contains("Title") && !j.at("Title").is_null()) {
    j.at("Title").get_to(l.title);
  }

  if (j.contains("Description") && !j.at("Description").is_null()) {
    j.at("Description").get_to(l.description);
  }

  if (j.contains("Hidden") && !j.at("Hidden").is_null()) {
    j.at("Hidden").get_to(l.hidden);
  }
}

inline void to_json(nlohmann::json &j, const Leaderboard &l) {
  j = nlohmann::json{{"ID", l.id},         {"Mem", l.memAddr},
                     {"Format", l.format}, {"LowerIsBetter", l.lowerIsBetter},
                     {"Title", l.title},   {"Description", l.description},
                     {"Hidden", l.hidden}};
}
} // namespace firelight::achievements

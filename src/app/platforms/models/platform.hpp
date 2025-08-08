#pragma once
#include "controller_type.hpp"

#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <string>
#include <vector>

namespace firelight::platforms {

struct Platform {
  unsigned id = 0;
  std::string name;
  std::string abbreviation;
  std::string slug;
  std::vector<std::string> fileAssociations;
  std::vector<ControllerType> controllerTypes;
};

inline void from_json(const nlohmann::json &j, Platform &p) {
  j.at("id").get_to(p.id);
  j.at("name").get_to(p.name);
  j.at("abbreviation").get_to(p.abbreviation);
  j.at("slug").get_to(p.slug);
  j.at("file_associations").get_to(p.fileAssociations);
  j.at("controller_types").get_to(p.controllerTypes);
}

inline void to_json(nlohmann::json &j, const Platform &p) {
  j = nlohmann::json{{"id", p.id},
                     {"name", p.name},
                     {"abbreviation", p.abbreviation},
                     {"slug", p.slug},
                     {"file_associations", p.fileAssociations},
                     {"controller_types", p.controllerTypes}};
}

} // namespace firelight::platforms
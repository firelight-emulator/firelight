#pragma once
#include <firelight/platforms/controller_type.hpp>
#include <firelight/settings/emulation_setting.hpp>

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
  // rcheevos console id (RC_CONSOLE_*) this platform maps to; 0 if none.
  unsigned retroAchievementsId = 0;
  std::vector<std::string> fileAssociations;
  // Discord rich-presence large-image key; empty means fall back to `slug`.
  std::string discordImage;
  std::vector<ControllerType> controllerTypes;
  std::vector<settings::EmulationSetting> emulationSettings;

  // The Discord large-image key, defaulting to the slug when unset.
  [[nodiscard]] std::string discordImageOrSlug() const {
    return discordImage.empty() ? slug : discordImage;
  }
};

inline void from_json(const nlohmann::json &j, Platform &p) {
  j.at("id").get_to(p.id);
  j.at("name").get_to(p.name);
  j.at("abbreviation").get_to(p.abbreviation);
  j.at("slug").get_to(p.slug);
  p.retroAchievementsId = j.value("retro_achievements_id", 0u);
  j.at("file_associations").get_to(p.fileAssociations);
  p.discordImage = j.value("discord_image", std::string{});
  j.at("controller_types").get_to(p.controllerTypes);
}

inline void to_json(nlohmann::json &j, const Platform &p) {
  j = nlohmann::json{{"id", p.id},
                     {"name", p.name},
                     {"abbreviation", p.abbreviation},
                     {"slug", p.slug},
                     {"retro_achievements_id", p.retroAchievementsId},
                     {"file_associations", p.fileAssociations},
                     {"discord_image", p.discordImage},
                     {"controller_types", p.controllerTypes}};
}

} // namespace firelight::platforms

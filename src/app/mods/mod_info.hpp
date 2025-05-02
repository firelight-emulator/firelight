#pragma once
#include <string>
#include <vector>

namespace firelight::mods {
  struct ModInfo {
    int id;
    std::string name;
    std::string version;
    std::string author;
    std::string targetGameName;
    std::string targetContentHash;
    int platformId;
    std::string tagline;
    std::string description;
    std::string clearLogoUrl;
    std::vector<std::string> mediaUrls;
  };
}
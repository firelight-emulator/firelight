#pragma once

#include <string>

namespace firelight::shop {

struct Page {
    int id = -1;
    unsigned modId;
    std::string title;
    std::string tagline;
    std::string description;
    std::string capsuleImageUrl;
    int developmentStatus;
    int visibility;
    unsigned createdAt;
    unsigned lastUpdatedAt;
};

} // namespace firelight::shop
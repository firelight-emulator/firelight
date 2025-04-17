#pragma once

#include <string>

namespace firelight::achievements {
    struct Achievement {
        int id;
        std::string name;
        std::string description;
        std::string imageUrl;
        bool earnedHardcore;
        bool earned;
        std::string earnedDateHardcore;
        std::string earnedDate;
        int points;
        std::string type;
        int displayOrder;
    };
}

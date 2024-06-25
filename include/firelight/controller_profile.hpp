#pragma once

#include <string>

#include "input_mapping.hpp"

namespace firelight {
    struct ControllerProfile {
        int id = -1;
        std::string displayName;
        std::vector<InputMapping> inputMappings;
    };
}

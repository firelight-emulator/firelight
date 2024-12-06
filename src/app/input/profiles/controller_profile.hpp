#pragma once
#include "input_mapping.hpp"

namespace firelight::input {
    class ControllerProfile {
    public:
        std::optional<InputMapping> getControllerMappingForPlatform(int platformId);
    };
}

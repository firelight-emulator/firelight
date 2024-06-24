#pragma once
#include "input_mapping.hpp"

namespace firelight::input {
    class ControllerProfile {
    public:
        InputMapping getControllerMappingForPlatform(int platformId);
    };
}

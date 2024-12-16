#pragma once

#include <memory>

#include "input_mapping.hpp"

namespace firelight::input {
    class ControllerProfile {
    public:
        std::optional<InputMapping> getControllerMappingForPlatform(int platformId) const;

    private:
        int m_id = 0;
    };
}

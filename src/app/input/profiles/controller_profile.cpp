#include "controller_profile.hpp"

namespace firelight::input {
    std::optional<InputMapping> ControllerProfile::getControllerMappingForPlatform(int platformId) {
        static auto mapping = InputMapping();
        return {mapping};
    }
}

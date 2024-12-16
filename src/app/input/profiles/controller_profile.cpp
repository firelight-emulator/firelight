#include "controller_profile.hpp"

namespace firelight::input {
    std::optional<InputMapping> ControllerProfile::getControllerMappingForPlatform(const int platformId) const {
        // auto mapping = getControllerRepository()->getInputMapping(m_id, platformId);
        // if (!mapping) {
        //     return std::nullopt;
        // }
        //
        // return {*mapping};
        return std::nullopt;
    }
}

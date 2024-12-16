#pragma once

#include <vector>
#include "controller_info.hpp"
#include "profiles/controller_profile.hpp"
#include "profiles/input_mapping.hpp"

namespace firelight::input {
    class IControllerRepository {
    public:
        virtual ~IControllerRepository() = default;

        // controller has:
        // name
        // type
        // vendor id
        // product id
        // version
        // serial number?
        [[nodiscard]] virtual std::vector<ControllerInfo> getKnownControllerTypes() const = 0;

        // profile has:
        // name
        // controller type
        // input mappings
        // deadzones
        // sensitivities
        [[nodiscard]] virtual std::shared_ptr<ControllerProfile> getControllerProfile(int profileId) const = 0;

        // input mapping has:
        // platform id
        // profile id
        // list of retropad -> other mappings
        [[nodiscard]] virtual std::shared_ptr<InputMapping> getInputMapping(int mappingId) const = 0;

        [[nodiscard]] virtual std::shared_ptr<InputMapping> getInputMapping(int profileId, int platformId) = 0;
    };
}

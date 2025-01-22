#pragma once

#include "profiles/controller_profile.hpp"
#include "profiles/input_mapping.hpp"
#include "profiles/keyboard_mapping.hpp"
#include <vector>

namespace firelight::input {
    class IControllerRepository {
    public:
        virtual ~IControllerRepository() = default;

        virtual std::shared_ptr<ControllerProfile> getControllerProfile(std::string name, int vendorId, int productId,
                                                                        int productVersion) = 0;

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

        virtual std::shared_ptr<KeyboardMapping> getKeyboardMapping(int profileId, int platformId) = 0;
    };
}

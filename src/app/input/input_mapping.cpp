#include "input_mapping.hpp"

namespace firelight::input {
    std::optional<InputMapping::InputDescription> InputMapping::getButtonMapping(
        const libretro::IRetroPad::Button button) {
        if (!m_buttonMappings.contains(button)) {
            return std::nullopt;
        }

        return m_buttonMappings[button];
    }

    std::optional<InputMapping::InputDescription> InputMapping::getAxisMapping(const libretro::IRetroPad::Axis axis) {
        if (!m_axisMappings.contains(axis)) {
            return std::nullopt;
        }

        return m_axisMappings[axis];
    }
}


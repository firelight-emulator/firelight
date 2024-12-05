#include "input_mapping.hpp"

namespace firelight::input {
    std::optional<bool>
    InputMapping::evaluateButtonMapping(SDL_Joystick *joystick, libretro::IRetroPad::Button button) {
        return std::nullopt;
    }

    std::optional<float> InputMapping::evaluateAxisMapping(SDL_Joystick *joystick, libretro::IRetroPad::Axis axis) {
        return std::nullopt;
    }

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


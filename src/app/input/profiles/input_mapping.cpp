#include "input_mapping.hpp"

namespace firelight::input {
    InputMapping::InputMapping(std::function<void(InputMapping &)> syncCallback) : m_syncCallback(
        std::move(syncCallback)) {
    }

    void InputMapping::addMapping(libretro::IRetroPad::Input input, libretro::IRetroPad::Input mappedInput) {
        m_mappings.erase(input);
        m_mappings.emplace(input, mappedInput);
    }

    std::optional<libretro::IRetroPad::Input> InputMapping::getMappedInput(const libretro::IRetroPad::Input input) {
        if (!m_mappings.contains(input)) {
            return std::nullopt;
        }
        return {m_mappings[input]};
    }

    std::map<libretro::IRetroPad::Input, libretro::IRetroPad::Input> &InputMapping::getMappings() {
        return m_mappings;
    }

    void InputMapping::removeMapping(const libretro::IRetroPad::Input input) {
        m_mappings.erase(input);
    }

    std::string InputMapping::serialize() {
        return "heya";
    }

    void InputMapping::deserialize(const std::string &data) {
    }

    void InputMapping::sync() {
        if (m_syncCallback) {
            m_syncCallback(*this);
        }
    }

    unsigned InputMapping::getId() const {
        return m_id;
    }

    unsigned InputMapping::getControllerProfileId() {
        return m_controllerProfileId;
    }

    unsigned InputMapping::getPlatformId() const {
        return m_platformId;
    }

    void InputMapping::setId(unsigned id) {
        m_id = id;
    }

    void InputMapping::setControllerProfileId(unsigned controllerProfileId) {
        m_controllerProfileId = controllerProfileId;
    }

    void InputMapping::setPlatformId(unsigned platformId) {
        m_platformId = platformId;
    }
}


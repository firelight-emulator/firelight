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
        auto result = std::string();
        for (const auto &[input, mappedInput]: m_mappings) {
            result += std::to_string(input) + ":" + std::to_string(mappedInput) + ",";
        }
        return result;
    }

    void InputMapping::deserialize(const std::string &data) {
        m_mappings.clear();

        std::string::size_type start = 0;
        std::string::size_type end = 0;
        while ((end = data.find(',', start)) != std::string::npos) {
            auto mapping = data.substr(start, end - start);
            const auto split = mapping.find(':');
            if (split == std::string::npos) {
                continue;
            }
            auto input = std::stoi(mapping.substr(0, split));
            auto mappedInput = std::stoi(mapping.substr(split + 1));
            m_mappings.emplace(static_cast<libretro::IRetroPad::Input>(input),
                               static_cast<libretro::IRetroPad::Input>(mappedInput));
            start = end + 1;
        }
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


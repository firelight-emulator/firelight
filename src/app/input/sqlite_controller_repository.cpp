#include "sqlite_controller_repository.hpp"

namespace firelight::input {
    SqliteControllerRepository::SqliteControllerRepository() {
        m_profiles.emplace(0, ControllerProfile{});
    }

    std::vector<ControllerInfo> SqliteControllerRepository::getKnownControllerTypes() const {
        return {};
    }

    std::shared_ptr<ControllerProfile> SqliteControllerRepository::getControllerProfile(const int profileId) const {
        if (m_profiles.contains(profileId)) {
            return std::make_shared<ControllerProfile>(m_profiles.at(profileId));
        }

        return {};
    }

    std::shared_ptr<InputMapping> SqliteControllerRepository::getInputMapping(const int mappingId) const {
        if (m_inputMappings.contains(mappingId)) {
            return std::make_shared<InputMapping>(m_inputMappings.at(mappingId));
        }

        return {};
    }

    std::shared_ptr<InputMapping> SqliteControllerRepository::getInputMapping(int profileId, int platformId) const {
        return {};
    }
} // input

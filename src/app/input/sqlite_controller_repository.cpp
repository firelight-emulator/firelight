#include "sqlite_controller_repository.hpp"

namespace firelight::input {
    SqliteControllerRepository::SqliteControllerRepository() {
        m_inputMappings.emplace_back(std::make_shared<InputMapping>());
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
        for (const auto &mapping: m_inputMappings) {
            if (mapping->getId() == mappingId) {
                return mapping;
            }
        }

        return nullptr;
    }

    std::shared_ptr<InputMapping>
    SqliteControllerRepository::getInputMapping(const int profileId, const int platformId) {
        for (const auto &mapping: m_inputMappings) {
            if (mapping->getControllerProfileId() == profileId && mapping->getPlatformId() ==
                platformId) {
                return mapping;
            }
        }

        auto mapping = std::make_shared<InputMapping>();
        mapping->setControllerProfileId(profileId);
        mapping->setPlatformId(platformId);

        m_inputMappings.emplace_back(mapping);
        return mapping;
    }
} // input

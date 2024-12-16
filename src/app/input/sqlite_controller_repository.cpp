#include "sqlite_controller_repository.hpp"

namespace firelight::input {
    SqliteControllerRepository::SqliteControllerRepository() = default;

    std::shared_ptr<ControllerProfile> SqliteControllerRepository::getControllerProfile(const int profileId) const {
        // if (m_profiles.contains(profileId)) {
        //     return std::make_shared<ControllerProfile>(m_profiles.at(profileId));
        // }

        return {};
    }

    std::shared_ptr<ControllerProfile> SqliteControllerRepository::getControllerProfile(
        const int vendorId, const int productId,
        const int productVersion) {
        for (const auto &profile: m_profiles) {
            if (profile->getVendorId() == vendorId && profile->getProductId() == productId &&
                profile->getProductVersion() == productVersion) {
                return profile;
            }
        }

        auto profile = std::make_shared<ControllerProfile>(vendorId, productId, productVersion);
        m_profiles.emplace_back(profile);
        return profile;
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

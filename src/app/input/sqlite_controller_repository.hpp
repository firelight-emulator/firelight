#pragma once

#include <memory>

#include "controller_repository.hpp"

namespace firelight::input {
    class SqliteControllerRepository : public IControllerRepository {
    public:
        SqliteControllerRepository();

        ~SqliteControllerRepository() override = default;

        [[nodiscard]] std::vector<ControllerInfo> getKnownControllerTypes() const override;

        [[nodiscard]] std::shared_ptr<ControllerProfile> getControllerProfile(int profileId) const override;

        [[nodiscard]] std::shared_ptr<InputMapping> getInputMapping(int mappingId) const override;

        [[nodiscard]] std::shared_ptr<InputMapping> getInputMapping(int profileId, int platformId) const override;

    private:
        std::vector<ControllerInfo> m_controllerTypes{};
        std::map<int, ControllerProfile> m_profiles{};
        std::map<int, InputMapping> m_inputMappings{};
    };
} // input
// firelight

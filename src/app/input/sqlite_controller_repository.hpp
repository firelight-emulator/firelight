#pragma once

#include <memory>
#include <QSqlDatabase>

#include "controller_repository.hpp"

namespace firelight::input {
    class SqliteControllerRepository final : public IControllerRepository {
    public:
        explicit SqliteControllerRepository(QString dbFilePath);

        ~SqliteControllerRepository() override = default;

        [[nodiscard]] std::shared_ptr<ControllerProfile> getControllerProfile(int profileId) const override;

        [[nodiscard]] std::shared_ptr<ControllerProfile> getControllerProfile(
            std::string name, int vendorId, int productId,
            int productVersion) override;

        [[nodiscard]] std::shared_ptr<InputMapping> getInputMapping(int mappingId) const override;

        [[nodiscard]] std::shared_ptr<InputMapping> getInputMapping(int profileId, int platformId) override;

    private:
        QString m_dbFilePath;

        [[nodiscard]] QSqlDatabase getDatabase() const;

        std::vector<std::shared_ptr<ControllerProfile> > m_profiles{};
        std::vector<std::shared_ptr<InputMapping> > m_inputMappings{};
    };
} // input
// firelight

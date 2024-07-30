#pragma once

#include <QObject>
#include <firelight/userdata_database.hpp>

#include "libretro/core_configuration.hpp"

class EmulatorConfigManager : public QObject {
    Q_OBJECT

public:
    explicit EmulatorConfigManager(firelight::db::IUserdataDatabase &userdataDatabase);

    ~EmulatorConfigManager() override;

    [[nodiscard]] std::shared_ptr<CoreConfiguration> getCoreConfigFor(int platformId, int entryId);

    Q_INVOKABLE void setOptionValueForPlatform(int platformId, const QString &key, const QString &value);

    Q_INVOKABLE void setOptionValueForEntry(int entryId, const QString &key, const QString &value);

    Q_INVOKABLE QString getOptionValueForPlatform(int platformId, const QString &key);

    Q_INVOKABLE QString getOptionValueForEntry(int entryId, const QString &key);

private:
    firelight::db::IUserdataDatabase &m_userdataDatabase;
    std::map<std::string, std::shared_ptr<CoreConfiguration> > m_coreConfigs;
};

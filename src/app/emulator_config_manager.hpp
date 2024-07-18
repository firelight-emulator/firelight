#pragma once

#include <QObject>

#include "libretro/core_configuration.hpp"

class EmulatorConfigManager : public QObject {
    Q_OBJECT

public:
    explicit EmulatorConfigManager(QObject *parent = nullptr);

    ~EmulatorConfigManager() override;

    [[nodiscard]] std::shared_ptr<CoreConfiguration> getCoreConfigFor(int platformId, int entryId);

    Q_INVOKABLE void setOptionValueForPlatform(int platformId, QString key, QString value);

    Q_INVOKABLE void setOptionValueForEntry(int entryId, QString key, QString value);

private:
    std::map<std::string, std::shared_ptr<CoreConfiguration> > m_coreConfigs;
};

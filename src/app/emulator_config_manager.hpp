#pragma once

#include <QObject>
#include <firelight/userdata_database.hpp>

// NOTE: legacy. Only the GB/GBC/GBA platform settings QML pages still use this
// (a thin wrapper over userdata platform_settings + PlatformMetadata defaults).
// It is slated for removal when those pages move to the catalog-driven settings
// UI (config redesign Phase 4). It no longer touches CoreConfiguration.
class EmulatorConfigManager : public QObject {
    Q_OBJECT

public:
    explicit EmulatorConfigManager(firelight::db::IUserdataDatabase &userdataDatabase);

    ~EmulatorConfigManager() override;

    Q_INVOKABLE void setOptionValueForPlatform(int platformId, const QString &key, const QString &value);

    Q_INVOKABLE void setOptionValueForEntry(int entryId, const QString &key, const QString &value);

    Q_INVOKABLE QString getOptionValueForPlatform(int platformId, const QString &key);

    Q_INVOKABLE QString getOptionValueForEntry(int entryId, const QString &key);

private:
    firelight::db::IUserdataDatabase &m_userdataDatabase;
};

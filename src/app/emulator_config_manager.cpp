#include "emulator_config_manager.hpp"
#include "platform_metadata.hpp"

#include <utility>

static QString thing = "disabled";

EmulatorConfigManager::EmulatorConfigManager(firelight::db::IUserdataDatabase &userdataDatabase) : m_userdataDatabase(
    userdataDatabase) {
}

EmulatorConfigManager::~EmulatorConfigManager() = default;

std::shared_ptr<CoreConfiguration> EmulatorConfigManager::getCoreConfigFor(
    const int platformId, const QString &contentHash) {
    const auto key = std::to_string(platformId) + "_" + contentHash.toStdString();

    if (!m_coreConfigs.contains(key)) {
        m_coreConfigs[key] = std::make_shared<CoreConfiguration>();
    }

    // m_coreConfigs[key] = std::make_shared<CoreConfiguration>();

    for (const auto &setting: firelight::PlatformMetadata::getDefaultConfigValues(platformId)) {
        m_coreConfigs[key]->setPlatformValue(setting.first, setting.second);
    }

    const auto allSettings = m_userdataDatabase.getAllPlatformSettings(platformId);
    for (const auto &setting: allSettings) {
        m_coreConfigs[key]->setPlatformValue(setting.first, setting.second);
    }

    return m_coreConfigs.at(key);
}

void EmulatorConfigManager::setOptionValueForPlatform(const int platformId, const QString &key, const QString &value) {
    printf("Setting PLATFORM option %s to %s\n", key.toStdString().c_str(), value.toStdString().c_str());
    m_userdataDatabase.setPlatformSettingValue(platformId, key.toStdString(), value.toStdString());
}

void EmulatorConfigManager::setOptionValueForEntry(int entryId, const QString &key, const QString &value) {
    printf("Changing ENTRY option %s to %s\n", key.toStdString().c_str(), value.toStdString().c_str());
}

QString EmulatorConfigManager::getOptionValueForPlatform(const int platformId, const QString &key) {
    printf("Getting PLATFORM option %s\n", key.toStdString().c_str());

    auto val = m_userdataDatabase.getPlatformSettingValue(platformId, key.toStdString());
    if (!val.has_value()) {
        val = firelight::PlatformMetadata::getDefaultConfigValue(platformId, key.toStdString());
    }

    if (val.has_value()) {
        printf("Found value (%s)!\n", val.value().c_str());
        return QString::fromStdString(val.value());
    }

    printf("No value\n");
    return "";
}

QString EmulatorConfigManager::getOptionValueForEntry(int entryId, const QString &key) {
    printf("Getting ENTRY option %s\n", key.toStdString().c_str());
    return "";
}


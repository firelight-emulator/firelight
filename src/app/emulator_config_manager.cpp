#include "emulator_config_manager.hpp"

#include <utility>

static QString thing = "disabled";

EmulatorConfigManager::EmulatorConfigManager(QObject *parent) : QObject(parent) {
}

EmulatorConfigManager::~EmulatorConfigManager() = default;

std::shared_ptr<CoreConfiguration> EmulatorConfigManager::getCoreConfigFor(const int platformId, const int entryId) {
    const auto key = std::to_string(platformId) + "_" + std::to_string(entryId);

    if (m_coreConfigs.contains(key)) {
        m_coreConfigs[key]->setPlatformValue("gambatte_gb_colorization", thing.toStdString());
        return m_coreConfigs.at(key);
    }

    m_coreConfigs[key] = std::make_shared<CoreConfiguration>();
    m_coreConfigs[key]->setPlatformValue("gambatte_gb_colorization", thing.toStdString());
    return m_coreConfigs.at(key);
}

void EmulatorConfigManager::setOptionValueForPlatform(int platformId, const QString &key, const QString &value) {
    printf("Changing PLATFORM option %s to %s\n", key.toStdString().c_str(), value.toStdString().c_str());
}

void EmulatorConfigManager::setOptionValueForEntry(int entryId, const QString &key, const QString &value) {
    printf("Changing ENTRY option %s to %s\n", key.toStdString().c_str(), value.toStdString().c_str());
}

QString EmulatorConfigManager::getOptionValueForPlatform(int platformId, const QString &key) {
    printf("Getting PLATFORM option %s\n", key.toStdString().c_str());
    return "";
}

QString EmulatorConfigManager::getOptionValueForEntry(int entryId, const QString &key) {
    printf("Getting ENTRY option %s\n", key.toStdString().c_str());
    return "";
}


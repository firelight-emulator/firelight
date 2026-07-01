#include "manager_accessor.hpp"

#include <utility>

namespace firelight {
saves::ISaveManager *ManagerAccessor::m_saveManager;
db::IUserdataDatabase *ManagerAccessor::m_userdataDatabase;
achievements::RAClient *ManagerAccessor::m_achievementManager;
std::shared_ptr<EmulatorConfigManager> ManagerAccessor::m_emulatorConfigManager;
gui::GameImageProvider *ManagerAccessor::m_gameImageProvider;
activity::IActivityLog *ManagerAccessor::m_activityLog;
std::string ManagerAccessor::m_coreSystemDirectory;
mods::IModRepository *ManagerAccessor::m_modDatabase;
settings::ISettingsRepository
    *ManagerAccessor::m_emulationSettingsManager;
discord::IDiscordManager *ManagerAccessor::m_discordManager;

void ManagerAccessor::setSaveManager(saves::ISaveManager *t_manager) {
  m_saveManager = t_manager;
}

void ManagerAccessor::setUserdataManager(
    db::IUserdataDatabase *t_userdataManager) {
  m_userdataDatabase = t_userdataManager;
}

void ManagerAccessor::setAchievementManager(
    achievements::RAClient *t_achievementManager) {
  m_achievementManager = t_achievementManager;
}

void ManagerAccessor::setEmulatorConfigManager(
    std::shared_ptr<EmulatorConfigManager> t_emulatorConfigManager) {
  m_emulatorConfigManager = std::move(t_emulatorConfigManager);
}

void ManagerAccessor::setGameImageProvider(
    gui::GameImageProvider *t_gameImageProvider) {
  m_gameImageProvider = t_gameImageProvider;
}

void ManagerAccessor::setActivityLog(activity::IActivityLog *t_activityLog) {
  m_activityLog = t_activityLog;
}
void ManagerAccessor::setCoreSystemDirectory(
    const std::string &t_coreSystemDirectory) {
  m_coreSystemDirectory = t_coreSystemDirectory;
}
void ManagerAccessor::setModRepository(mods::IModRepository *t_modDatabase) {
  m_modDatabase = t_modDatabase;
}
void ManagerAccessor::setEmulationSettingsManager(
    settings::ISettingsRepository *t_emulationSettingsManager) {
  m_emulationSettingsManager = t_emulationSettingsManager;
}
void ManagerAccessor::setDiscordManager(
    discord::IDiscordManager *t_discordManager) {
  m_discordManager = t_discordManager;
}

saves::ISaveManager *ManagerAccessor::getSaveManager() { return m_saveManager; }

db::IUserdataDatabase *ManagerAccessor::getUserdataManager() {
  return m_userdataDatabase;
}

achievements::RAClient *ManagerAccessor::getAchievementManager() {
  return m_achievementManager;
}

std::shared_ptr<EmulatorConfigManager>
ManagerAccessor::getEmulatorConfigManager() {
  return m_emulatorConfigManager;
}

gui::GameImageProvider *ManagerAccessor::getGameImageProvider() {
  return m_gameImageProvider;
}

activity::IActivityLog *ManagerAccessor::getActivityLog() {
  return m_activityLog;
}

std::string ManagerAccessor::getCoreSystemDirectory() {
  return m_coreSystemDirectory;
}
mods::IModRepository *ManagerAccessor::getModRepository() {
  return m_modDatabase;
}
settings::ISettingsRepository *
ManagerAccessor::getEmulationSettingsManager() {
  return m_emulationSettingsManager;
}
discord::IDiscordManager *ManagerAccessor::getDiscordManager() {
  return m_discordManager;
}
} // namespace firelight

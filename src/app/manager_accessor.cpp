#include "manager_accessor.hpp"

#include <utility>

namespace firelight {
input::ControllerManager *ManagerAccessor::m_controllerManager;
saves::SaveManager *ManagerAccessor::m_saveManager;
db::IUserdataDatabase *ManagerAccessor::m_userdataDatabase;
db::ILibraryDatabase *ManagerAccessor::m_libraryDatabase;
achievements::RAClient *ManagerAccessor::m_achievementManager;
std::shared_ptr<EmulatorConfigManager> ManagerAccessor::m_emulatorConfigManager;
gui::GameImageProvider *ManagerAccessor::m_gameImageProvider;
library::IUserLibrary *ManagerAccessor::m_userLibrary;
activity::IActivityLog *ManagerAccessor::m_activityLog;
input::InputManager *ManagerAccessor::m_inputManager;
input::IControllerRepository *ManagerAccessor::m_controllerRepository;
std::string ManagerAccessor::m_coreSystemDirectory;
mods::IModRepository *ManagerAccessor::m_modDatabase;
settings::IEmulationSettingsManager
    *ManagerAccessor::m_emulationSettingsManager;
discord::DiscordManager *ManagerAccessor::m_discordManager;

void ManagerAccessor::setControllerManager(
    input::ControllerManager *t_manager) {
  m_controllerManager = t_manager;
}

void ManagerAccessor::setSaveManager(saves::SaveManager *t_manager) {
  m_saveManager = t_manager;
}

void ManagerAccessor::setUserdataManager(
    db::IUserdataDatabase *t_userdataManager) {
  m_userdataDatabase = t_userdataManager;
}

void ManagerAccessor::setLibraryDatabase(
    db::ILibraryDatabase *t_libraryDatabase) {
  m_libraryDatabase = t_libraryDatabase;
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

void ManagerAccessor::setUserLibrary(library::IUserLibrary *t_userLibrary) {
  m_userLibrary = t_userLibrary;
}

void ManagerAccessor::setActivityLog(activity::IActivityLog *t_activityLog) {
  m_activityLog = t_activityLog;
}

void ManagerAccessor::setInputManager(input::InputManager *t_inputManager) {
  m_inputManager = t_inputManager;
}

void ManagerAccessor::setControllerRepository(
    input::IControllerRepository *t_controllerRepository) {
  m_controllerRepository = t_controllerRepository;
}
void ManagerAccessor::setCoreSystemDirectory(
    const std::string &t_coreSystemDirectory) {
  m_coreSystemDirectory = t_coreSystemDirectory;
}
void ManagerAccessor::setModRepository(mods::IModRepository *t_modDatabase) {
  m_modDatabase = t_modDatabase;
}
void ManagerAccessor::setEmulationSettingsManager(
    settings::IEmulationSettingsManager *t_emulationSettingsManager) {
  m_emulationSettingsManager = t_emulationSettingsManager;
}
void ManagerAccessor::setDiscordManager(
    discord::DiscordManager *t_discordManager) {
  m_discordManager = t_discordManager;
}

input::ControllerManager *ManagerAccessor::getControllerManager() {
  // TODO: Check for nullptr and throw exception or something
  return m_controllerManager;
}

saves::SaveManager *ManagerAccessor::getSaveManager() { return m_saveManager; }

db::IUserdataDatabase *ManagerAccessor::getUserdataManager() {
  return m_userdataDatabase;
}

db::ILibraryDatabase *ManagerAccessor::getLibraryDatabase() {
  return m_libraryDatabase;
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

library::IUserLibrary *ManagerAccessor::getUserLibrary() {
  return m_userLibrary;
}

activity::IActivityLog *ManagerAccessor::getActivityLog() {
  return m_activityLog;
}

input::InputManager *ManagerAccessor::getInputManager() {
  return m_inputManager;
}

input::IControllerRepository *ManagerAccessor::getControllerRepository() {
  return m_controllerRepository;
}
std::string ManagerAccessor::getCoreSystemDirectory() {
  return m_coreSystemDirectory;
}
mods::IModRepository *ManagerAccessor::getModRepository() {
  return m_modDatabase;
}
settings::IEmulationSettingsManager *
ManagerAccessor::getEmulationSettingsManager() {
  return m_emulationSettingsManager;
}
discord::DiscordManager *ManagerAccessor::getDiscordManager() {
  return m_discordManager;
}
} // namespace firelight

#include "service_accessor.hpp"

namespace firelight {

input::InputService *ServiceAccessor::s_inputService;
platforms::PlatformService *ServiceAccessor::s_platformService;
settings::SettingsService *ServiceAccessor::s_settingsService;
achievements::AchievementService *ServiceAccessor::s_achievementService;
library::IUserLibrary *ServiceAccessor::s_libraryService;
saves::ISaveManager *ServiceAccessor::s_saveManager;
gui::GameImageProvider *ServiceAccessor::s_gameImageProvider;

void ServiceAccessor::setInputService(input::InputService *service) {
  s_inputService = service;
}
void ServiceAccessor::setPlatformService(platforms::PlatformService *service) {
  s_platformService = service;
}
void ServiceAccessor::setSettingsService(settings::SettingsService *service) {
  s_settingsService = service;
}
void ServiceAccessor::setAchievementService(
    achievements::AchievementService *service) {
  s_achievementService = service;
}
void ServiceAccessor::setLibraryService(library::IUserLibrary *service) { s_libraryService = service; }

void ServiceAccessor::setSaveManager(saves::ISaveManager *manager) { s_saveManager = manager; }

void ServiceAccessor::setGameImageProvider(gui::GameImageProvider *provider) { s_gameImageProvider = provider; }

input::InputService *ServiceAccessor::getInputService() {
  return s_inputService;
}
platforms::PlatformService *ServiceAccessor::getPlatformService() {
  return s_platformService;
}
settings::SettingsService *ServiceAccessor::getSettingsService() {
  return s_settingsService;
}
achievements::AchievementService *ServiceAccessor::getAchievementService() {
  return s_achievementService;
}
library::IUserLibrary *ServiceAccessor::getLibraryService() { return s_libraryService; }

saves::ISaveManager *ServiceAccessor::getSaveManager() { return s_saveManager; }

gui::GameImageProvider *ServiceAccessor::getGameImageProvider() { return s_gameImageProvider;}
} // namespace firelight
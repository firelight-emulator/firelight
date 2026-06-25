#include "service_accessor.hpp"

namespace firelight {
  input::InputService *ServiceAccessor::s_inputService;
  platforms::PlatformService *ServiceAccessor::s_platformService;
  settings::SettingsService *ServiceAccessor::s_settingsService;
  achievements::AchievementService *ServiceAccessor::s_achievementService;
  library::IUserLibrary *ServiceAccessor::s_libraryService;
  activity::IActivityLog *ServiceAccessor::s_activityService;

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

  void ServiceAccessor::setLibraryService(library::IUserLibrary *service) {
    s_libraryService = service;
  }

  void ServiceAccessor::setActivityService(activity::IActivityLog *service) {
    s_activityService = service;
  }

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

  library::IUserLibrary *ServiceAccessor::getLibraryService() {
    return s_libraryService;
  }

  activity::IActivityLog *ServiceAccessor::getActivityService() {
    return s_activityService;
  }
} // namespace firelight

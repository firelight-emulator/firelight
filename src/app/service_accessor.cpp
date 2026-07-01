#include "service_accessor.hpp"

namespace firelight {
  input::InputService *ServiceAccessor::s_inputService;
  input::IControllerRepository *ServiceAccessor::s_controllerProfileRepository;
  platforms::PlatformService *ServiceAccessor::s_platformService;
  settings::SettingsService *ServiceAccessor::s_settingsService;
  achievements::AchievementService *ServiceAccessor::s_achievementService;
  library::UserLibraryService *ServiceAccessor::s_libraryService;
  activity::IActivityLog *ServiceAccessor::s_activityService;

  void ServiceAccessor::setInputService(input::InputService *service) {
    s_inputService = service;
  }

  void ServiceAccessor::setControllerProfileRepository(
    input::IControllerRepository *repository) {
    s_controllerProfileRepository = repository;
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

  void ServiceAccessor::setLibraryService(library::UserLibraryService *service) {
    s_libraryService = service;
  }

  void ServiceAccessor::setActivityService(activity::IActivityLog *service) {
    s_activityService = service;
  }

  input::InputService *ServiceAccessor::getInputService() {
    return s_inputService;
  }

  input::IControllerRepository *
  ServiceAccessor::getControllerProfileRepository() {
    return s_controllerProfileRepository;
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

  library::UserLibraryService *ServiceAccessor::getLibraryService() {
    return s_libraryService;
  }

  activity::IActivityLog *ServiceAccessor::getActivityService() {
    return s_activityService;
  }
} // namespace firelight

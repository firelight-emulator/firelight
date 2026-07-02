#include "service_accessor.hpp"

namespace firelight {
  input::InputService *ServiceAccessor::s_inputService;
  input::IControllerRepository *ServiceAccessor::s_controllerProfileRepository;
  platforms::PlatformService *ServiceAccessor::s_platformService;
  settings::SettingsService *ServiceAccessor::s_settingsService;
  settings::ICoreOptionRepository *ServiceAccessor::s_coreOptionRepository;
  achievements::AchievementService *ServiceAccessor::s_achievementService;
  library::UserLibraryService *ServiceAccessor::s_libraryService;
  activity::IActivityLog *ServiceAccessor::s_activityService;
  saves::ISaveManager *ServiceAccessor::s_saveManager;
  db::IUserdataDatabase *ServiceAccessor::s_userdataDatabase;
  achievements::RAClient *ServiceAccessor::s_achievementManager;
  gui::GameImageProvider *ServiceAccessor::s_gameImageProvider;
  std::string ServiceAccessor::s_coreSystemDirectory;
  mods::IModRepository *ServiceAccessor::s_modRepository;
  settings::ISettingsRepository *ServiceAccessor::s_emulationSettingsManager;
  discord::IDiscordManager *ServiceAccessor::s_discordManager;

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

  void ServiceAccessor::setCoreOptionRepository(
    settings::ICoreOptionRepository *repository) {
    s_coreOptionRepository = repository;
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

  void ServiceAccessor::setSaveManager(saves::ISaveManager *manager) {
    s_saveManager = manager;
  }

  void ServiceAccessor::setUserdataManager(db::IUserdataDatabase *manager) {
    s_userdataDatabase = manager;
  }

  void ServiceAccessor::setAchievementManager(achievements::RAClient *manager) {
    s_achievementManager = manager;
  }

  void ServiceAccessor::setGameImageProvider(gui::GameImageProvider *provider) {
    s_gameImageProvider = provider;
  }

  void ServiceAccessor::setCoreSystemDirectory(const std::string &directory) {
    s_coreSystemDirectory = directory;
  }

  void ServiceAccessor::setModRepository(mods::IModRepository *repository) {
    s_modRepository = repository;
  }

  void ServiceAccessor::setEmulationSettingsManager(
    settings::ISettingsRepository *repository) {
    s_emulationSettingsManager = repository;
  }

  void ServiceAccessor::setDiscordManager(discord::IDiscordManager *manager) {
    s_discordManager = manager;
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

  settings::ICoreOptionRepository *ServiceAccessor::getCoreOptionRepository() {
    return s_coreOptionRepository;
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

  saves::ISaveManager *ServiceAccessor::getSaveManager() {
    return s_saveManager;
  }

  db::IUserdataDatabase *ServiceAccessor::getUserdataManager() {
    return s_userdataDatabase;
  }

  achievements::RAClient *ServiceAccessor::getAchievementManager() {
    return s_achievementManager;
  }

  gui::GameImageProvider *ServiceAccessor::getGameImageProvider() {
    return s_gameImageProvider;
  }

  std::string ServiceAccessor::getCoreSystemDirectory() {
    return s_coreSystemDirectory;
  }

  mods::IModRepository *ServiceAccessor::getModRepository() {
    return s_modRepository;
  }

  settings::ISettingsRepository *ServiceAccessor::getEmulationSettingsManager() {
    return s_emulationSettingsManager;
  }

  discord::IDiscordManager *ServiceAccessor::getDiscordManager() {
    return s_discordManager;
  }
} // namespace firelight

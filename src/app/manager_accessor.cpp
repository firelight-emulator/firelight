#include "manager_accessor.hpp"

#include <utility>

namespace firelight {
  Input::ControllerManager *ManagerAccessor::m_controllerManager;
  saves::SaveManager *ManagerAccessor::m_saveManager;
  LibraryScanner *ManagerAccessor::m_libraryManager;
  db::IUserdataDatabase *ManagerAccessor::m_userdataDatabase;
  db::ILibraryDatabase *ManagerAccessor::m_libraryDatabase;
  achievements::RAClient *ManagerAccessor::m_achievementManager;
  std::shared_ptr<EmulatorConfigManager> ManagerAccessor::m_emulatorConfigManager;

  void ManagerAccessor::setControllerManager(
    Input::ControllerManager *t_manager) {
    m_controllerManager = t_manager;
  }

  void ManagerAccessor::setSaveManager(saves::SaveManager *t_manager) {
    m_saveManager = t_manager;
  }

  void ManagerAccessor::setLibraryManager(LibraryScanner *t_libraryManager) {
    m_libraryManager = t_libraryManager;
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

  void ManagerAccessor::setEmulatorConfigManager(std::shared_ptr<EmulatorConfigManager> t_emulatorConfigManager) {
    m_emulatorConfigManager = std::move(t_emulatorConfigManager);
  }

  Input::ControllerManager *ManagerAccessor::getControllerManager() {
    // TODO: Check for nullptr and throw exception or something
    return m_controllerManager;
  }

  saves::SaveManager *ManagerAccessor::getSaveManager() { return m_saveManager; }

  LibraryScanner *ManagerAccessor::getLibraryManager() {
    return m_libraryManager;
  }

  db::IUserdataDatabase *ManagerAccessor::getUserdataManager() {
    return m_userdataDatabase;
  }

  db::ILibraryDatabase *ManagerAccessor::getLibraryDatabase() {
    return m_libraryDatabase;
  }

  achievements::RAClient *ManagerAccessor::getAchievementManager() {
    return m_achievementManager;
  }

  std::shared_ptr<EmulatorConfigManager> ManagerAccessor::getEmulatorConfigManager() {
    return m_emulatorConfigManager;
  }
} // namespace firelight

//
// Created by alexs on 1/7/2024.
//

#include "manager_accessor.hpp"

namespace firelight {

Input::ControllerManager *ManagerAccessor::m_controllerManager;
Saves::SaveManager *ManagerAccessor::m_saveManager;
QLibraryManager *ManagerAccessor::m_libraryManager;
db::IUserdataDatabase *ManagerAccessor::m_userdataDatabase;
db::ILibraryDatabase *ManagerAccessor::m_libraryDatabase;

void ManagerAccessor::setControllerManager(
    Input::ControllerManager *t_manager) {
  m_controllerManager = t_manager;
}
void ManagerAccessor::setSaveManager(Saves::SaveManager *t_manager) {
  m_saveManager = t_manager;
}
void ManagerAccessor::setLibraryManager(QLibraryManager *t_libraryManager) {
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

Input::ControllerManager *ManagerAccessor::getControllerManager() {
  // TODO: Check for nullptr and throw exception or something
  return m_controllerManager;
}
Saves::SaveManager *ManagerAccessor::getSaveManager() { return m_saveManager; }
QLibraryManager *ManagerAccessor::getLibraryManager() {
  return m_libraryManager;
}
db::IUserdataDatabase *ManagerAccessor::getUserdataManager() {
  return m_userdataDatabase;
}
db::ILibraryDatabase *ManagerAccessor::getLibraryDatabase() {
  return m_libraryDatabase;
}
} // namespace firelight

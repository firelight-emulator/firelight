//
// Created by alexs on 1/7/2024.
//

#include "manager_accessor.hpp"

namespace Firelight {

Input::ControllerManager *ManagerAccessor::m_controllerManager;
Saves::SaveManager *ManagerAccessor::m_saveManager;
QLibraryManager *ManagerAccessor::m_libraryManager;
UserdataDatabase *ManagerAccessor::m_userdataDatabase;

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
void ManagerAccessor::setUserdataManager(UserdataDatabase *t_userdataManager) {
  m_userdataDatabase = t_userdataManager;
}

Input::ControllerManager *ManagerAccessor::getControllerManager() {
  // TODO: Check for nullptr and throw exception or something
  return m_controllerManager;
}
Saves::SaveManager *ManagerAccessor::getSaveManager() { return m_saveManager; }
QLibraryManager *ManagerAccessor::getLibraryManager() {
  return m_libraryManager;
}
UserdataDatabase *ManagerAccessor::getUserdataManager() {
  return m_userdataDatabase;
}
} // namespace Firelight

//
// Created by alexs on 1/7/2024.
//

#include "manager_accessor.hpp"

namespace Firelight {

Input::ControllerManager *ManagerAccessor::m_controllerManager;
Saves::SaveManager *ManagerAccessor::m_saveManager;

void ManagerAccessor::setControllerManager(
    Input::ControllerManager *t_manager) {
  m_controllerManager = t_manager;
}
void ManagerAccessor::setSaveManager(Saves::SaveManager *t_manager) {
  m_saveManager = t_manager;
}

Input::ControllerManager *ManagerAccessor::getControllerManager() {
  // TODO: Check for nullptr and throw exception or something
  return m_controllerManager;
}
Saves::SaveManager *ManagerAccessor::getSaveManager() { return m_saveManager; }
} // namespace Firelight

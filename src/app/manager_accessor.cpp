//
// Created by alexs on 1/7/2024.
//

#include "manager_accessor.hpp"

namespace Firelight {

Input::ControllerManager *ManagerAccessor::m_controllerManager;

void ManagerAccessor::setControllerManager(
    Input::ControllerManager *t_manager) {
  m_controllerManager = t_manager;
}

Input::ControllerManager *ManagerAccessor::getControllerManager() {
  // TODO: Check for nullptr and throw exception or something
  return m_controllerManager;
}
} // namespace Firelight

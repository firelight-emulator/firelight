//
// Created by alexs on 1/7/2024.
//

#ifndef MANAGER_ACCESSOR_HPP
#define MANAGER_ACCESSOR_HPP

#include "../gui/controller_manager.hpp"

namespace Firelight {

namespace Input {
class ControllerManager;
}

class ManagerAccessor {
public:
  static void setControllerManager(Input::ControllerManager *t_manager);

protected:
  static Input::ControllerManager *getControllerManager();

private:
  static Input::ControllerManager *m_controllerManager;
};

} // namespace Firelight

#endif // MANAGER_ACCESSOR_HPP

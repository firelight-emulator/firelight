//
// Created by alexs on 1/7/2024.
//

#ifndef MANAGER_ACCESSOR_HPP
#define MANAGER_ACCESSOR_HPP

#include "../gui/QLibraryManager.hpp"
#include "../gui/controller_manager.hpp"
#include "../gui/save_manager.hpp"

namespace Firelight {

namespace Input {
class ControllerManager;
}

class ManagerAccessor {
public:
  static void setControllerManager(Input::ControllerManager *t_manager);
  static void setSaveManager(Saves::SaveManager *t_manager);
  static void setLibraryManager(QLibraryManager *t_libraryManager);

protected:
  static Input::ControllerManager *getControllerManager();
  static Saves::SaveManager *getSaveManager();
  static QLibraryManager *getLibraryManager();

private:
  static Input::ControllerManager *m_controllerManager;
  static Saves::SaveManager *m_saveManager;
  static QLibraryManager *m_libraryManager;
};

} // namespace Firelight

#endif // MANAGER_ACCESSOR_HPP
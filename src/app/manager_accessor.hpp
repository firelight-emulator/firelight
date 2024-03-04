#pragma once

#include "../gui/QLibraryManager.hpp"
#include "../gui/save_manager.hpp"
#include "db/userdata_database.hpp"
#include "input/controller_manager.hpp"

namespace firelight {

class ManagerAccessor {
public:
  static void setControllerManager(Input::ControllerManager *t_manager);
  static void setSaveManager(Saves::SaveManager *t_manager);
  static void setLibraryManager(QLibraryManager *t_libraryManager);
  static void setUserdataManager(IUserdataDatabase *t_userdataManager);
  static void setLibraryDatabase(db::ILibraryDatabase *t_libraryDatabase);

protected:
  static Input::ControllerManager *getControllerManager();
  static Saves::SaveManager *getSaveManager();
  static QLibraryManager *getLibraryManager();
  static IUserdataDatabase *getUserdataManager();
  static db::ILibraryDatabase *getLibraryDatabase();

private:
  static Input::ControllerManager *m_controllerManager;
  static Saves::SaveManager *m_saveManager;
  static QLibraryManager *m_libraryManager;
  static IUserdataDatabase *m_userdataDatabase;
  static db::ILibraryDatabase *m_libraryDatabase;
};

} // namespace firelight

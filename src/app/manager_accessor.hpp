#pragma once

#include "firelight/userdata_database.hpp"
#include "input/controller_manager.hpp"
#include "library/library_scanner.hpp"
#include "saves/save_manager.hpp"

namespace firelight {

class ManagerAccessor {
public:
  static void setControllerManager(Input::ControllerManager *t_manager);
  static void setSaveManager(saves::SaveManager *t_manager);
  static void setLibraryManager(LibraryScanner *t_libraryManager);
  static void setUserdataManager(db::IUserdataDatabase *t_userdataManager);
  static void setLibraryDatabase(db::ILibraryDatabase *t_libraryDatabase);

protected:
  static Input::ControllerManager *getControllerManager();
  static saves::SaveManager *getSaveManager();
  static LibraryScanner *getLibraryManager();
  static db::IUserdataDatabase *getUserdataManager();
  static db::ILibraryDatabase *getLibraryDatabase();

private:
  static Input::ControllerManager *m_controllerManager;
  static saves::SaveManager *m_saveManager;
  static LibraryScanner *m_libraryManager;
  static db::IUserdataDatabase *m_userdataDatabase;
  static db::ILibraryDatabase *m_libraryDatabase;
};

} // namespace firelight

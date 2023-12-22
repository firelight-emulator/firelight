//
// Created by alexs on 12/22/2023.
//

#ifndef FIRELIGHT_QLIBRARYMANAGER_HPP
#define FIRELIGHT_QLIBRARYMANAGER_HPP

#include "QLibEntryModelShort.hpp"
#include "src/app/library/library_manager.hpp"

#include <QObject>
class QLibraryManager : public QObject {
  Q_OBJECT

public:
  explicit QLibraryManager(FL::Library::LibraryManager *library_manager,
                           QLibEntryModelShort *shortModel);

  void refresh() const;
  // TODO: refresh slot

private:
  FL::Library::LibraryManager *m_library_manager;
  QLibEntryModelShort *m_shortModel;
};

#endif // FIRELIGHT_QLIBRARYMANAGER_HPP

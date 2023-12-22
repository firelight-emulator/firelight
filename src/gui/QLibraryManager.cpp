//
// Created by alexs on 12/22/2023.
//

#include "QLibraryManager.hpp"
QLibraryManager::QLibraryManager(FL::Library::LibraryManager *library_manager,
                                 QLibEntryModelShort *shortModel)
    : m_shortModel(shortModel), m_library_manager(library_manager) {}

void QLibraryManager::refresh() const {
  m_shortModel->setEntries(m_library_manager->getEntries());
}

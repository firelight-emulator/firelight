//
// Created by alexs on 12/22/2023.
//

#ifndef FIRELIGHT_QLIBRARYMANAGER_HPP
#define FIRELIGHT_QLIBRARYMANAGER_HPP

#include "QLibEntryModelShort.hpp"
#include <QObject>
class QLibraryManager : public QObject {
  Q_OBJECT

public:
  explicit QLibraryManager(QLibEntryModelShort *shortModel);

private:
  QLibEntryModelShort *m_shortModel;
};

#endif // FIRELIGHT_QLIBRARYMANAGER_HPP

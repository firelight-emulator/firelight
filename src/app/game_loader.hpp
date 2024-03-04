#pragma once

#include "manager_accessor.hpp"
#include <QObject>

namespace firelight {

class GameLoader : public QObject, public ManagerAccessor {
  Q_OBJECT

public slots:
  void loadGame(int entryId);

signals:
  void gameLoaded(int entryId, QByteArray romData, QByteArray saveData,
                  QString corePath);
  void gameLoadFailedNoSuchEntry(int entryId);
  void gameLoadFailedOrphanedPatch(int entryId);
  void gameLoadFailedContentMissing(int entryId);
};

} // namespace firelight

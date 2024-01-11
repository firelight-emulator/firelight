//
// Created by alexs on 1/10/2024.
//

#ifndef GAME_LOADER_HPP
#define GAME_LOADER_HPP
#include "manager_accessor.hpp"

#include <QObject>

namespace Firelight {

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

} // namespace Firelight

#endif // GAME_LOADER_HPP

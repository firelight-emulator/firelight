//
// Created by alexs on 4/5/2024.
//

#include "mod_list_model.hpp"

namespace firelight::gui {
ModListModel::ModListModel(IContentDatabase &contentDatabase)
    : m_contentDatabase(contentDatabase) {
  auto mods = m_contentDatabase.getAllMods();
  for (const auto &mod : mods) {

    for (const auto &releaseId : mod.gameReleaseIds) {
      auto release = m_contentDatabase.getGameRelease(releaseId);
      if (!release) {
        continue;
      }

      Item item;
      item.id = mod.id;
      item.name = QString::fromStdString(mod.name);
      const auto actual = *release;

      auto platform = m_contentDatabase.getPlatform(actual.platformId);
      if (!platform) {
        // TODO: ????
        continue;
      }

      item.platform = QString::fromStdString(platform->name);

      item.targetGameId = actual.gameId;
      auto game = m_contentDatabase.getGameByRomId(actual.gameId);
      if (!game) {
        // TODO: ????
        continue;
      }

      item.targetGameName = QString::fromStdString(game->name);

      item.primaryAuthor = QString::fromStdString(mod.primaryAuthor);
      m_items.push_back(item);
    }
  }
}
int ModListModel::rowCount(const QModelIndex &parent) const { return 0; }
QVariant ModListModel::data(const QModelIndex &index, int role) const {
  return QVariant();
}
QHash<int, QByteArray> ModListModel::roleNames() const {
  return QAbstractListModel::roleNames();
}
} // namespace firelight::gui
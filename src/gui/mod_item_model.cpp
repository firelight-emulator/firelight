#include "mod_item_model.hpp"

namespace firelight::gui {
ModItemModel::ModItemModel(db::IContentDatabase &contentDatabase)
    : m_contentDatabase(contentDatabase) {
  auto mods = m_contentDatabase.getAllMods();
  for (const auto &mod : mods) {
    Item item;
    item.id = mod.id;
    item.name = QString::fromStdString(mod.name);
    item.imageSource = QString::fromStdString(mod.imageSource);
    item.description = QString::fromStdString(mod.description);
    item.primaryAuthor = QString::fromStdString(mod.primaryAuthor);
    item.targetGameId = mod.gameId;

    auto patches = m_contentDatabase.getMatchingPatches({.modId = mod.id});
    for (const auto &patch : patches) {
      item.romIds.push_back(patch.romId);
    }

    // QList<int> list;
    // for (const auto &element : release->romIds) {
    //   list.append(element);
    // }

    // item.romIds = list;

    auto game = m_contentDatabase.getGame(mod.gameId);
    if (!game) {
      // TODO: ????
      continue;
    }

    item.targetGameName = QString::fromStdString(game->name);

    auto platform = m_contentDatabase.getPlatform(game->platformId);
    if (!platform) {
      // TODO: ????
      continue;
    }

    item.platform = QString::fromStdString(platform->name);

    m_items.push_back(item);
  }
}

int ModItemModel::rowCount(const QModelIndex &parent) const {
  return m_items.size();
}

QVariant ModItemModel::data(const QModelIndex &index, int role) const {
  if (role < Qt::UserRole || index.row() >= m_items.size()) {
    return QVariant{};
  }

  auto item = m_items.at(index.row());

  switch (role) {
  case Id:
    return item.id;
  case Name:
  case Qt::DisplayRole:
    return item.name;
  case Description:
    return item.description;
  case PrimaryAuthor:
    return item.primaryAuthor;
  case Platform:
    return item.platform;
  case TargetGameName:
    return item.targetGameName;
  case TargetGameId:
    return item.targetGameId;
  case ImageSource:
    return item.imageSource;
  case RomIds:
    return QVariant::fromValue(item.romIds);
  default:
    return QVariant{};
  }
}

QHash<int, QByteArray> ModItemModel::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[Id] = "id";
  roles[Name] = "name";
  roles[Description] = "description";
  roles[PrimaryAuthor] = "primary_author";
  roles[Platform] = "platform_name";
  roles[TargetGameName] = "target_game_name";
  roles[TargetGameId] = "target_game_id";
  roles[ImageSource] = "image_source";
  roles[RomIds] = "rom_ids";
  return roles;
}
} // namespace firelight::gui
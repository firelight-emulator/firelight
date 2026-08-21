#include "shop_item_model.hpp"

namespace firelight::shop {
ShopItemModel::ShopItemModel() {}

QHash<int, QByteArray> ShopItemModel::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[Id] = "id";
  roles[Title] = "title";
  roles[Tagline] = "tagline";
  roles[Description] = "description";
  roles[CreatorName] = "creator_name";
  roles[ModId] = "mod_id";
  roles[PlatformId] = "platform_id";
  roles[PlatformName] = "platform_name";
  roles[TargetGameId] = "target_game_id";
  roles[TargetGameName] = "target_game_name";
  roles[ClearLogoImageUrl] = "clear_logo_image_url";
  roles[CapsuleImageUrl] = "capsule_image_url";
  roles[UserHasRequiredGame] = "user_has_required_game";
  roles[ScreenshotUrls] = "screenshot_urls";
  return roles;
}

int ShopItemModel::rowCount(const QModelIndex &parent) const { return m_items.size(); }

QVariant ShopItemModel::data(const QModelIndex &index, int role) const {
  if (role < Qt::UserRole || index.row() >= m_items.size()) {
    return QVariant{};
  }

  auto item = m_items.at(index.row());

  switch (role) {
  case Id:
    return item.id;
  case Title:
  case Qt::DisplayRole:
    return item.title;
  case Tagline:
    return item.tagline;
  case Description:
    return item.description;
  case CreatorName:
    return item.creatorName;
  case ModId:
    return item.modId;
  case PlatformId:
    return item.platformId;
  case PlatformName:
    return item.platformName;
  case TargetGameId:
    return item.targetGameId;
  case TargetGameName:
    return item.targetGameName;
  case ClearLogoImageUrl:
    return item.clearLogoImageUrl;
  case CapsuleImageUrl:
    return item.capsuleImageUrl;
  case UserHasRequiredGame:
    return item.userHasRequiredGame;
  case ScreenshotUrls:
    return item.screenshotUrls;
  default:
    return QVariant{};
  }
}
} // namespace firelight::shop

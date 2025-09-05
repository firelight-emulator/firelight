//
// Created by alexs on 9/5/2025.
//

#include "game_activity_list_model.hpp"

#include <platforms/platform_service.hpp>

namespace firelight::activity {
GameActivityListModel::GameActivityListModel(QObject *parent)
    : QAbstractListModel(parent) {
  const auto activityLog = getActivityLog();
  if (activityLog == nullptr) {
    return;
  }

  for (const auto &session : activityLog->getPlaySessions()) {
    auto it = std::ranges::find_if(m_items, [&session](const Item &item) {
      return item.contentHash == session.contentHash;
    });

    if (it != m_items.end()) {
      it->numSecondsPlayed += session.unpausedDurationMillis / 1000;
    } else {
      Item newItem;
      newItem.contentHash = QString::fromStdString(session.contentHash);
      newItem.numSecondsPlayed = session.unpausedDurationMillis / 1000;

      auto entry = getUserLibrary()->getEntryWithContentHash(
          QString::fromStdString(session.contentHash));
      if (entry) {
        auto platform = platforms::PlatformService::getInstance().getPlatform(
            entry->platformId);
        if (platform) {
          newItem.platformName = QString::fromStdString(platform->name);
          newItem.platformSlug = QString::fromStdString(platform->slug);
        } else {
          newItem.platformSlug = "unknown";
          newItem.platformName = "Unknown Platform";
        }

        newItem.displayName = entry->displayName;
        newItem.iconUrl1x1 = entry->icon1x1SourceUrl;
      } else {
        spdlog::info("Didn't find entry for content hash {}",
                     session.contentHash);
        newItem.displayName = "Unknown Game";
        newItem.platformSlug = "unknown";
        newItem.platformName = "Unknown Platform";
      }

      m_items.push_back(newItem);
    }
  }

  std::ranges::sort(m_items, [](const Item &a, const Item &b) {
    return a.numSecondsPlayed > b.numSecondsPlayed;
  });
}
int GameActivityListModel::rowCount(const QModelIndex &parent) const {
  return m_items.size();
}
QVariant GameActivityListModel::data(const QModelIndex &index, int role) const {
  if (role < Qt::UserRole || index.row() >= m_items.size()) {
    return QVariant{};
  }

  const auto item = m_items.at(index.row());

  switch (role) {
  case DisplayName:
    return QVariant::fromValue(item.displayName);
  case ContentHash:
    return QVariant::fromValue(item.contentHash);
  case PlatformName:
    return QVariant::fromValue(item.platformName);
  case PlatformSlug:
    return QVariant::fromValue(item.platformSlug);
  case IconUrl1x1:
    return QVariant::fromValue(item.iconUrl1x1);
  case NumSecondsPlayed:
    return QVariant::fromValue(item.numSecondsPlayed);
  default:
    return QVariant{};
  }
}
QHash<int, QByteArray> GameActivityListModel::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[DisplayName] = "displayName";
  roles[ContentHash] = "contentHash";
  roles[PlatformName] = "platformName";
  roles[PlatformSlug] = "platformSlug";
  roles[IconUrl1x1] = "iconUrl1x1";
  roles[NumSecondsPlayed] = "numSecondsPlayed";
  return roles;
}
} // namespace firelight::activity
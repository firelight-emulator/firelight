#include "entry_list_model.hpp"

#include <firelight/achievement_service.hpp>
#include <firelight/activity/activity_log.hpp>
#include <firelight/library/folder_info.hpp>
#include <firelight/platforms/platform_service.hpp>

#include <QDateTime>
#include <algorithm>
#include <emulation/emulation_service.hpp>
#include <spdlog/spdlog.h>

namespace firelight::library {
EntryListModel::EntryListModel(UserLibraryService &userLibrary, activity::IActivityLog &activityLog,
                               platforms::IPlatformService &platformService,
                               achievements::AchievementService &achievementService, QObject *parent)
    : QAbstractListModel(parent), m_userLibrary(userLibrary), m_activityLog(activityLog),
      m_platformService(platformService), m_achievementService(achievementService) {
  m_gamePlayedConnection = EventDispatcher::instance().subscribe<emulation::EmulationStartedEvent>(
      [this](const emulation::EmulationStartedEvent &event) {
        // Published on the render thread; hop to the GUI thread before
        // touching the model
        const auto contentHash = event.contentHash;
        QMetaObject::invokeMethod(
            this,
            [this, contentHash] {
              for (auto i = 0; i < m_items.size(); ++i) {
                auto &item = m_items[i];
                if (item.entry.contentHash == contentHash) {
                  item.lastPlayedEpochMillis = QDateTime::currentMSecsSinceEpoch();
                  emit dataChanged(createIndex(i, 0), createIndex(i, 0), {LastPlayedAt});
                }
              }
            },
            Qt::QueuedConnection);
      });

  m_countsChangedTimer.setSingleShot(true);
  m_countsChangedTimer.setInterval(0);
  connect(&m_countsChangedTimer, &QTimer::timeout, this, [this] {
    emit countChanged();
    emit numFavoritesChanged();
    emit countByFolderIdChanged();
  });

  m_entryCreatedConnection =
      EventDispatcher::instance().subscribe<EntryCreatedEvent>([this](const EntryCreatedEvent &event) {
        const int id = event.entryId;
        QMetaObject::invokeMethod(this, [this, id] { syncEntry(id); }, Qt::QueuedConnection);
      });
  m_entryUpdatedConnection =
      EventDispatcher::instance().subscribe<EntryUpdatedEvent>([this](const EntryUpdatedEvent &event) {
        const int id = event.entryId;
        QMetaObject::invokeMethod(this, [this, id] { syncEntry(id); }, Qt::QueuedConnection);
      });

  // A finished session may have unlocked achievements; refresh every row's
  // counts (cheap indexed lookups) on the GUI thread
  m_achievementSessionEndedConnection =
      EventDispatcher::instance().subscribe<achievements::AchievementSessionEndedEvent>(
          [this](const achievements::AchievementSessionEndedEvent &) {
            QMetaObject::invokeMethod(this, [this] { refreshAllAchievementCounts(); }, Qt::QueuedConnection);
          });

  // Login completes asynchronously, after this constructor's reset() has
  // already computed counts with no user; recompute when the user arrives
  // (and on logout, which zeroes earned)
  m_userLoggedInConnection = EventDispatcher::instance().subscribe<achievements::UserLoggedInEvent>(
      [this](const achievements::UserLoggedInEvent &) {
        QMetaObject::invokeMethod(this, [this] { refreshAllAchievementCounts(); }, Qt::QueuedConnection);
      });

  reset();
}

QHash<int, QByteArray> EntryListModel::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[Id] = "entryId";
  roles[DisplayName] = "displayName";
  roles[ContentHash] = "contentHash";
  roles[PlatformId] = "platformId";
  roles[PlatformIconName] = "platformIconName";
  roles[ActiveSaveSlot] = "activeSaveSlot";
  roles[Hidden] = "hidden";
  roles[Favorite] = "favorite";
  roles[Icon1x1SourceUrl] = "icon1x1SourceUrl";
  roles[BoxartFrontSourceUrl] = "boxartFrontSourceUrl";
  roles[BoxartBackSourceUrl] = "boxartBackSourceUrl";
  roles[Description] = "description";
  roles[ReleaseYear] = "releaseYear";
  roles[Developer] = "developer";
  roles[Publisher] = "publisher";
  roles[Genres] = "genres";
  roles[RegionIds] = "regionIds";
  roles[FolderIds] = "folderIds";
  roles[ContentDirectoryIds] = "contentDirectoryIds";
  roles[ContentPaths] = "contentPaths";
  roles[CreatedAt] = "createdAt";
  roles[LastPlayedAt] = "lastPlayedAt";
  roles[NumSecondsPlayed] = "numSecondsPlayed";
  roles[AchievementsEarned] = "achievementsEarned";
  roles[AchievementsTotal] = "achievementsTotal";
  roles[AchievementSetCount] = "achievementSetCount";
  return roles;
}

int EntryListModel::rowCount(const QModelIndex &parent) const { return m_items.size(); }

QVariant EntryListModel::data(const QModelIndex &index, int role) const {
  if (role < Qt::UserRole || index.row() >= m_items.size()) {
    return QVariant{};
  }

  const auto item = m_items.at(index.row());

  switch (role) {
  case Id:
    return item.entry.id;
  case DisplayName:
    return QString::fromStdString(item.entry.displayName);
  case ContentHash:
    return QString::fromStdString(item.entry.contentHash);
  case PlatformId:
    return item.entry.platformId;
  case PlatformIconName: {
    auto platform = m_platformService.getPlatform(item.entry.platformId);
    if (!platform.has_value()) {
      return {};
    }

    return QString::fromStdString(platform.value().slug);
  }
  case ActiveSaveSlot:
    return item.entry.activeSaveSlot;
  case Hidden:
    return item.entry.hidden;
  case Favorite:
    return item.entry.favorite;
  case Icon1x1SourceUrl:
    return QString::fromStdString(item.entry.icon1x1SourceUrl);
  case BoxartFrontSourceUrl:
    return QString::fromStdString(item.entry.boxartFrontSourceUrl);
  case BoxartBackSourceUrl:
    return QString::fromStdString(item.entry.boxartBackSourceUrl);
  case Description:
    return QString::fromStdString(item.entry.description);
  case ReleaseYear:
    return item.entry.releaseYear;
  case Developer:
    return QString::fromStdString(item.entry.developer);
  case Publisher:
    return QString::fromStdString(item.entry.publisher);
  case Genres:
    return QString::fromStdString(item.entry.genres);
  case RegionIds:
    return QString::fromStdString(item.entry.regionIds);
  case CreatedAt:
    return QVariant::fromValue(item.entry.createdAt);
  case FolderIds:
    return QVariant::fromValue(QList(item.entry.folderIds.begin(), item.entry.folderIds.end()));
  case ContentDirectoryIds:
    return QVariant::fromValue(QList(item.entry.contentDirectoryIds.begin(), item.entry.contentDirectoryIds.end()));
  case ContentPaths: {
    QStringList paths;
    for (const auto &p : item.entry.contentPaths) {
      paths.append(QString::fromStdString(p));
    }
    return paths;
  }
  case LastPlayedAt:
    return QVariant::fromValue(item.lastPlayedEpochMillis);
  case NumSecondsPlayed:
    return QVariant::fromValue(item.numSecondsPlayed);
  case AchievementsEarned:
    return item.achievementsEarned;
  case AchievementsTotal:
    return item.achievementsTotal;
  case AchievementSetCount:
    return item.achievementSetCount;
  default:
    return QVariant{};
  }
}

bool EntryListModel::setData(const QModelIndex &index, const QVariant &value, int role) {
  if (index.row() < 0 || index.row() >= m_items.size()) {
    return false;
  }

  auto &item = m_items[index.row()];

  switch (role) {
  case DisplayName:
    item.entry.displayName = value.toString().toStdString();
    item.entry.nameUserSet = true;
    break;
  case Favorite:
    spdlog::info("Favorite changed for entry {}: {}", item.entry.id, value.toBool());
    item.entry.favorite = value.toBool();
    emit numFavoritesChanged();
    break;
  default:
    return false;
  }

  emit dataChanged(index, index, {role});
  m_userLibrary.update(item.entry);

  return true;
}

Qt::ItemFlags EntryListModel::flags(const QModelIndex &index) const {
  return QAbstractListModel::flags(index) | Qt::ItemIsEditable;
}

void EntryListModel::addEntryToFolder(int entryId, int folderId) {
  auto libraryEntryInfo = FolderEntryInfo{.folderId = folderId, .entryId = entryId};
  if (!m_userLibrary.create(libraryEntryInfo)) {
    spdlog::error("Failed to add entry {} to folder {}", entryId, folderId);
    return;
  }

  for (auto &item : m_items) {
    if (item.entry.id == entryId) {
      if (std::ranges::find(item.entry.folderIds, folderId) != item.entry.folderIds.end()) {
        spdlog::warn("Entry {} is already in folder {}", entryId, folderId);
        return;
      }

      item.entry.folderIds.push_back(folderId);
      emit dataChanged(createIndex(0, 0), createIndex(m_items.size() - 1, 0), {FolderIds});
      emit countByFolderIdChanged();
      break;
    }
  }
}

void EntryListModel::removeEntryFromFolder(int entryId, int folderId) {
  if (auto entryInfo = FolderEntryInfo{.folderId = folderId, .entryId = entryId};
      !m_userLibrary.deleteFolderEntry(entryInfo)) {
    spdlog::error("Failed to remove entry {} from folder {}", entryId, folderId);
    return;
  }

  for (auto &item : m_items) {
    if (item.entry.id == entryId) {
      auto it = std::ranges::find(item.entry.folderIds, folderId);
      if (it != item.entry.folderIds.end()) {
        item.entry.folderIds.erase(it);
        emit dataChanged(createIndex(0, 0), createIndex(m_items.size() - 1, 0), {FolderIds});
        emit countByFolderIdChanged();
        break;
      }

      spdlog::warn("Entry {} is not in folder {}", entryId, folderId);
      break;
    }
  }
}

void EntryListModel::setEntryFavorite(int entryId, bool favorite) {
  const auto it = m_indexByEntryId.find(entryId);
  if (it == m_indexByEntryId.end()) {
    return;
  }
  const int row = it->second;
  if (row < 0 || row >= m_items.size()) {
    return;
  }
  auto &item = m_items[row];
  if (item.entry.favorite == favorite) {
    return;
  }
  item.entry.favorite = favorite;
  m_userLibrary.update(item.entry);
  emit dataChanged(createIndex(row, 0), createIndex(row, 0), {Favorite});
  emit numFavoritesChanged();
}

int EntryListModel::getCount() const { return m_items.size(); }

int EntryListModel::numFavorites() const {
  return std::ranges::count_if(m_items, [](const auto &item) { return item.entry.favorite; });
}

QVariantMap EntryListModel::getCountByPlatform() const {
  QVariantMap countByPlatform;
  for (const auto &item : m_items) {
    const auto current = countByPlatform[QString::number(item.entry.platformId)].toInt();
    countByPlatform[QString::number(item.entry.platformId)] = current + 1;
  }

  return countByPlatform;
}

QVariantMap EntryListModel::getCountByFolderId() const {
  QVariantMap countByFolderId;

  // Manual folders
  for (const auto &item : m_items) {
    for (const auto &folderId : item.entry.folderIds) {
      auto current = countByFolderId[QString::number(folderId)].toInt();
      countByFolderId[QString::number(folderId)] = current + 1;
    }
  }

  // Smart folders
  for (const auto &folder : m_userLibrary.listFolders()) {
    if (folder.type != static_cast<int>(FolderType::Smart)) {
      continue;
    }
    const auto criteria = SmartFolderCriteria::parse(folder.filterJson);
    int count = 0;

    for (const auto &item : m_items) {
      if (criteria.matches(buildEntryFields(item))) {
        ++count;
      }
    }
    countByFolderId[QString::number(folder.id)] = count;
  }

  return countByFolderId;
}

EntryFields EntryListModel::buildEntryFields(const Item &item) {
  EntryFields fields;
  fields.platformId = static_cast<int>(item.entry.platformId);
  fields.favorite = item.entry.favorite;
  fields.genres = item.entry.genres;
  fields.developer = item.entry.developer;
  fields.publisher = item.entry.publisher;
  fields.releaseYear = static_cast<int>(item.entry.releaseYear);
  fields.contentDirectoryIds = item.entry.contentDirectoryIds;
  fields.contentPaths = item.entry.contentPaths;
  fields.lastPlayedMillis = static_cast<int64_t>(item.lastPlayedEpochMillis);
  fields.secondsPlayed = static_cast<int64_t>(item.numSecondsPlayed);
  return fields;
}

const SmartFolderCriteria &EntryListModel::criteriaForFolder(int folderId) const {
  if (const auto it = m_smartFolderCache.find(folderId); it != m_smartFolderCache.end()) {
    return it->second;
  }

  SmartFolderCriteria criteria;
  for (const auto &folder : m_userLibrary.listFolders()) {
    if (folder.id == folderId) {
      criteria = SmartFolderCriteria::parse(folder.filterJson);
      break;
    }
  }
  return m_smartFolderCache.emplace(folderId, std::move(criteria)).first->second;
}

bool EntryListModel::matchesSmartFolder(int folderId, int entryId) {
  const auto it = m_indexByEntryId.find(entryId);
  if (it == m_indexByEntryId.end()) {
    return false;
  }
  const auto &criteria = criteriaForFolder(folderId);
  return criteria.matches(buildEntryFields(m_items.at(it->second)));
}

void EntryListModel::invalidateSmartFolderCache() {
  m_smartFolderCache.clear();
  emit countByFolderIdChanged();
}

void EntryListModel::reset() {
  emit beginResetModel();
  m_items.clear();
  m_smartFolderCache.clear();
  m_indexByEntryId.clear();

  // Aggregate play stats for the whole library
  struct Stats {
    uint64_t totalMillis = 0;
    uint64_t lastEndMillis = 0;
  };

  std::unordered_map<std::string, Stats> statsByHash;
  for (const auto &session : m_activityLog.getPlaySessions()) {
    auto &s = statsByHash[session.contentHash];
    s.totalMillis += session.unpausedDurationMillis;
    s.lastEndMillis = std::max(s.lastEndMillis, session.endTime);
  }

  for (const auto &entry : m_userLibrary.getEntries(0, 0)) {
    if (!entry.hidden) {
      auto item = Item{.entry = entry};

      if (const auto it = statsByHash.find(entry.contentHash); it != statsByHash.end()) {
        item.numSecondsPlayed = it->second.totalMillis / 1000;
        item.lastPlayedEpochMillis = it->second.lastEndMillis;
      }

      applyAchievementCounts(item);

      m_indexByEntryId[item.entry.id] = static_cast<int>(m_items.size());
      m_items.emplace_back(item);
    }
  }

  emit endResetModel();
  emit countChanged();
  emit numFavoritesChanged();
  emit countByFolderIdChanged();
}

void EntryListModel::removeFolderId(const int folderId) {
  beginResetModel();
  for (auto &item : m_items) {
    auto it = std::ranges::find(item.entry.folderIds, folderId);
    if (it != item.entry.folderIds.end()) {
      item.entry.folderIds.erase(it);
    }
  }
  endResetModel();
}

void EntryListModel::applyPlayStats(Item &item) const {
  uint64_t totalMillis = 0;
  uint64_t lastEndMillis = 0;
  for (const auto &session : m_activityLog.getPlaySessions(item.entry.contentHash)) {
    totalMillis += session.unpausedDurationMillis;
    lastEndMillis = std::max(lastEndMillis, session.endTime);
  }
  item.numSecondsPlayed = totalMillis / 1000;
  item.lastPlayedEpochMillis = lastEndMillis;
}

void EntryListModel::applyAchievementCounts(Item &item) const {
  const auto [earned, total] =
      m_achievementService.getAchievementCounts(item.entry.contentHash, m_achievementService.getLoggedInUsername());
  item.achievementsEarned = earned;
  item.achievementsTotal = total;
  item.achievementSetCount =
      static_cast<int>(m_achievementService.getAchievementSetsForContentHash(item.entry.contentHash).size());
}

void EntryListModel::refreshAllAchievementCounts() {
  for (auto &item : m_items) {
    applyAchievementCounts(item);
  }
  if (!m_items.empty()) {
    emit dataChanged(createIndex(0, 0), createIndex(static_cast<int>(m_items.size()) - 1, 0),
                     {AchievementsEarned, AchievementsTotal, AchievementSetCount});
  }
}

void EntryListModel::rebuildIndex() {
  m_indexByEntryId.clear();
  for (int i = 0; i < m_items.size(); ++i) {
    m_indexByEntryId[m_items[i].entry.id] = i;
  }
}

void EntryListModel::scheduleCountsChanged() { m_countsChangedTimer.start(); }

void EntryListModel::syncEntry(const int entryId) {
  const auto entry = m_userLibrary.getEntry(entryId);
  const auto it = m_indexByEntryId.find(entryId);
  const bool present = it != m_indexByEntryId.end();
  const bool visible = entry.has_value() && !entry->hidden;

  // If gone or hidden, drop it from the model if necessary
  if (!visible) {
    if (present) {
      const int row = it->second;
      beginRemoveRows(QModelIndex(), row, row);
      m_items.removeAt(row);
      rebuildIndex(); // keep the id->row map consistent before the proxy reads
      endRemoveRows();
      scheduleCountsChanged();
    }
    return;
  }

  Item item{.entry = *entry};
  applyPlayStats(item);
  applyAchievementCounts(item);

  // Update the entry in place if it's already present in the model
  if (present) {
    const int row = it->second;
    m_items[row] = item;
    emit dataChanged(createIndex(row, 0), createIndex(row, 0), {});
    scheduleCountsChanged();
    return;
  }

  // Just append new ones
  const int row = static_cast<int>(m_items.size());
  beginInsertRows(QModelIndex(), row, row);
  m_items.append(item);
  m_indexByEntryId[entryId] = row;
  endInsertRows();
  scheduleCountsChanged();
}
} // namespace firelight::library

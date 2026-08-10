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

namespace {
// TODO
// QML reads and writes these as one comma-separated string, so the list shape stops
// at the model boundary
QString joinList(const std::vector<std::string> &values) {
  QStringList parts;

  for (const auto &value : values) {
    parts.append(QString::fromStdString(value));
  }

  return parts.join(QStringLiteral(", "));
}

std::vector<std::string> splitList(const QString &value) {
  std::vector<std::string> values;

  for (const auto &part : value.split(QLatin1Char(','), Qt::SkipEmptyParts)) {
    const auto trimmed = part.trimmed();

    if (!trimmed.isEmpty()) {
      values.push_back(trimmed.toStdString());
    }
  }

  return values;
}
} // namespace

EntryListModel::EntryListModel(UserLibraryService &userLibrary, activity::IActivityLog &activityLog,
                               platforms::IPlatformService &platformService,
                               achievements::AchievementService &achievementService, VariantGroupService &variantGroups,
                               QObject *parent)
    : QAbstractListModel(parent), m_userLibrary(userLibrary), m_activityLog(activityLog),
      m_platformService(platformService), m_achievementService(achievementService), m_variantGroups(variantGroups) {
  m_variantGroupUpdatedConnection =
      EventDispatcher::instance().subscribe<VariantGroupUpdatedEvent>([this](const VariantGroupUpdatedEvent &event) {
        const auto groupId = event.groupId;
        QMetaObject::invokeMethod(this, [this, groupId] { refreshVariantGroup(groupId); }, Qt::QueuedConnection);
      });

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
  roles[Rating] = "rating";
  roles[GroupKey] = "groupKey";
  roles[VariantGroupId] = "variantGroupId";
  roles[VariantCount] = "variantCount";
  roles[IsVariantPrimary] = "isVariantPrimary";
  roles[VariantAutoLaunch] = "variantAutoLaunch";
  roles[Playable] = "playable";
  roles[SearchText] = "searchText";
  return roles;
}

int EntryListModel::rowCount(const QModelIndex &parent) const { return m_items.size(); }

QVariant EntryListModel::data(const QModelIndex &index, int role) const {
  if (role < Qt::UserRole || index.row() >= m_items.size()) {
    return QVariant{};
  }

  const auto &item = m_items.at(index.row());

  switch (role) {
  case Id:
    return item.entry.id;
  case DisplayName:
    // TODO
    // The entry standing for a group is shown under the group's name. Until somebody
    // renames the group the two are the same string, so this only shows through once
    // the name was chosen deliberately
    if (item.isVariantPrimary && !item.variantTitle.isEmpty()) {
      return item.variantTitle;
    }

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
  case Rating:
    return item.entry.rating;
  case GroupKey:
    return item.groupKey;
  case VariantGroupId:
    return item.variantGroupId;
  case VariantCount:
    return item.variantCount;
  case IsVariantPrimary:
    return item.isVariantPrimary;
  case VariantAutoLaunch:
    return item.variantAutoLaunch;
  case Playable:
    return m_playablePlatformIds.contains(static_cast<int>(item.entry.platformId));
  case SearchText:
    return item.searchText;
  case Icon1x1SourceUrl:
    return QString::fromStdString(item.entry.icon1x1SourceUrl);
  case BoxartFrontSourceUrl:
    return QString::fromStdString(item.entry.boxartFrontSourceUrl);
  case BoxartBackSourceUrl:
    return QString::fromStdString(item.entry.boxartBackSourceUrl);
  case Description:
    return QString::fromStdString(item.entry.metadata.description);
  case ReleaseYear:
    return item.entry.metadata.releaseYear;
  case Developer:
    return QString::fromStdString(item.entry.metadata.developer);
  case Publisher:
    return QString::fromStdString(item.entry.metadata.publisher);
  case Genres:
    return joinList(item.entry.metadata.genres);
  case RegionIds:
    return joinList(item.entry.metadata.regions);
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
    return QVariant::fromValue(item.variantLastPlayedMillis);
  case NumSecondsPlayed:
    return QVariant::fromValue(item.variantSecondsPlayed);
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

  // TODO
  // A metadata field goes through applyEntryMetadata so the write merges into the
  // stored document and records that the user set it; the rest go through update()
  const char *changedField = nullptr;

  switch (role) {
  case DisplayName:
    // TODO
    // The row is showing the group's name, so that is what a rename edits
    if (item.isVariantPrimary && item.variantGroupId != -1) {
      m_variantGroups.setTitle(item.variantGroupId, value.toString().toStdString());
      return true;
    }

    item.entry.displayName = value.toString().toStdString();
    item.entry.nameUserSet = true;
    break;
  case Favorite:
    spdlog::info("Favorite changed for entry {}: {}", item.entry.id, value.toBool());
    item.entry.favorite = value.toBool();
    emit numFavoritesChanged();
    break;
  case Rating:
    item.entry.rating = value.toUInt();
    break;
  case ActiveSaveSlot:
    item.entry.activeSaveSlot = value.toUInt();
    break;
  case Description:
    item.entry.metadata.description = value.toString().toStdString();
    changedField = metadata_fields::DESCRIPTION;
    break;
  case Developer:
    item.entry.metadata.developer = value.toString().toStdString();
    changedField = metadata_fields::DEVELOPER;
    break;
  case Publisher:
    item.entry.metadata.publisher = value.toString().toStdString();
    changedField = metadata_fields::PUBLISHER;
    break;
  case ReleaseYear:
    item.entry.metadata.releaseYear = value.toUInt();
    changedField = metadata_fields::RELEASE_YEAR;
    break;
  case Genres:
    item.entry.metadata.genres = splitList(value.toString());
    changedField = metadata_fields::GENRES;
    break;
  default:
    return false;
  }

  emit dataChanged(index, index, {role});
  if (changedField != nullptr) {
    item.entry.metadataOverrides.markUserSet(changedField);
    m_userLibrary.applyEntryMetadata(item.entry.id, item.entry.metadata, {changedField}, true);
  } else {
    m_userLibrary.update(item.entry);
  }

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

QString EntryListModel::getGroupMode() const { return m_groupMode; }

void EntryListModel::setGroupMode(const QString &mode) {
  if (m_groupMode == mode) {
    return;
  }
  m_groupMode = mode;
  emit groupModeChanged();
  // Recompute the cached key once per row for the new mode, then notify so the
  // proxy re-sorts and the section headers re-evaluate
  for (auto &item : m_items) {
    item.groupKey = computeGroupKey(item);
  }
  if (!m_items.empty()) {
    emit dataChanged(createIndex(0, 0), createIndex(static_cast<int>(m_items.size()) - 1, 0), {GroupKey});
  }
}

QString EntryListModel::computeGroupKey(const Item &item) const {
  if (m_groupMode == "platform") {
    const auto platform = m_platformService.getPlatform(item.entry.platformId);
    return platform.has_value() ? QString::fromStdString(platform->name) : QStringLiteral("Unknown platform");
  }
  if (m_groupMode == "decade") {
    if (item.entry.metadata.releaseYear == 0) {
      return QStringLiteral("Unknown");
    }
    return QString::number((item.entry.metadata.releaseYear / 10) * 10) + QStringLiteral("s");
  }
  if (m_groupMode == "year") {
    return item.entry.metadata.releaseYear == 0 ? QStringLiteral("Unknown")
                                                : QString::number(item.entry.metadata.releaseYear);
  }
  if (m_groupMode == "genre") {
    if (item.entry.metadata.genres.empty()) {
      return QStringLiteral("No genre");
    }
    return QString::fromStdString(item.entry.metadata.genres.front()).trimmed();
  }
  if (m_groupMode == "title") {
    if (item.entry.displayName.empty()) {
      return QStringLiteral("#");
    }
    const QChar first = QString::fromStdString(item.entry.displayName).at(0).toUpper();
    return first.isLetter() ? QString(first) : QStringLiteral("#");
  }
  return {};
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
  fields.genres = item.entry.metadata.genres;
  fields.developer = item.entry.metadata.developer;
  fields.publisher = item.entry.metadata.publisher;
  fields.releaseYear = static_cast<int>(item.entry.metadata.releaseYear);
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

void EntryListModel::applyVariantGrouping() {
  m_entryIdsByGroup.clear();

  const auto groups = m_userLibrary.getVariantGroups();

  if (groups.empty()) {
    for (auto &item : m_items) {
      item.variantGroupId = -1;
      item.variantCount = 1;
      item.isVariantPrimary = true;
      item.variantAutoLaunch = false;
      item.variantTitle.clear();
      item.variantSecondsPlayed = item.numSecondsPlayed;
      item.variantLastPlayedMillis = item.lastPlayedEpochMillis;
      item.searchText = computeSearchText(item);
    }

    return;
  }

  std::vector<Entry> entries;
  entries.reserve(m_items.size());

  for (const auto &item : m_items) {
    entries.push_back(item.entry);
  }

  const auto resolution = m_variantGroups.resolveAll(entries, groups);

  std::unordered_map<int, const VariantGroup *> groupsById;
  for (const auto &group : groups) {
    groupsById[group.id] = &group;
  }

  struct Totals {
    uint64_t seconds = 0;
    uint64_t lastPlayed = 0;
  };

  // TODO
  // Only the rows count. A hidden entry is one the user took out of their library, so it
  // stops counting toward what the group played
  std::unordered_map<int, Totals> totalsByGroup;

  for (auto &item : m_items) {
    const auto groupId = item.entry.variantGroupId.value_or(-1);
    const auto group = groupsById.find(groupId);

    if (groupId == -1 || group == groupsById.end()) {
      item.variantGroupId = -1;
      item.variantCount = 1;
      item.isVariantPrimary = true;
      item.variantAutoLaunch = false;
      item.variantTitle.clear();
      item.variantSecondsPlayed = item.numSecondsPlayed;
      item.variantLastPlayedMillis = item.lastPlayedEpochMillis;
      continue;
    }

    const auto primary = resolution.primaryByGroup.find(groupId);
    const auto count = resolution.memberCountByGroup.find(groupId);

    item.variantGroupId = groupId;
    item.variantCount = count == resolution.memberCountByGroup.end() ? 1 : count->second;
    item.isVariantPrimary = primary != resolution.primaryByGroup.end() && primary->second == item.entry.id;
    item.variantAutoLaunch = group->second->autoLaunchPrimary;
    item.variantTitle = QString::fromStdString(group->second->title);

    m_entryIdsByGroup[groupId].push_back(item.entry.id);

    auto &totals = totalsByGroup[groupId];
    totals.seconds += item.numSecondsPlayed;
    totals.lastPlayed = std::max(totals.lastPlayed, item.lastPlayedEpochMillis);
  }

  for (auto &item : m_items) {
    if (item.variantGroupId != -1) {
      const auto &totals = totalsByGroup[item.variantGroupId];
      item.variantSecondsPlayed = totals.seconds;
      item.variantLastPlayedMillis = totals.lastPlayed;
    }

    item.searchText = computeSearchText(item);
  }
}

QString EntryListModel::computeSearchText(const Item &item) const {
  QStringList parts;
  parts.append(QString::fromStdString(item.entry.displayName));

  if (!item.variantTitle.isEmpty()) {
    parts.append(item.variantTitle);
  }

  if (item.isVariantPrimary && item.variantGroupId != -1) {
    const auto members = m_entryIdsByGroup.find(item.variantGroupId);

    if (members != m_entryIdsByGroup.end()) {
      for (const auto memberId : members->second) {
        const auto index = m_indexByEntryId.find(memberId);

        if (index != m_indexByEntryId.end()) {
          parts.append(QString::fromStdString(m_items.at(index->second).entry.displayName));
        }
      }
    }
  }

  return parts.join(QLatin1Char('\n')).toLower();
}

void EntryListModel::refreshVariantGroup(const int groupId) {
  std::vector<int> affected;

  for (auto i = 0; i < m_items.size(); ++i) {
    if (m_items.at(i).variantGroupId == groupId || m_items.at(i).entry.variantGroupId.value_or(-1) == groupId) {
      affected.push_back(i);
    }
  }

  applyVariantGrouping();

  // TODO
  // Rows that joined or left the group in this pass are told about too, so the proxy
  // re-tests the one that stopped standing for the group as well as the one that started
  for (auto i = 0; i < m_items.size(); ++i) {
    if (m_items.at(i).variantGroupId == groupId && std::find(affected.begin(), affected.end(), i) == affected.end()) {
      affected.push_back(i);
    }
  }

  for (const auto row : affected) {
    emit dataChanged(createIndex(row, 0), createIndex(row, 0), {});
  }
}

void EntryListModel::refreshPlayablePlatforms() {
  m_playablePlatformIds.clear();

  for (const auto &platform : m_platformService.listPlatforms()) {
    if (CoreRegistry::instance().isPlatformPlayable(static_cast<int>(platform.id))) {
      m_playablePlatformIds.insert(platform.id);
    }
  }
}

void EntryListModel::reset() {
  refreshPlayablePlatforms();

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
    s.lastEndMillis = std::max(s.lastEndMillis, session.endedAt);
  }

  for (const auto &entry : m_userLibrary.getEntries(0, 0)) {
    if (!entry.hidden) {
      auto item = Item{.entry = entry};

      if (const auto it = statsByHash.find(entry.contentHash); it != statsByHash.end()) {
        item.numSecondsPlayed = it->second.totalMillis / 1000;
        item.lastPlayedEpochMillis = it->second.lastEndMillis;
      }

      applyAchievementCounts(item);
      item.groupKey = computeGroupKey(item);

      m_indexByEntryId[item.entry.id] = static_cast<int>(m_items.size());
      m_items.emplace_back(item);
    }
  }

  applyVariantGrouping();

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
    lastEndMillis = std::max(lastEndMillis, session.endedAt);
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
      const auto leftGroup = m_items.at(row).variantGroupId;

      beginRemoveRows(QModelIndex(), row, row);
      m_items.removeAt(row);
      rebuildIndex(); // keep the id->row map consistent before the proxy reads
      endRemoveRows();

      if (leftGroup != -1) {
        refreshVariantGroup(leftGroup);
      }

      scheduleCountsChanged();
    }
    return;
  }

  Item item{.entry = *entry};
  applyPlayStats(item);
  applyAchievementCounts(item);
  item.groupKey = computeGroupKey(item);

  const auto joinedGroup = entry->variantGroupId.value_or(-1);

  // Update the entry in place if it's already present in the model
  if (present) {
    const int row = it->second;
    const auto leftGroup = m_items.at(row).variantGroupId;
    m_items[row] = item;

    applyVariantGrouping();
    emit dataChanged(createIndex(row, 0), createIndex(row, 0), {});

    if (leftGroup != -1 && leftGroup != joinedGroup) {
      refreshVariantGroup(leftGroup);
    }

    if (joinedGroup != -1) {
      refreshVariantGroup(joinedGroup);
    }

    scheduleCountsChanged();
    return;
  }

  // Just append new ones
  const int row = static_cast<int>(m_items.size());
  beginInsertRows(QModelIndex(), row, row);
  m_items.append(item);
  m_indexByEntryId[entryId] = row;
  endInsertRows();

  if (joinedGroup != -1) {
    refreshVariantGroup(joinedGroup);
  } else {
    applyVariantGrouping();
  }

  scheduleCountsChanged();
}
} // namespace firelight::library

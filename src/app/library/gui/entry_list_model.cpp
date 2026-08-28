// TODO: NEEDS REVIEW
#include "entry_list_model.hpp"

#include <firelight/achievement_service.hpp>
#include <firelight/activity/activity_log.hpp>
#include <firelight/library/folder_info.hpp>
#include <firelight/library/user_library_repository.hpp>
#include <firelight/platforms/platform_service.hpp>

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <algorithm>
#include <emulation/emulation_service.hpp>
#include <spdlog/spdlog.h>

namespace firelight::library {

namespace {
// Everything a sentence needs that a bare problem cannot carry. Empty where it is unknown, and
// every sentence below falls back to its plain form rather than naming a blank
struct StatusDetail {
  QString platformName;
  QString missingBios;
  QString unreachableRoot;
  QList<int> missingDiscs;
  // TODO
  // Where the files were when they were last seen
  QString lastKnownPath;
  // TODO
  // Whether the folder they were under was taken out of the library, rather than the files
  // themselves going
  bool wasFolderRemoved = false;
};

// Reads as a sentence rather than a list: "1", "1 and 2", "1, 2 and 4"
QString joinNumbers(const QList<int> &numbers) {
  QStringList parts;
  for (const auto number : numbers) {
    parts << QString::number(number);
  }

  if (parts.size() < 2) {
    return parts.join(QString());
  }

  const auto last = parts.takeLast();
  return QObject::tr("%1 and %2").arg(parts.join(QStringLiteral(", ")), last);
}

// What is wrong, in the order somebody can act on it. Built here rather than in QML because
// assembling a sentence from a list of numbers is the view doing the model's job
QString statusText(const EntryStatus &status, const StatusDetail &detail) {
  QStringList sentences;

  for (const auto problem : status.problems) {
    switch (problem) {
    // Both mean the same thing to somebody looking at the grid, and neither is theirs to fix
    // today: no core can be installed by hand yet
    case EntryProblem::PlatformNotSupported:
    case EntryProblem::CoreNotInstalled:
      sentences << (detail.platformName.isEmpty()
                        ? QObject::tr("Launching this game is not yet supported.")
                        : QObject::tr("Launching %1 games is not yet supported.").arg(detail.platformName));
      break;
    case EntryProblem::BiosMissing:
      sentences << (detail.missingBios.isEmpty()
                        ? QObject::tr("This system needs a BIOS file that Firelight can't provide.")
                        : QObject::tr("This system needs a BIOS file that Firelight can't provide: %1.")
                              .arg(detail.missingBios));
      break;
    case EntryProblem::ContentUnavailable:
      sentences
          << (detail.unreachableRoot.isEmpty()
                  ? QObject::tr("This game's folder isn't available right now.")
                  : QObject::tr("This game is in %1, which isn't available right now.").arg(detail.unreachableRoot));
      break;
    case EntryProblem::FilesMissing:
      if (detail.lastKnownPath.isEmpty()) {
        sentences << QObject::tr("The files for this game are missing.");
      } else if (detail.wasFolderRemoved) {
        sentences << QObject::tr("This game was in %1, which you removed from your library.").arg(detail.lastKnownPath);
      } else {
        sentences << QObject::tr("The files for this game are missing. They were in %1.").arg(detail.lastKnownPath);
      }
      break;
    case EntryProblem::ContentInArchive:
      sentences << QObject::tr("This disc is inside an archive and has to be extracted first.");
      break;
    case EntryProblem::DiscsMissing:
      if (detail.missingDiscs.isEmpty()) {
        sentences << QObject::tr("Some of this game's discs are missing.");
      } else if (detail.missingDiscs.size() == 1) {
        sentences << QObject::tr("Disc %1 is missing.").arg(detail.missingDiscs.front());
      } else {
        sentences << QObject::tr("Discs %1 are missing.").arg(joinNumbers(detail.missingDiscs));
      }
      break;
    }
  }

  return sentences.join(QStringLiteral(" "));
}

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
                               achievements::AchievementService &achievementService,
                               settings::SettingsService &settings, QObject *parent)
    : QAbstractListModel(parent), m_userLibrary(userLibrary), m_activityLog(activityLog),
      m_platformService(platformService), m_achievementService(achievementService), m_settings(settings) {
  // Pointing a platform at a different core changes what every game on it can do
  m_coreSettingChangedConnection = EventDispatcher::instance().subscribe<settings::PlatformSettingChangedEvent>(
      [this](const settings::PlatformSettingChangedEvent &event) {
        if (event.key != CoreRegistry::CORE_SETTING_KEY) {
          return;
        }

        QMetaObject::invokeMethod(this, [this] { refreshStatuses(); }, Qt::QueuedConnection);
      });

  m_coreSettingResetConnection = EventDispatcher::instance().subscribe<settings::PlatformSettingResetEvent>(
      [this](const settings::PlatformSettingResetEvent &event) {
        if (event.key != CoreRegistry::CORE_SETTING_KEY) {
          return;
        }

        QMetaObject::invokeMethod(this, [this] { refreshStatuses(); }, Qt::QueuedConnection);
      });

  // TODO
  // A folder's criteria are cached, so an edit anywhere has to reach the cache without the view
  // being asked to poke it
  m_folderChangedConnection =
      EventDispatcher::instance().subscribe<FolderChangedEvent>([this](const FolderChangedEvent &) {
        QMetaObject::invokeMethod(
            this,
            [this] {
              invalidateSmartFolderCache();
              emit countByFolderIdChanged();
            },
            Qt::QueuedConnection);
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

  // Folding a disc set destroys the entries it absorbs, and syncEntry only takes a row away for an
  // id it is handed
  m_entryDeletedConnection =
      EventDispatcher::instance().subscribe<EntryDeletedEvent>([this](const EntryDeletedEvent &event) {
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
  roles[Problems] = "problems";
  roles[StatusText] = "statusText";
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
    return item.status.isPlayable();
  case Problems: {
    QList<int> problems;

    for (const auto problem : item.status.problems) {
      problems.append(static_cast<int>(problem));
    }

    return QVariant::fromValue(problems);
  }
  case StatusText:
    return item.statusText;
  case SearchText:
    return item.searchText;
  case FilterFields:
    return QVariant::fromValue(&item.fields);
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

  // TODO
  // The row derives searchText and its filter fields from what was just written, so they are
  // rebuilt before anyone is told the row changed
  item.searchText = computeSearchText(item);
  item.fields = buildEntryFields(item);

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
  auto libraryEntryInfo = FolderEntry{.folderId = folderId, .entryId = entryId};
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
      item.fields = buildEntryFields(item);
      emit dataChanged(createIndex(0, 0), createIndex(m_items.size() - 1, 0), {FolderIds});
      emit countByFolderIdChanged();
      break;
    }
  }
}

void EntryListModel::removeEntryFromFolder(int entryId, int folderId) {
  if (auto entryInfo = FolderEntry{.folderId = folderId, .entryId = entryId};
      !m_userLibrary.deleteFolderEntry(entryInfo)) {
    spdlog::error("Failed to remove entry {} from folder {}", entryId, folderId);
    return;
  }

  for (auto &item : m_items) {
    if (item.entry.id == entryId) {
      auto it = std::ranges::find(item.entry.folderIds, folderId);
      if (it != item.entry.folderIds.end()) {
        item.entry.folderIds.erase(it);
        item.fields = buildEntryFields(item);
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
  item.fields = buildEntryFields(item);
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

    const auto &criteria = criteriaForFolder(folder.id);
    int count = 0;

    for (const auto &item : m_items) {
      if (criteria.matches(item.fields)) {
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
  fields.searchText = item.searchText.toStdString();
  fields.playable = item.status.isPlayable();
  fields.folderIds = item.entry.folderIds;
  return fields;
}

const SmartFolderCriteria &EntryListModel::criteriaForFolder(int folderId) const {
  if (const auto it = m_smartFolderCache.find(folderId); it != m_smartFolderCache.end()) {
    return it->second;
  }

  // TODO
  // Every folder is parsed on the first miss, because reaching the one asked for costs the same
  // listFolders() call as reaching all of them
  for (const auto &folder : m_userLibrary.listFolders()) {
    m_smartFolderCache.insert_or_assign(folder.id, SmartFolderCriteria::parse(folder.filterJson));
  }

  return m_smartFolderCache.emplace(folderId, SmartFolderCriteria{}).first->second;
}

void EntryListModel::invalidateSmartFolderCache() {
  m_smartFolderCache.clear();
  emit countByFolderIdChanged();
}

// TODO
// Rebuilds what every row derives rather than stores. Variant grouping is not wired up, so each
// row stands for itself: its own name, its own play stats, and nothing folded away behind it
void EntryListModel::refreshRowFields() {
  for (auto &item : m_items) {
    item.variantGroupId = -1;
    item.variantCount = 1;
    item.isVariantPrimary = true;
    item.variantAutoLaunch = false;
    item.variantTitle.clear();
    item.variantSecondsPlayed = item.numSecondsPlayed;
    item.variantLastPlayedMillis = item.lastPlayedEpochMillis;
    item.searchText = computeSearchText(item);
    item.fields = buildEntryFields(item);
  }
}

QString EntryListModel::computeSearchText(const Item &item) const {
  return QString::fromStdString(item.entry.displayName).toLower();
}

void EntryListModel::refreshStatuses() {
  refreshPlatformProblems();

  for (auto &item : m_items) {
    applyStatus(item);
  }

  if (!m_items.empty()) {
    emit dataChanged(createIndex(0, 0), createIndex(static_cast<int>(m_items.size()) - 1, 0),
                     {Playable, Problems, StatusText});
  }
}

void EntryListModel::refreshPlatformProblems() {
  m_problemByPlatformId.clear();
  m_platformNameById.clear();
  m_missingBiosByPlatformId.clear();
  m_unreachableRoots.clear();
  m_knownContentDirectoryIds.clear();

  // TODO
  // A root that cannot be read makes every game under it unplayable for as long as that lasts,
  // which is a folder fact rather than an entry one
  for (const auto &directory : m_userLibrary.getContentDirectories()) {
    const auto path = QString::fromStdString(directory.path);
    m_knownContentDirectoryIds.insert(directory.id);

    if (!QFileInfo::exists(path)) {
      m_unreachableRoots.append(QDir::cleanPath(path));
    }
  }

  for (const auto &platform : m_platformService.listPlatforms()) {
    const auto platformId = static_cast<int>(platform.id);
    m_platformNameById.insert(platformId, QString::fromStdString(platform.name));

    const auto problem = CoreRegistry::instance().coreProblemForPlatform(platformId, &m_settings);
    if (!problem) {
      continue;
    }

    m_problemByPlatformId.insert(platformId, *problem);

    if (*problem != EntryProblem::BiosMissing) {
      continue;
    }

    // Only the ones actually standing in the way. An optional file is never the reason a game
    // will not start, so naming it here would send somebody after the wrong thing
    QStringList names;
    for (const auto &bios : CoreRegistry::instance().biosStatusForPlatform(platformId, &m_settings)) {
      if (bios.isSatisfied) {
        continue;
      }

      for (const auto &filename : bios.requirement.filenames) {
        names << QString::fromStdString(filename);
      }
    }

    m_missingBiosByPlatformId.insert(platformId, names.join(QStringLiteral(", ")));
  }
}

EntryStatus EntryListModel::statusOf(const Entry &entry) const {
  EntryStatusFacts facts;
  facts.hasWayIn = entry.isContentAvailable;

  if (const auto problem = m_problemByPlatformId.constFind(static_cast<int>(entry.platformId));
      problem != m_problemByPlatformId.constEnd()) {
    facts.isCoreRegistered = *problem != EntryProblem::PlatformNotSupported;
    facts.isCoreInstalled = *problem != EntryProblem::CoreNotInstalled;
    facts.hasRequiredBios = *problem != EntryProblem::BiosMissing;
  }

  facts.isContentReachable = unreachableRootFor(entry).isEmpty();

  if (entry.discSetId.has_value()) {
    const auto discs = m_userLibrary.getPresentDiscsInSet(*entry.discSetId);
    facts.presentDiscCount = static_cast<int>(discs.size());
    if (const auto set = m_userLibrary.getDiscSet(*entry.discSetId)) {
      facts.expectedDiscCount = set->discCount;
    }

    facts.isDiscInArchive = std::ranges::any_of(discs, [](const ContentFile &disc) { return disc.m_inArchive; });
  }

  return evaluateEntryStatus(facts);
}

// TODO
// Both halves in one place: the verdict and the sentence were assigned separately at three call
// sites, and a new one only updated the first
void EntryListModel::applyStatus(Item &item) const {
  item.status = statusOf(item.entry);
  item.statusText = describeStatus(item.entry, item.status);
}

QString EntryListModel::unreachableRootFor(const Entry &entry) const {
  if (m_unreachableRoots.isEmpty() || entry.readableContentPaths.empty()) {
    return {};
  }

  QString behindAnUnreachableRoot;

  // One copy on an unplugged drive says nothing while another sits on a disk that is right here,
  // so this answers only when every copy still on disk is somewhere that cannot be read
  for (const auto &path : entry.readableContentPaths) {
    const auto candidate = QDir::cleanPath(QString::fromStdString(path));
    auto isBehindOne = false;

    for (const auto &root : m_unreachableRoots) {
      // A separator on the end, so D:/Games does not claim D:/GamesArchive
      if (candidate == root || candidate.startsWith(root + QLatin1Char('/'))) {
        isBehindOne = true;
        behindAnUnreachableRoot = root;
        break;
      }
    }

    if (!isBehindOne) {
      return {};
    }
  }

  return behindAnUnreachableRoot;
}

QString EntryListModel::describeStatus(const Entry &entry, const EntryStatus &status) const {
  const auto platformId = static_cast<int>(entry.platformId);

  StatusDetail detail;
  detail.platformName = m_platformNameById.value(platformId);
  detail.missingBios = m_missingBiosByPlatformId.value(platformId);
  detail.unreachableRoot = QDir::toNativeSeparators(unreachableRootFor(entry));

  // TODO
  // The rows outlive the files, so the path a game had is still there to be named. Which of the
  // two sentences it gets turns on whether the folder it sat under is still in the library
  if (!entry.contentPaths.empty()) {
    detail.lastKnownPath = QDir::toNativeSeparators(QString::fromStdString(entry.contentPaths.front()));
    detail.wasFolderRemoved = std::ranges::any_of(entry.contentDirectoryIds, [this](const int id) {
      return id != -1 && !m_knownContentDirectoryIds.contains(id);
    });
  }

  if (entry.discSetId.has_value() &&
      std::ranges::any_of(status.problems, [](const EntryProblem p) { return p == EntryProblem::DiscsMissing; })) {
    std::vector<int> present;
    for (const auto &disc : m_userLibrary.getPresentDiscsInSet(*entry.discSetId)) {
      present.push_back(disc.m_discNumber);
    }

    const auto set = m_userLibrary.getDiscSet(*entry.discSetId);
    for (const auto number : missingDiscs(present, set ? set->discCount : 0)) {
      detail.missingDiscs.append(number);
    }
  }

  return statusText(status, detail);
}

void EntryListModel::reset() {
  refreshPlatformProblems();

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

  // A game whose files are gone stays in the library and is badged. Taking the row away is
  // what left somebody unable to find out why their game had disappeared
  for (const auto &entry : m_userLibrary.getEntries()) {
    auto item = Item{.entry = entry};

    if (const auto it = statsByHash.find(entry.contentHash); it != statsByHash.end()) {
      item.numSecondsPlayed = it->second.totalMillis / 1000;
      item.lastPlayedEpochMillis = it->second.lastEndMillis;
    }

    applyAchievementCounts(item);
    applyStatus(item);
    item.groupKey = computeGroupKey(item);

    m_indexByEntryId[item.entry.id] = static_cast<int>(m_items.size());
    m_items.emplace_back(item);
  }

  refreshRowFields();

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
  const bool visible = entry.has_value();

  // Only a row that is genuinely gone leaves the model
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
  applyStatus(item);
  item.groupKey = computeGroupKey(item);

  // Update the entry in place if it's already present in the model
  if (present) {
    const int row = it->second;
    m_items[row] = item;

    refreshRowFields();
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

  refreshRowFields();

  scheduleCountsChanged();
}
} // namespace firelight::library

#include "library_entry_sort_filter_model.hpp"

#include <QDateTime>
#include <algorithm>
#include <spdlog/spdlog.h>

namespace firelight::gui {

LibraryEntrySortFilterModel::LibraryEntrySortFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent), m_filter(new LibraryFilter(this)) {
  connect(m_filter, &LibraryFilter::changed, this, [this] {
    emit filtersOrSortChanged();

    if (!m_adoptingFolder) {
      emit refinementChanged();
    }
  });

  setSortCaseSensitivity(Qt::CaseInsensitive);
  QSortFilterProxyModel::setSortRole(m_pendingSortRole);
  setDynamicSortFilter(true);
  sort(0, Qt::AscendingOrder);

  connect(this, &QAbstractItemModel::rowsInserted, this, &LibraryEntrySortFilterModel::countChanged);
  connect(this, &QAbstractItemModel::rowsRemoved, this, &LibraryEntrySortFilterModel::countChanged);
  connect(this, &QAbstractItemModel::modelReset, this, &LibraryEntrySortFilterModel::countChanged);
}

QAbstractListModel *LibraryEntrySortFilterModel::getSourceModel() const { return m_sourceModel; }

void LibraryEntrySortFilterModel::setSourceModel(QAbstractListModel *sourceModel) {
  if (m_sourceModel == sourceModel) {
    return;
  }

  auto *entryModel = qobject_cast<library::EntryListModel *>(sourceModel);

  if (sourceModel != nullptr && entryModel == nullptr) {
    spdlog::error("LibraryEntrySortFilterModel source must be an EntryListModel");
    return;
  }

  m_sourceModel = entryModel;
  QSortFilterProxyModel::setSourceModel(entryModel);

  if (m_sourceModel != nullptr) {
    connect(m_sourceModel, &library::EntryListModel::countChanged, this,
            &LibraryEntrySortFilterModel::countByPlatformChanged);
  }

  emit sourceModelChanged();
  emit filtersOrSortChanged();
  applyFilters();
  emit countChanged();
}

int LibraryEntrySortFilterModel::getCount() const { return rowCount({}); }

LibraryEntrySortFilterModel::SortRole LibraryEntrySortFilterModel::getSortRole() const { return m_pendingSortRole; }

void LibraryEntrySortFilterModel::setSortRole(const SortRole sortRole) {
  if (m_pendingSortRole == sortRole) {
    return;
  }

  m_pendingSortRole = sortRole;
  pinSortToOpenFolder();
  emit sortRoleChanged();
  emit filtersOrSortChanged();
  emit refinementChanged();
}

QString LibraryEntrySortFilterModel::getSortDisplayName() const {
  for (const auto &option : getSortOptions()) {
    if (option.toMap().value("value").toInt() == m_pendingSortRole) {
      return option.toMap().value("text").toString();
    }
  }

  return {};
}

bool LibraryEntrySortFilterModel::isSortAscending() const { return m_pendingSortAscending; }

void LibraryEntrySortFilterModel::setSortAscending(const bool sortAscending) {
  if (m_pendingSortAscending == sortAscending) {
    return;
  }

  m_pendingSortAscending = sortAscending;
  pinSortToOpenFolder();
  emit sortAscendingChanged();
  emit filtersOrSortChanged();
  emit refinementChanged();
}

bool LibraryEntrySortFilterModel::anyFiltersActive() const { return !m_filter->isEmpty(); }

LibraryFilter *LibraryEntrySortFilterModel::getFilter() const { return m_filter; }

bool LibraryEntrySortFilterModel::isPending() const {
  return m_filter->getCriteria() != m_appliedCriteria || m_scopeFolderId != m_appliedScopeFolderId ||
         sortRole() != m_pendingSortRole || (sortOrder() == Qt::AscendingOrder) != m_pendingSortAscending;
}

int LibraryEntrySortFilterModel::getScopeFolderId() const { return m_scopeFolderId; }

void LibraryEntrySortFilterModel::setScopeFolderId(const int scopeFolderId) {
  if (m_scopeFolderId == scopeFolderId) {
    return;
  }

  m_scopeFolderId = scopeFolderId;
  emit filtersOrSortChanged();
}

LibraryFolderListModel *LibraryEntrySortFilterModel::getFolderModel() const { return m_folderModel; }

void LibraryEntrySortFilterModel::setFolderModel(LibraryFolderListModel *folderModel) {
  if (m_folderModel == folderModel) {
    return;
  }

  m_folderModel = folderModel;
  emit folderModelChanged();

  if (m_folderModel != nullptr) {
    // A collection adopted before its row was cached reads as manual; adopting again once the row
    // is there is the only case that re-reads it, so a refinement is never thrown away
    connect(m_folderModel, &QAbstractItemModel::rowsInserted, this, [this] {
      if (m_openFolderId != -1 && m_scopeFolderId == m_openFolderId && isOpenFolderSmart()) {
        setOpenFolderId(m_openFolderId);
      }
    });
  }

  if (m_openFolderId != -1) {
    setOpenFolderId(m_openFolderId);
  }
}

int LibraryEntrySortFilterModel::getOpenFolderId() const { return m_openFolderId; }

const library::FolderInfo *LibraryEntrySortFilterModel::openFolder() const {
  if (m_openFolderId == -1 || m_folderModel == nullptr) {
    return nullptr;
  }

  return m_folderModel->findFolder(m_openFolderId);
}

bool LibraryEntrySortFilterModel::isOpenFolderSmart() const {
  const auto *folder = openFolder();

  return folder != nullptr && folder->type == static_cast<int>(library::FolderType::Smart);
}

bool LibraryEntrySortFilterModel::isSortPinnedToOpenFolder() const {
  const auto *folder = openFolder();

  return folder != nullptr && !folder->sortRole.empty();
}

QString LibraryEntrySortFilterModel::sortRoleName(const SortRole sortRole) {
  for (const auto &option : getSortOptions()) {
    if (option.toMap().value("value").toInt() == sortRole) {
      return option.toMap().value("role").toString();
    }
  }

  return {};
}

LibraryEntrySortFilterModel::SortRole LibraryEntrySortFilterModel::sortRoleForName(const QString &roleName) {
  for (const auto &option : getSortOptions()) {
    if (option.toMap().value("role").toString() == roleName) {
      return static_cast<SortRole>(option.toMap().value("value").toInt());
    }
  }

  return DisplayName;
}

void LibraryEntrySortFilterModel::pinSortToOpenFolder() {
  if (m_adoptingFolder || m_openFolderId == -1 || m_folderModel == nullptr) {
    return;
  }

  m_folderModel->setFolderSort(m_openFolderId, sortRoleName(m_pendingSortRole), m_pendingSortAscending);
  emit sortPinnedChanged();
}

void LibraryEntrySortFilterModel::setSortPinnedToOpenFolder(const bool pinned) {
  if (pinned || m_openFolderId == -1 || m_folderModel == nullptr) {
    return;
  }

  m_folderModel->setFolderSort(m_openFolderId, {}, true);

  m_adoptingFolder = true;
  m_pendingSortRole = DisplayName;
  m_pendingSortAscending = true;
  m_adoptingFolder = false;

  applyFilters();

  emit sortRoleChanged();
  emit sortAscendingChanged();
  emit sortPinnedChanged();
  emit filtersOrSortChanged();
}

void LibraryEntrySortFilterModel::resetToSaved() { setOpenFolderId(m_openFolderId); }

void LibraryEntrySortFilterModel::setOpenFolderId(const int folderId) {
  m_openFolderId = folderId;

  const auto *folder = openFolder();
  const auto hasPinnedSort = folder != nullptr && !folder->sortRole.empty();

  m_adoptingFolder = true;

  if (isOpenFolderSmart()) {
    m_scopeFolderId = -1;
    m_filter->setJson(QString::fromStdString(folder->filterJson));
    m_filter->setSavedJson(QString::fromStdString(folder->filterJson));
  } else {
    m_scopeFolderId = folderId;
    m_filter->clear();
    m_filter->setSavedJson({});
  }

  m_pendingSortRole = hasPinnedSort ? sortRoleForName(QString::fromStdString(folder->sortRole)) : DisplayName;
  m_pendingSortAscending = !hasPinnedSort || folder->sortAscending;

  m_adoptingFolder = false;

  applyFilters();

  emit openFolderChanged();
  emit sortPinnedChanged();
  emit sortRoleChanged();
  emit sortAscendingChanged();
  emit filtersOrSortChanged();
}

void LibraryEntrySortFilterModel::clearAllFilters() {
  m_filter->clear();
  applyFilters();
}

void LibraryEntrySortFilterModel::applyFilters() { applyFilters(QDateTime::currentMSecsSinceEpoch()); }

void LibraryEntrySortFilterModel::applyFilters(const qint64 nowMillis) {
  m_appliedCriteria = m_filter->getCriteria();
  m_appliedScopeFolderId = m_scopeFolderId;
  m_nowMillis = nowMillis;

  m_appliedScopeMembers.clear();

  if (m_appliedScopeFolderId != -1 && m_sourceModel != nullptr) {
    for (auto row = 0; row < m_sourceModel->rowCount({}); ++row) {
      const auto sourceIndex = m_sourceModel->index(row, 0);
      const auto *fields =
          m_sourceModel->data(sourceIndex, library::EntryListModel::FilterFields).value<const library::EntryFields *>();

      if (fields == nullptr) {
        continue;
      }

      if (std::ranges::find(fields->folderIds, m_appliedScopeFolderId) != fields->folderIds.end()) {
        m_appliedScopeMembers.insert(m_sourceModel->data(sourceIndex, library::EntryListModel::Id).toInt());
      }
    }
  }

  QSortFilterProxyModel::setSortRole(m_pendingSortRole);
  sort(0, m_pendingSortAscending ? Qt::AscendingOrder : Qt::DescendingOrder);

  emit sortRoleChanged();
  invalidateFilter();
}

bool LibraryEntrySortFilterModel::filterAcceptsRow(const int sourceRow, const QModelIndex &sourceParent) const {
  if (m_sourceModel == nullptr) {
    return false;
  }

  const auto sourceIndex = m_sourceModel->index(sourceRow, 0, sourceParent);

  const auto *fields =
      m_sourceModel->data(sourceIndex, library::EntryListModel::FilterFields).value<const library::EntryFields *>();

  if (fields == nullptr) {
    return false;
  }

  if (m_appliedScopeFolderId != -1 &&
      !m_appliedScopeMembers.contains(m_sourceModel->data(sourceIndex, library::EntryListModel::Id).toInt())) {
    return false;
  }

  return m_appliedCriteria.matches(*fields, m_nowMillis);
}

bool LibraryEntrySortFilterModel::isInScope(const QModelIndex &sourceIndex) const {
  if (m_appliedScopeFolderId == -1 || m_sourceModel == nullptr) {
    return true;
  }

  const auto *fields =
      m_sourceModel->data(sourceIndex, library::EntryListModel::FilterFields).value<const library::EntryFields *>();

  if (fields == nullptr) {
    return true;
  }

  return std::ranges::find(fields->folderIds, m_appliedScopeFolderId) != fields->folderIds.end();
}

QHash<int, QByteArray> LibraryEntrySortFilterModel::roleNames() const {
  auto names = QSortFilterProxyModel::roleNames();
  names[RemovedFromScope] = "removedFromScope";

  return names;
}

QVariant LibraryEntrySortFilterModel::data(const QModelIndex &index, const int role) const {
  if (role != RemovedFromScope) {
    return QSortFilterProxyModel::data(index, role);
  }

  return !isInScope(mapToSource(index));
}

QVariantList LibraryEntrySortFilterModel::getSortOptions() {
  return {
      QVariantMap{{"value", DisplayName}, {"role", "displayName"}, {"text", "Title"}},
      QVariantMap{{"value", LastPlayedAt}, {"role", "lastPlayedAt"}, {"text", "Last Played"}},
      QVariantMap{{"value", NumSecondsPlayed}, {"role", "numSecondsPlayed"}, {"text", "Playtime"}},
      QVariantMap{{"value", AchievementsEarned}, {"role", "achievementsEarned"}, {"text", "Achievements"}},
      QVariantMap{{"value", CreatedAt}, {"role", "createdAt"}, {"text", "Date Added"}},
      QVariantMap{{"value", ReleaseYear}, {"role", "releaseYear"}, {"text", "Release Year"}},
      QVariantMap{{"value", Custom}, {"role", "position"}, {"text", "Custom"}},
  };
}

int LibraryEntrySortFilterModel::getEntryIdAt(const int row) const {
  if (row < 0 || row >= rowCount({})) {
    return -1;
  }

  return data(index(row, 0), library::EntryListModel::Id).toInt();
}

QVariantMap LibraryEntrySortFilterModel::getCountByPlatform() const {
  if (m_sourceModel == nullptr) {
    return {};
  }

  return m_sourceModel->getCountByPlatform();
}

void LibraryEntrySortFilterModel::setEntryFavorite(const int entryId, const bool favorite) {
  if (m_sourceModel == nullptr) {
    return;
  }

  m_sourceModel->setEntryFavorite(entryId, favorite);
}

void LibraryEntrySortFilterModel::removeEntryFromFolder(const int entryId, const int folderId) {
  if (m_sourceModel == nullptr) {
    return;
  }

  m_sourceModel->removeEntryFromFolder(entryId, folderId);
}

} // namespace firelight::gui

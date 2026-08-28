#include "library_entry_sort_filter_model.hpp"

#include <QDateTime>
#include <spdlog/spdlog.h>

namespace firelight::gui {

LibraryEntrySortFilterModel::LibraryEntrySortFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent), m_filter(new LibraryFilter(this)) {
  // Editing a criterion stages it; the pass it belongs to is run by applyFilters
  connect(m_filter, &LibraryFilter::changed, this, [this] { emit filtersOrSortChanged(); });

  // Names sort the way a reader expects rather than the way ASCII does
  setSortCaseSensitivity(Qt::CaseInsensitive);
  // TODO
  // Qualified: the unqualified name is this class's staging setter, which records what
  // to sort by later rather than telling the proxy what to sort by now
  QSortFilterProxyModel::setSortRole(m_pendingSortRole);
  setDynamicSortFilter(true);
  sort(0, Qt::AscendingOrder);

  // count is derived, so anything that changes the row set changes it
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

QString LibraryEntrySortFilterModel::getFilterText() const { return m_filter->getNameContains(); }

void LibraryEntrySortFilterModel::setFilterText(const QString &filterText) {
  if (m_filter->getNameContains() == filterText) {
    return;
  }

  m_filter->setNameContains(filterText);
  emit filterTextChanged();
}

bool LibraryEntrySortFilterModel::isFavoritesOnly() const { return m_filter->getFavorite() == LibraryFilter::Yes; }

void LibraryEntrySortFilterModel::setFavoritesOnly(const bool favoritesOnly) {
  if (isFavoritesOnly() == favoritesOnly) {
    return;
  }

  m_filter->setFavorite(favoritesOnly ? LibraryFilter::Yes : LibraryFilter::Unset);
  emit favoritesOnlyChanged();
}

bool LibraryEntrySortFilterModel::isHideUnavailable() const { return m_filter->getPlayable() == LibraryFilter::Yes; }

void LibraryEntrySortFilterModel::setHideUnavailable(const bool hideUnavailable) {
  if (isHideUnavailable() == hideUnavailable) {
    return;
  }

  m_filter->setPlayable(hideUnavailable ? LibraryFilter::Yes : LibraryFilter::Unset);
  emit hideUnavailableChanged();
}

QVariantList LibraryEntrySortFilterModel::getPlatformIds() const { return m_filter->getPlatformIds(); }

void LibraryEntrySortFilterModel::setPlatformIds(const QVariantList &platformIds) {
  if (m_filter->getPlatformIds() == platformIds) {
    return;
  }

  m_filter->setPlatformIds(platformIds);
  emit platformIdsChanged();
}

LibraryEntrySortFilterModel::SortRole LibraryEntrySortFilterModel::getSortRole() const { return m_pendingSortRole; }

void LibraryEntrySortFilterModel::setSortRole(const SortRole sortRole) {
  if (m_pendingSortRole == sortRole) {
    return;
  }

  m_pendingSortRole = sortRole;
  emit sortRoleChanged();
  emit filtersOrSortChanged();
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
  emit sortAscendingChanged();
  emit filtersOrSortChanged();
}

bool LibraryEntrySortFilterModel::anyFiltersActive() const { return !m_filter->isEmpty(); }

LibraryFilter *LibraryEntrySortFilterModel::getFilter() const { return m_filter; }

bool LibraryEntrySortFilterModel::isPending() const {
  return m_filter->getCriteria() != m_appliedCriteria || QSortFilterProxyModel::sortRole() != m_pendingSortRole ||
         (sortOrder() == Qt::AscendingOrder) != m_pendingSortAscending;
}

void LibraryEntrySortFilterModel::clearAllFilters() {
  m_filter->clear();
  applyFilters();
}

void LibraryEntrySortFilterModel::applyFilters() { applyFilters(QDateTime::currentMSecsSinceEpoch()); }

void LibraryEntrySortFilterModel::applyFilters(const qint64 nowMillis) {
  m_appliedCriteria = m_filter->getCriteria();
  m_nowMillis = nowMillis;

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

  // TODO
  // The row's own record rather than one built here, because this runs per row on every pass and
  // again while sorting
  const auto *fields =
      m_sourceModel->data(sourceIndex, library::EntryListModel::FilterFields).value<const library::EntryFields *>();

  if (fields == nullptr) {
    return false;
  }

  return m_appliedCriteria.matches(*fields, m_nowMillis);
}

QVariantList LibraryEntrySortFilterModel::getSortOptions() {
  return {
      QVariantMap{{"value", DisplayName}, {"role", "displayName"}, {"text", "Title"}},
      QVariantMap{{"value", LastPlayedAt}, {"role", "lastPlayedAt"}, {"text", "Last Played"}},
      QVariantMap{{"value", NumSecondsPlayed}, {"role", "numSecondsPlayed"}, {"text", "Playtime"}},
      QVariantMap{{"value", AchievementsEarned}, {"role", "achievementsEarned"}, {"text", "Achievements"}},
      QVariantMap{{"value", CreatedAt}, {"role", "createdAt"}, {"text", "Date Added"}},
      QVariantMap{{"value", ReleaseYear}, {"role", "releaseYear"}, {"text", "Release Year"}},
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

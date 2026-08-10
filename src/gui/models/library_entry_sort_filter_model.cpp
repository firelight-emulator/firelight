#include "library_entry_sort_filter_model.hpp"

#include <spdlog/spdlog.h>

namespace firelight::gui {

LibraryEntrySortFilterModel::LibraryEntrySortFilterModel(QObject *parent) : QSortFilterProxyModel(parent) {
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

QString LibraryEntrySortFilterModel::getFilterText() const { return m_pendingFilterText; }

void LibraryEntrySortFilterModel::setFilterText(const QString &filterText) {
  if (m_pendingFilterText == filterText) {
    return;
  }

  m_pendingFilterText = filterText;
  emit filterTextChanged();
  emit filtersOrSortChanged();
}

bool LibraryEntrySortFilterModel::isFavoritesOnly() const { return m_pendingFavoritesOnly; }

void LibraryEntrySortFilterModel::setFavoritesOnly(const bool favoritesOnly) {
  if (m_pendingFavoritesOnly == favoritesOnly) {
    return;
  }

  m_pendingFavoritesOnly = favoritesOnly;
  emit favoritesOnlyChanged();
  emit filtersOrSortChanged();
}

QVariantList LibraryEntrySortFilterModel::getPlatformIds() const { return m_pendingPlatformIds; }

void LibraryEntrySortFilterModel::setPlatformIds(const QVariantList &platformIds) {
  if (m_pendingPlatformIds == platformIds) {
    return;
  }

  m_pendingPlatformIds = platformIds;
  emit platformIdsChanged();
  emit filtersOrSortChanged();
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

bool LibraryEntrySortFilterModel::isSortAscending() const { return m_pendingSortAscending; }

void LibraryEntrySortFilterModel::setSortAscending(const bool sortAscending) {
  if (m_pendingSortAscending == sortAscending) {
    return;
  }

  m_pendingSortAscending = sortAscending;
  emit sortAscendingChanged();
  emit filtersOrSortChanged();
}

bool LibraryEntrySortFilterModel::anyFiltersActive() const {
  return !m_pendingFilterText.isEmpty() || m_pendingFavoritesOnly || !m_pendingPlatformIds.isEmpty();
}

void LibraryEntrySortFilterModel::applyFilters() {
  // TODO
  // Lowered once here to match the search text, which is stored that way so the compare
  // is not case-folding both sides on every row
  m_filterText = m_pendingFilterText.toLower();
  m_favoritesOnly = m_pendingFavoritesOnly;

  // TODO
  // Converted once here rather than per row in filterAcceptsRow
  m_acceptedPlatformIds.clear();

  for (const auto &platformId : m_pendingPlatformIds) {
    m_acceptedPlatformIds.insert(platformId.toInt());
  }

  QSortFilterProxyModel::setSortRole(m_pendingSortRole);
  sort(0, m_pendingSortAscending ? Qt::AscendingOrder : Qt::DescendingOrder);
  invalidateFilter();
}

bool LibraryEntrySortFilterModel::filterAcceptsRow(const int sourceRow, const QModelIndex &sourceParent) const {
  if (m_sourceModel == nullptr) {
    return false;
  }

  const auto sourceIndex = m_sourceModel->index(sourceRow, 0, sourceParent);

  if (m_favoritesOnly && !m_sourceModel->data(sourceIndex, library::EntryListModel::Favorite).toBool()) {
    return false;
  }

  if (!m_acceptedPlatformIds.isEmpty()) {
    const auto platformId = m_sourceModel->data(sourceIndex, library::EntryListModel::PlatformId).toInt();

    if (!m_acceptedPlatformIds.contains(platformId)) {
      return false;
    }
  }

  if (!m_filterText.isEmpty()) {
    // TODO
    // Precomputed and already lowercased, and it carries the group's other names, so
    // searching a variant by its own title finds the row standing for it
    const auto searchText = m_sourceModel->data(sourceIndex, library::EntryListModel::SearchText).toString();

    if (!searchText.contains(m_filterText)) {
      return false;
    }
  }

  // TODO
  // A group shows as the one entry standing for it. Last, so a filtered-out group does
  // not cost a grouping lookup
  if (m_collapseVariants) {
    const auto groupId = m_sourceModel->data(sourceIndex, library::EntryListModel::VariantGroupId).toInt();

    if (groupId != -1 && !m_sourceModel->data(sourceIndex, library::EntryListModel::IsVariantPrimary).toBool()) {
      return false;
    }
  }

  return true;
}

bool LibraryEntrySortFilterModel::isCollapseVariants() const { return m_collapseVariants; }

void LibraryEntrySortFilterModel::setCollapseVariants(const bool collapseVariants) {
  if (m_collapseVariants == collapseVariants) {
    return;
  }

  m_collapseVariants = collapseVariants;
  invalidateFilter();
  emit collapseVariantsChanged();
  emit countByPlatformChanged();
}

QVariantList LibraryEntrySortFilterModel::getVariantEntryIds(const int groupId) const {
  if (m_sourceModel == nullptr || groupId == -1) {
    return {};
  }

  QVariantList primary;
  QVariantList others;

  for (auto row = 0; row < m_sourceModel->rowCount(QModelIndex()); ++row) {
    const auto index = m_sourceModel->index(row, 0);

    if (m_sourceModel->data(index, library::EntryListModel::VariantGroupId).toInt() != groupId) {
      continue;
    }

    const auto entryId = m_sourceModel->data(index, library::EntryListModel::Id);

    if (m_sourceModel->data(index, library::EntryListModel::IsVariantPrimary).toBool()) {
      primary.append(entryId);
    } else {
      others.append(entryId);
    }
  }

  primary.append(others);
  return primary;
}

QVariantList LibraryEntrySortFilterModel::getSortOptions() {
  auto list = QVariantList();

  list.append(QVariantMap{
      {"value", DisplayName},
      {"text", "Title"},
  });

  list.append(QVariantMap{
      {"value", LastPlayedAt},
      {"text", "Last Played"},
  });

  list.append(QVariantMap{
      {"value", NumSecondsPlayed},
      {"text", "Playtime"},
  });

  list.append(QVariantMap{
      {"value", AchievementsEarned},
      {"text", "Achievements"},
  });

  list.append(QVariantMap{
      {"value", CreatedAt},
      {"text", "Date Added"},
  });

  return list;
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

  if (!m_collapseVariants) {
    return m_sourceModel->getCountByPlatform();
  }

  // TODO
  // Counted over the rows a collapsed grid actually shows, so the platform totals and
  // the grid do not disagree by however many variants are folded away. The platform
  // filter itself is left out, so its own chip still shows what selecting it would give
  QVariantMap counts;

  for (auto row = 0; row < m_sourceModel->rowCount(QModelIndex()); ++row) {
    const auto index = m_sourceModel->index(row, 0);
    const auto groupId = m_sourceModel->data(index, library::EntryListModel::VariantGroupId).toInt();

    if (groupId != -1 && !m_sourceModel->data(index, library::EntryListModel::IsVariantPrimary).toBool()) {
      continue;
    }

    const auto platformId = m_sourceModel->data(index, library::EntryListModel::PlatformId).toString();
    counts[platformId] = counts.value(platformId, 0).toInt() + 1;
  }

  return counts;
}

bool LibraryEntrySortFilterModel::matchesSmartFolder(const int folderId, const int entryId) {
  if (m_sourceModel == nullptr) {
    return false;
  }

  return m_sourceModel->matchesSmartFolder(folderId, entryId);
}

void LibraryEntrySortFilterModel::removeEntryFromFolder(const int entryId, const int folderId) {
  if (m_sourceModel == nullptr) {
    return;
  }

  m_sourceModel->removeEntryFromFolder(entryId, folderId);
}

void LibraryEntrySortFilterModel::setEntryFavorite(const int entryId, const bool favorite) {
  if (m_sourceModel == nullptr) {
    return;
  }

  m_sourceModel->setEntryFavorite(entryId, favorite);
}

} // namespace firelight::gui

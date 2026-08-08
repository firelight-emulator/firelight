#include "library_entry_sort_filter_model.hpp"

#include <algorithm>
#include <spdlog/spdlog.h>

namespace firelight::gui {

namespace {
/**
 * One accepted row with its sort key already read, so a comparison touches no roles
 */
struct SortableRow {
  int sourceRow;
  QString text;
  qint64 number;
};
} // namespace

LibraryEntrySortFilterModel::LibraryEntrySortFilterModel(QObject *parent) : QAbstractListModel(parent) {
  m_applyTimer.setSingleShot(true);
  m_applyTimer.setInterval(0);
  connect(&m_applyTimer, &QTimer::timeout, this, &LibraryEntrySortFilterModel::applyFilters);
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

  if (m_sourceModel != nullptr) {
    m_sourceModel->disconnect(this);
  }

  m_sourceModel = entryModel;

  if (m_sourceModel != nullptr) {
    connect(m_sourceModel, &QAbstractListModel::dataChanged, this, &LibraryEntrySortFilterModel::scheduleApply);
    connect(m_sourceModel, &QAbstractListModel::rowsInserted, this, &LibraryEntrySortFilterModel::scheduleApply);
    connect(m_sourceModel, &QAbstractListModel::rowsRemoved, this, &LibraryEntrySortFilterModel::scheduleApply);
    connect(m_sourceModel, &QAbstractListModel::modelReset, this, &LibraryEntrySortFilterModel::scheduleApply);
    connect(m_sourceModel, &library::EntryListModel::countChanged, this,
            &LibraryEntrySortFilterModel::countByPlatformChanged);
  }

  emit sourceModelChanged();
  emit filtersOrSortChanged();
  applyFilters();
}

int LibraryEntrySortFilterModel::getCount() const { return static_cast<int>(m_sourceRows.size()); }

QString LibraryEntrySortFilterModel::getFilterText() const { return m_filterText; }

void LibraryEntrySortFilterModel::setFilterText(const QString &filterText) {
  if (m_filterText == filterText) {
    return;
  }

  m_filterText = filterText;
  emit filterTextChanged();
  emit filtersOrSortChanged();
  // scheduleApply();
}

bool LibraryEntrySortFilterModel::isFavoritesOnly() const { return m_favoritesOnly; }

void LibraryEntrySortFilterModel::setFavoritesOnly(const bool favoritesOnly) {
  if (m_favoritesOnly == favoritesOnly) {
    return;
  }

  m_favoritesOnly = favoritesOnly;
  emit favoritesOnlyChanged();
  emit filtersOrSortChanged();
  // scheduleApply();
}

QVariantList LibraryEntrySortFilterModel::getPlatformIds() const { return m_platformIds; }

void LibraryEntrySortFilterModel::setPlatformIds(const QVariantList &platformIds) {
  if (m_platformIds == platformIds) {
    return;
  }

  m_platformIds = platformIds;

  // TODO
  // Converted once here rather than per row in accepts()
  m_acceptedPlatformIds.clear();

  for (const auto &platformId : platformIds) {
    m_acceptedPlatformIds.insert(platformId.toInt());
  }

  emit platformIdsChanged();
  emit filtersOrSortChanged();
  // scheduleApply();
}

LibraryEntrySortFilterModel::SortRole LibraryEntrySortFilterModel::getSortRole() const { return m_sortRole; }

void LibraryEntrySortFilterModel::setSortRole(const SortRole sortRole) {
  if (m_sortRole == sortRole) {
    return;
  }

  m_sortRole = sortRole;
  emit sortRoleChanged();
  emit filtersOrSortChanged();
  // scheduleApply();
}

bool LibraryEntrySortFilterModel::isSortAscending() const { return m_sortAscending; }

void LibraryEntrySortFilterModel::setSortAscending(const bool sortAscending) {
  if (m_sortAscending == sortAscending) {
    return;
  }

  m_sortAscending = sortAscending;
  emit sortAscendingChanged();
  emit filtersOrSortChanged();
  // scheduleApply();
}

bool LibraryEntrySortFilterModel::anyFiltersActive() const {
  return !m_filterText.isEmpty() || m_favoritesOnly || !m_acceptedPlatformIds.isEmpty();
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

bool LibraryEntrySortFilterModel::accepts(const QModelIndex &sourceIndex) const {
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
    const auto displayName = m_sourceModel->data(sourceIndex, library::EntryListModel::DisplayName).toString();

    if (!displayName.contains(m_filterText, Qt::CaseInsensitive)) {
      return false;
    }
  }

  return true;
}

void LibraryEntrySortFilterModel::scheduleApply() { m_applyTimer.start(); }

void LibraryEntrySortFilterModel::applyFilters() {
  m_applyTimer.stop();

  beginResetModel();
  m_sourceRows.clear();

  if (m_sourceModel == nullptr) {
    endResetModel();
    emit countChanged();
    return;
  }

  const auto sortRole = m_sortRole;
  const auto isTextSort = sortRole == library::EntryListModel::DisplayName;
  const auto sourceRowCount = m_sourceModel->rowCount({});

  std::vector<SortableRow> rows;
  rows.reserve(sourceRowCount);

  for (auto row = 0; row < sourceRowCount; ++row) {
    const auto sourceIndex = m_sourceModel->index(row, 0);

    if (!accepts(sourceIndex)) {
      continue;
    }

    // TODO
    // Read once per row here; a comparison then compares two members
    if (isTextSort) {
      rows.push_back({row, m_sourceModel->data(sourceIndex, sortRole).toString().toCaseFolded(), 0});
    } else {
      rows.push_back({row, {}, m_sourceModel->data(sourceIndex, sortRole).toLongLong()});
    }
  }

  // TODO
  // Stable so rows sharing a key keep the same order between passes, rather than
  // shuffling when an unrelated filter changes
  std::stable_sort(rows.begin(), rows.end(), [isTextSort, this](const SortableRow &left, const SortableRow &right) {
    if (isTextSort) {
      return m_sortAscending ? left.text < right.text : right.text < left.text;
    }

    return m_sortAscending ? left.number < right.number : right.number < left.number;
  });

  m_sourceRows.reserve(rows.size());

  for (const auto &row : rows) {
    m_sourceRows.push_back(row.sourceRow);
  }

  endResetModel();
  emit countChanged();
}

QModelIndex LibraryEntrySortFilterModel::sourceIndexFor(const int row) const {
  if (m_sourceModel == nullptr || row < 0 || row >= static_cast<int>(m_sourceRows.size())) {
    return {};
  }

  const auto sourceRow = m_sourceRows.at(row);

  if (sourceRow >= m_sourceModel->rowCount({})) {
    return {};
  }

  return m_sourceModel->index(sourceRow, 0);
}

int LibraryEntrySortFilterModel::getEntryIdAt(const int row) const {
  const auto sourceIndex = sourceIndexFor(row);

  if (!sourceIndex.isValid()) {
    return -1;
  }

  return m_sourceModel->data(sourceIndex, library::EntryListModel::Id).toInt();
}

QVariantMap LibraryEntrySortFilterModel::getCountByPlatform() const {
  if (m_sourceModel == nullptr) {
    return {};
  }

  return m_sourceModel->getCountByPlatform();
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

int LibraryEntrySortFilterModel::rowCount(const QModelIndex &parent) const {
  if (parent.isValid()) {
    return 0;
  }

  return static_cast<int>(m_sourceRows.size());
}

QVariant LibraryEntrySortFilterModel::data(const QModelIndex &index, const int role) const {
  const auto sourceIndex = sourceIndexFor(index.row());

  if (!index.isValid() || !sourceIndex.isValid()) {
    return {};
  }

  return m_sourceModel->data(sourceIndex, role);
}

bool LibraryEntrySortFilterModel::setData(const QModelIndex &index, const QVariant &value, const int role) {
  const auto sourceIndex = sourceIndexFor(index.row());

  if (!index.isValid() || !sourceIndex.isValid()) {
    return false;
  }

  return m_sourceModel->setData(sourceIndex, value, role);
}

QHash<int, QByteArray> LibraryEntrySortFilterModel::roleNames() const {
  if (m_sourceModel == nullptr) {
    return {};
  }

  return m_sourceModel->roleNames();
}

Qt::ItemFlags LibraryEntrySortFilterModel::flags(const QModelIndex &index) const {
  const auto sourceIndex = sourceIndexFor(index.row());

  if (!index.isValid() || !sourceIndex.isValid()) {
    return Qt::NoItemFlags;
  }

  return m_sourceModel->flags(sourceIndex);
}

} // namespace firelight::gui

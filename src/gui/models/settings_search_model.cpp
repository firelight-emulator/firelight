#include "settings_search_model.hpp"

#include <firelight/settings/settings_catalog.hpp>

namespace firelight::settings {

SettingsSearchModel::SettingsSearchModel(QObject *parent)
    : QAbstractListModel(parent), m_index(SettingsCatalog::instance()) {}

QString SettingsSearchModel::query() const { return m_query; }

void SettingsSearchModel::setQuery(const QString &query) {
  if (m_query == query) {
    return;
  }
  m_query = query;
  refresh();
  emit queryChanged();
}

int SettingsSearchModel::count() const {
  return static_cast<int>(m_results.size());
}

void SettingsSearchModel::refresh() {
  beginResetModel();
  m_results = m_index.search(m_query.toStdString());
  endResetModel();
  emit countChanged();
}

QString SettingsSearchModel::topRoute() const {
  return m_results.empty() ? QString()
                           : QString::fromStdString(m_results.front().route);
}

QString SettingsSearchModel::topKey() const {
  return m_results.empty() ? QString()
                           : QString::fromStdString(m_results.front().key);
}

int SettingsSearchModel::rowCount(const QModelIndex &parent) const {
  return parent.isValid() ? 0 : static_cast<int>(m_results.size());
}

QVariant SettingsSearchModel::data(const QModelIndex &index,
                                   const int role) const {
  if (!index.isValid() || index.row() < 0 ||
      index.row() >= static_cast<int>(m_results.size())) {
    return {};
  }
  const auto &result = m_results[index.row()];
  switch (role) {
  case KeyRole:
    return QString::fromStdString(result.key);
  case LabelRole:
    return QString::fromStdString(result.label);
  case DescriptionRole:
    return QString::fromStdString(result.description);
  case PageLabelRole:
    return QString::fromStdString(result.pageLabel);
  case GroupLabelRole:
    return QString::fromStdString(result.groupLabel);
  case RouteRole:
    return QString::fromStdString(result.route);
  case IsPageRole:
    return result.kind == SearchResultKind::Page;
  case AdvancedRole:
    return result.advanced;
  default:
    return {};
  }
}

QHash<int, QByteArray> SettingsSearchModel::roleNames() const {
  return {{KeyRole, "key"},
          {LabelRole, "label"},
          {DescriptionRole, "description"},
          {PageLabelRole, "pageLabel"},
          {GroupLabelRole, "groupLabel"},
          {RouteRole, "route"},
          {IsPageRole, "isPage"},
          {AdvancedRole, "advanced"}};
}

} // namespace firelight::settings

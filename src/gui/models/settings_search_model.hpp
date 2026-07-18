#pragma once

#include <QAbstractListModel>

#include <firelight/settings/settings_index.hpp>

namespace firelight::settings {

// TODO
// Search over every declared page and setting, for the settings nav's search
// field. Set `query`; the rows are the ranked hits. A hit carries the `route`
// to navigate to and, for a setting, the `key` the page should reveal.
class SettingsSearchModel : public QAbstractListModel {
  Q_OBJECT
  Q_PROPERTY(QString query READ query WRITE setQuery NOTIFY queryChanged)
  Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
  enum Roles {
    KeyRole = Qt::UserRole + 1,
    LabelRole,
    DescriptionRole,
    PageLabelRole,
    GroupLabelRole,
    RouteRole,
    IsPageRole,
    AdvancedRole
  };

  explicit SettingsSearchModel(QObject *parent = nullptr);

  [[nodiscard]] QString query() const;
  void setQuery(const QString &query);

  [[nodiscard]] int count() const;

  // The route of the best hit, or "" when there are none — what Enter in the
  // search field follows
  [[nodiscard]] Q_INVOKABLE QString topRoute() const;
  // The setting key of the best hit, "" if it's a page hit or there are none
  [[nodiscard]] Q_INVOKABLE QString topKey() const;

  [[nodiscard]] int rowCount(const QModelIndex &parent) const override;
  [[nodiscard]] QVariant data(const QModelIndex &index,
                              int role) const override;
  [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

signals:
  void queryChanged();
  void countChanged();

private:
  void refresh();

  SettingsIndex m_index;
  QString m_query;
  std::vector<SettingSearchResult> m_results;
};

} // namespace firelight::settings

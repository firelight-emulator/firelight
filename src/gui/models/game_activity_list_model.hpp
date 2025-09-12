#pragma once
#include <QAbstractListModel>
#include <manager_accessor.hpp>

namespace firelight::activity {

class GameActivityListModel : public QAbstractListModel,
                              public ManagerAccessor {
  Q_OBJECT

public:
  explicit GameActivityListModel(QObject *parent = nullptr);
  int rowCount(const QModelIndex &parent) const override;
  QVariant data(const QModelIndex &index, int role) const override;
  QHash<int, QByteArray> roleNames() const override;

private:
  struct Item {
    QString displayName;
    QString contentHash;
    QString platformName;
    QString platformSlug;
    QString iconUrl1x1;
    int numSecondsPlayed = 0;
  };

  enum Roles {
    DisplayName = Qt::UserRole + 1,
    ContentHash,
    PlatformName,
    PlatformSlug,
    IconUrl1x1,
    NumSecondsPlayed
  };

  QVector<Item> m_items;
};

} // namespace firelight::activity
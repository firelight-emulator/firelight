#pragma once
#include <QAbstractListModel>

namespace firelight::settings {

class EmulatorSettingsModel : public QAbstractListModel {
  Q_OBJECT
  Q_PROPERTY(int platformId READ platformId WRITE setPlatformId NOTIFY
                 platformIdChanged)

public:
  EmulatorSettingsModel(QObject *parent = nullptr);
  int platformId() const;
  void setPlatformId(int platformId);

  int rowCount(const QModelIndex &parent) const override;
  QVariant data(const QModelIndex &index, int role) const override;
  QHash<int, QByteArray> roleNames() const override;

signals:
  void platformIdChanged();

private:
  enum Roles { Name = Qt::UserRole + 1, Description, Category, Type, Value };

  struct Item {
    QString type;
    QString name;
    QString description;
    QString category;
    QString stringValue;
    int numberValue;
    bool boolValue;
  };

  QVector<Item> m_items;

  int m_platformId = -1;
};

} // namespace firelight::settings
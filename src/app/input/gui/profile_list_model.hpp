#pragma once

#include "service_accessor.hpp"

#include <QAbstractListModel>
#include <QUrl>
#include <qqmlintegration.h>

namespace firelight::input {

// QML-facing list of controller profiles with create/clone/rename/delete and
// JSON import/export. All operations delegate to the InputService/repository
// and refresh the model
class ProfileListModel : public QAbstractListModel, public ServiceAccessor {
  Q_OBJECT

public:
  explicit ProfileListModel(QObject *parent = nullptr);

  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  QVariant data(const QModelIndex &index, int role) const override;
  QHash<int, QByteArray> roleNames() const override;

  Q_INVOKABLE void refresh();
  Q_INVOKABLE int createProfile(const QString &name);
  Q_INVOKABLE int cloneProfile(int sourceId, const QString &name);
  Q_INVOKABLE bool renameProfile(int id, const QString &name);
  Q_INVOKABLE bool deleteProfile(int id);
  // Writes a profile to a .json file (fileUrl may be a file:// URL or path)
  Q_INVOKABLE bool exportProfile(int id, const QUrl &fileUrl);
  // Reads a profile from a .json file; returns the new profile id or -1
  Q_INVOKABLE int importProfile(const QUrl &fileUrl);

private:
  enum Roles {
    ProfileId = Qt::UserRole + 1,
    Name,
    Builtin,
    BasedOnType,
    Icon,
    IsKeyboard,
  };

  struct Item {
    int id = -1;
    QString name;
    bool builtin = false;
    int basedOnType = -1;
    QString icon;
    bool isKeyboard = false;
  };

  QList<Item> m_items;
};

} // namespace firelight::input

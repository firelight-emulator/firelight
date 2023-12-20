//
// Created by alexs on 12/18/2023.
//

#ifndef FIRELIGHT_ROM_MODEL_HPP
#define FIRELIGHT_ROM_MODEL_HPP

#include <QAbstractListModel>
#include <QtQmlIntegration>
class RomModel : public QAbstractListModel {
  Q_OBJECT

public:
  enum RomRoles { FilenameRole = Qt::UserRole + 1, SizeRole };
  QHash<int, QByteArray> roleNames() const override;

  explicit RomModel(QObject *parent = nullptr);
  QVariant data(const QModelIndex &index, int role) const override;
  int rowCount(const QModelIndex &parent) const override;
};

#endif // FIRELIGHT_ROM_MODEL_HPP

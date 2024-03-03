//
// Created by alexs on 2/16/2024.
//

#ifndef CONTROLLER_LIST_MODEL_HPP
#define CONTROLLER_LIST_MODEL_HPP

#include <QAbstractListModel>

namespace firelight::Input {

class ControllerListModel final : public QAbstractListModel {
public:
  bool insertRows(int row, int count, const QModelIndex &parent) override;
  bool insertColumns(int column, int count, const QModelIndex &parent) override;
  bool removeRows(int row, int count, const QModelIndex &parent) override;
  bool removeColumns(int column, int count, const QModelIndex &parent) override;
  [[nodiscard]] int rowCount(const QModelIndex &parent) const override;
  [[nodiscard]] QVariant data(const QModelIndex &index,
                              int role) const override;

private:
  Q_OBJECT
};

} // namespace Firelight::Input

#endif // CONTROLLER_LIST_MODEL_HPP

#pragma once

#include <QAbstractListModel>

namespace firelight::gui {

class ControllerListModel : public QAbstractListModel {
  Q_OBJECT

public:
  [[nodiscard]] int rowCount(const QModelIndex &parent) const override;
  [[nodiscard]] QVariant data(const QModelIndex &index,
                              int role) const override;
};

} // namespace firelight::gui

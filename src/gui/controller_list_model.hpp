#pragma once

#include "../app/input/controller_manager.hpp"
#include <QAbstractListModel>

namespace firelight::gui {
  class ControllerListModel : public QAbstractListModel {
    Q_OBJECT

  public:
    explicit ControllerListModel(Input::ControllerManager &controllerManager);

    [[nodiscard]] int rowCount(const QModelIndex &parent) const override;

    [[nodiscard]] QVariant data(const QModelIndex &index,
                                int role) const override;

    QHash<int, QByteArray> roleNames() const override;

  private:
    struct Item {
      int playerIndex;
      QString modelName;
      QString manufacturerName;
      bool wired;
    };

    enum Roles { PlayerIndex = Qt::UserRole + 1, ModelName, ManufacturerName, Wired };

    Input::ControllerManager &m_controllerManager;
    std::vector<Item> m_items;

    void refreshControllerList();
  };
} // namespace firelight::gui

#pragma once

#include "../controller_manager.hpp"
#include "event_dispatcher.hpp"
#include "input2/input_service.hpp"

#include <QAbstractListModel>

namespace firelight::gui {
class ControllerListModel : public QAbstractListModel {
  Q_OBJECT

public:
  explicit ControllerListModel(QObject *parent = nullptr);

  [[nodiscard]] int rowCount(const QModelIndex &parent) const override;

  [[nodiscard]] QVariant data(const QModelIndex &index,
                              int role) const override;

  QHash<int, QByteArray> roleNames() const override;

  Q_INVOKABLE void changeGamepadOrder(const QVariantMap &oldToNewIndex);

private:
  ScopedConnection m_connectedHandler;
  ScopedConnection m_disconnectedHandler;
  ScopedConnection m_gamepadOrderChangedHandler;

  void onGamepadConnected(const input::GamepadConnectedEvent &event);
  void onGamepadDisconnected(const input::GamepadDisconnectedEvent &event);

  struct Item {
    int playerIndex;
    bool connected;
    QString modelName;
    QString manufacturerName;
    bool wired;
    QString imageUrl;
  };

  enum Roles {
    PlayerIndex = Qt::UserRole + 1,
    Connected,
    ModelName,
    ManufacturerName,
    Wired,
    ImageUrl
  };

  input::InputService *m_inputService;
  std::vector<Item> m_items;

  void refreshControllerList();
};
} // namespace firelight::gui

//
// Created by alexs on 1/6/2024.
//

#ifndef CONTROLLER_MANAGER_HPP
#define CONTROLLER_MANAGER_HPP
#include "../libretro/retropad_provider.hpp"
#include "controller.hpp"
#include "keyboard_controller.hpp"

#include <QAbstractListModel>
#include <QObject>
#include <SDL_events.h>

namespace Firelight::Input {

class ControllerManager final : public QAbstractListModel,
                                public libretro::IRetropadProvider {
  Q_OBJECT
public:
  void handleSDLControllerEvent(const SDL_Event &event);
  void refreshControllerList();

  [[nodiscard]] std::optional<Controller *>
  getControllerForPlayer(int t_player) const;
  std::optional<libretro::IRetroPad *>
  getRetropadForPlayer(int t_player) override;
  [[nodiscard]] int rowCount(const QModelIndex &parent) const override;
  [[nodiscard]] QVariant data(const QModelIndex &index,
                              int role) const override;
  [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

public slots:
  void swap(int firstIndex, int secondIndex);
  void updateControllerOrder(const QVariantMap &map);

signals:
  void controllerConnected();
  void controllerDisconnected();

private:
  enum Roles { PlayerIndex = Qt::UserRole + 1, ControllerName };

  int m_numControllers = 0;
  std::array<std::unique_ptr<Controller>, 32> m_controllers{};

  void openControllerWithDeviceIndex(int32_t t_deviceIndex);
};

} // namespace Firelight::Input

#endif // CONTROLLER_MANAGER_HPP

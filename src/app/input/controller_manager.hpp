#pragma once

#include "controller.hpp"
#include "firelight/libretro/retropad_provider.hpp"
#include <QAbstractListModel>
#include <QObject>
#include <SDL_events.h>

namespace firelight::Input {
  class ControllerManager final : public QObject,
                                  public libretro::IRetropadProvider {
    Q_OBJECT

  public:
    void setKeyboardRetropad(libretro::IRetroPad *keyboard);

    void handleSDLControllerEvent(const SDL_Event &event);

    void refreshControllerList();

    [[nodiscard]] std::optional<Controller *>
    getControllerForPlayer(int t_player) const;

    std::optional<libretro::IRetroPad *>
    getRetropadForPlayerIndex(int t_player) override;

    Q_INVOKABLE void updateControllerOrder(const QVector<int> &order);

    Q_INVOKABLE QAbstractListModel *getPlatformInputModel(int platformId);

  public slots:
    void updateControllerOrder(const QVariantMap &map);

  signals:
    void controllerConnected();

    void controllerDisconnected();

    void controllerOrderChanged();

  private:
    libretro::IRetroPad *m_keyboard = nullptr;
    int m_numControllers = 0;
    std::array<std::unique_ptr<Controller>, 32> m_controllers{};

    void openControllerWithDeviceIndex(int32_t t_deviceIndex);
  };
} // namespace firelight::Input

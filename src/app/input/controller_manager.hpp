#pragma once

#include "sdl_controller.hpp"
#include "firelight/libretro/retropad_provider.hpp"
#include "firelight/libretro/pointer_input_provider.hpp"
#include <QAbstractListModel>
#include <QObject>
#include <SDL_events.h>

#include "controller_repository.hpp"
#include "keyboard_input_handler.hpp"

namespace firelight::input {
  class ControllerManager final : public QObject,
                                  public libretro::IRetropadProvider {
    Q_OBJECT
    Q_PROPERTY(bool blockGamepadInput READ blockGamepadInput WRITE setBlockGamepadInput NOTIFY blockGamepadInputChanged)

  public:
    bool m_blockGamepadInput = false;

    explicit ControllerManager(IControllerRepository &controllerRepository);

    void setKeyboardRetropad(KeyboardInputHandler *keyboard);

    void handleSDLControllerEvent(const SDL_Event &event);

    void refreshControllerList();

    [[nodiscard]] std::optional<IGamepad *>
    getControllerForPlayerIndex(int t_player) const;

    std::optional<libretro::IRetroPad *>
    getRetropadForPlayerIndex(int t_player, int platformId) override;

    Q_INVOKABLE void updateControllerOrder(const QVector<int> &order);

    bool blockGamepadInput() const;

    void setBlockGamepadInput(bool blockGamepadInput);

  public slots:
    void updateControllerOrder(const QVariantMap &map);

  signals:
    void controllerConnected(int playerNumber, QString controllerName, QString iconSourceUrl);

    void controllerDisconnected(int playerNumber, QString controllerName, QString iconSourceUrl);

    void controllerOrderChanged();

    void retropadInputStateChanged(int playerNumber, int input, bool activated);

    void buttonStateChanged(int playerNumber, int button, bool pressed);

    void axisStateChanged(int playerNumber, int axis, int value);

    void blockGamepadInputChanged();

  private:
    IControllerRepository &m_controllerRepository;
    int m_numControllers = 0;
    std::array<std::unique_ptr<SdlController>, 32> m_controllers{};

    void openControllerWithDeviceIndex(int32_t t_deviceIndex);
  };
} // namespace firelight::input

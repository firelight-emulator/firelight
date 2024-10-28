#pragma once

#include "controller.hpp"
#include "firelight/libretro/retropad_provider.hpp"
#include "firelight/libretro/pointer_input_provider.hpp"
#include <QAbstractListModel>
#include <QObject>
#include <SDL_events.h>

#include "keyboard_input_handler.hpp"

namespace firelight::Input {
  class ControllerManager final : public QObject,
                                  public libretro::IRetropadProvider, public libretro::IPointerInputProvider {
    Q_OBJECT

  public:
    void setKeyboardRetropad(input::KeyboardInputHandler *keyboard);

    void handleSDLControllerEvent(const SDL_Event &event);

    void refreshControllerList();

    [[nodiscard]] std::optional<Controller *>
    getControllerForPlayerIndex(int t_player) const;

    std::optional<libretro::IRetroPad *>
    getRetropadForPlayerIndex(int t_player) override;

    Q_INVOKABLE void updateControllerOrder(const QVector<int> &order);

    Q_INVOKABLE QAbstractListModel *getPlatformInputModel(int platformId);

    void updateMouseState(double x, double y, bool pressed);

    void updateMousePressed(bool pressed);

    [[nodiscard]] std::pair<int16_t, int16_t> getPointerPosition() const override;

    [[nodiscard]] bool isPressed() const override;

  public slots:
    void updateControllerOrder(const QVariantMap &map);

  signals:
    void controllerConnected(int playerNumber, QString controllerName, QString iconSourceUrl);

    void controllerDisconnected(int playerNumber, QString controllerName, QString iconSourceUrl);

    void controllerOrderChanged();

    void buttonStateChanged(int playerNumber, int button, bool pressed);

  private:
    input::KeyboardInputHandler *m_keyboard = nullptr;
    int m_numControllers = 0;
    std::array<std::unique_ptr<Controller>, 32> m_controllers{};

    int16_t m_pointerX = 0;
    int16_t m_pointerY = 0;
    bool m_pointerPressed = false;

    void openControllerWithDeviceIndex(int32_t t_deviceIndex);
  };
} // namespace firelight::Input

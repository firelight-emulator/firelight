#pragma once
#include "firelight/libretro/retropad_provider.hpp"
#include "firelight/libretro/pointer_input_provider.hpp"

#include <QObject>

#include "gamepad.hpp"

namespace firelight::input {
    class InputManager final : public QObject, public libretro::IRetropadProvider,
                               public libretro::IPointerInputProvider {
        Q_OBJECT

    public:
        void addGamepad(const std::shared_ptr<IGamepad> &gamepad);

        void removeGamepad(const std::shared_ptr<IGamepad> &gamepad);

        std::optional<libretro::IRetroPad *> getRetropadForPlayerIndex(int playerIndex) override;

        [[nodiscard]] std::pair<int16_t, int16_t> getPointerPosition() const override;

        [[nodiscard]] bool isPressed() const override;

        void updateMouseState(double x, double y, bool pressed);

        void updateMousePressed(bool pressed);

    signals:
        void controllerConnected(int playerNumber, QString controllerName, QString iconSourceUrl);

        void controllerDisconnected(int playerNumber, QString controllerName, QString iconSourceUrl);

        void controllerOrderChanged();

        void buttonStateChanged(int playerNumber, int button, bool pressed);

    private:
        std::vector<std::shared_ptr<IGamepad> > m_orderedGamepads;

        int16_t m_pointerX = 0;
        int16_t m_pointerY = 0;
        bool m_pointerPressed = false;
    };
}

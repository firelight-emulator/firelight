#pragma once
#include <QObject>

namespace firelight::gui {
    class GamepadMapping : public QObject {
        Q_OBJECT

        Q_PROPERTY(
            int leftStickXDeadzone READ leftStickXDeadzone WRITE setLeftStickXDeadzone NOTIFY leftStickXDeadzoneChanged)
        Q_PROPERTY(
            int rightStickXDeadzone READ rightStickXDeadzone WRITE setRightStickXDeadzone NOTIFY
            rightStickXDeadzoneChanged)

    public:
        explicit GamepadMapping(QObject *parent = nullptr);

        int leftStickXDeadzone() const;

        void setLeftStickXDeadzone(int deadzone);

        int rightStickXDeadzone() const;

        void setRightStickXDeadzone(int deadzone);

    signals:
        void leftStickXDeadzoneChanged();

        void rightStickXDeadzoneChanged();

    private:
    };
} // firelight::gui

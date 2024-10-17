#pragma once
#include <QQuickItem>

#include "manager_accessor.hpp"

namespace firelight::input {
    class GamepadStatusItem : public QQuickItem, public ManagerAccessor {
        Q_OBJECT
        Q_PROPERTY(int playerNumber READ getPlayerNumber WRITE setPlayerNumber NOTIFY playerNumberChanged)
        Q_PROPERTY(QString name READ getName NOTIFY nameChanged)
        Q_PROPERTY(bool connected READ isConnected NOTIFY isConnectedChanged)
        Q_PROPERTY(bool northFaceDown READ northFaceDown NOTIFY northFaceDownChanged)

    public:
        explicit GamepadStatusItem(QQuickItem *parent = nullptr);

        ~GamepadStatusItem() override = default;

        void setPlayerNumber(int playerNumber);

        [[nodiscard]] int getPlayerNumber() const;

        [[nodiscard]] bool isConnected() const;

        [[nodiscard]] bool northFaceDown() const;

        [[nodiscard]] QString getName() const;


    signals:
        void playerNumberChanged();

        void nameChanged();

        void isConnectedChanged();

        void northFaceDownChanged();

    private:
        Input::Controller *m_controller = nullptr;
        QString m_name;
        int m_playerNumber = 0;
        bool m_isConnected = false;
        bool m_northFaceDown = false;
    };
} // input
// firelight

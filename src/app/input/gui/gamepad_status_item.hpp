#pragma once
#include <QQuickItem>

#include "../../manager_accessor.hpp"

namespace firelight::input {
    class GamepadStatusItem : public QQuickItem, public ManagerAccessor {
        Q_OBJECT
        Q_PROPERTY(int playerNumber READ getPlayerNumber WRITE setPlayerNumber NOTIFY playerNumberChanged)
        Q_PROPERTY(QString name READ getName NOTIFY nameChanged)
        Q_PROPERTY(int profileId READ getProfileId NOTIFY profileIdChanged)
        Q_PROPERTY(bool connected READ isConnected NOTIFY isConnectedChanged)
        Q_PROPERTY(bool northFaceDown READ northFaceDown NOTIFY northFaceDownChanged)
        Q_PROPERTY(QVariantMap inputLabels READ getInputLabels NOTIFY inputLabelsChanged)

    public:
        explicit GamepadStatusItem(QQuickItem *parent = nullptr);

        ~GamepadStatusItem() override = default;

        void setPlayerNumber(int playerNumber);

        [[nodiscard]] int getPlayerNumber() const;

        [[nodiscard]] bool isConnected() const;

        [[nodiscard]] bool northFaceDown() const;

        [[nodiscard]] QString getName() const;

        QVariantMap getInputLabels() const;

        int getProfileId() const;

    signals:
        void playerNumberChanged();

        void nameChanged();

        void isConnectedChanged();

        void inputLabelsChanged();

        void northFaceDownChanged();

        void profileIdChanged();

    private:
        IGamepad *m_controller = nullptr;
        QString m_name;
        int m_playerNumber = 0;
        bool m_isConnected = false;
        bool m_northFaceDown = false;
    };
} // input
// firelight

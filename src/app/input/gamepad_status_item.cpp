#include "gamepad_status_item.hpp"


namespace firelight::input {
    GamepadStatusItem::GamepadStatusItem(QQuickItem *parent) : QQuickItem(parent) {
        auto controllerManager = getControllerManager();

        connect(controllerManager, &Input::ControllerManager::controllerConnected, this, [this](int playerNumber) {
            if (playerNumber == m_playerNumber) {
                m_controller = getControllerManager()->getControllerForPlayerIndex(m_playerNumber - 1).
                        value_or(nullptr);
                if (m_controller) {
                    m_name = QString::fromStdString(m_controller->getName());
                    emit nameChanged();

                    m_isConnected = true;
                    emit isConnectedChanged();
                } else {
                    m_isConnected = false;
                    emit isConnectedChanged();
                }
            }
        });
        connect(controllerManager, &Input::ControllerManager::controllerDisconnected, this, [this](int playerNumber) {
            if (playerNumber == m_playerNumber) {
                m_controller = nullptr;
                m_name.clear();
                m_isConnected = false;
                emit isConnectedChanged();
            }
        });
        connect(controllerManager, &Input::ControllerManager::buttonStateChanged, this,
                [this](int playerNumber, int button, bool pressed) {
                    if (playerNumber == m_playerNumber) {
                        if (button == SDL_CONTROLLER_BUTTON_Y) {
                            if (pressed && !m_northFaceDown) {
                                m_northFaceDown = true;
                                emit northFaceDownChanged();
                            } else if (!pressed && m_northFaceDown) {
                                m_northFaceDown = false;
                                emit northFaceDownChanged();
                            }
                        }
                    }
                });
    }

    void GamepadStatusItem::setPlayerNumber(int playerNumber) {
        if (m_playerNumber == playerNumber) {
            return;
        }

        m_playerNumber = playerNumber;
        m_controller = getControllerManager()->getControllerForPlayerIndex(m_playerNumber).value_or(nullptr);
        if (m_controller != nullptr && !m_isConnected) {
            m_isConnected = true;
            emit isConnectedChanged();
        }

        emit playerNumberChanged();
    }

    int GamepadStatusItem::getPlayerNumber() const {
        return m_playerNumber;
    }

    bool GamepadStatusItem::isConnected() const {
        return m_isConnected;
    }

    bool GamepadStatusItem::northFaceDown() const {
        return m_northFaceDown;
    }

    QString GamepadStatusItem::getName() const {
        return m_name;
    }
} // input
// firelight

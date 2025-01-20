#include "gamepad_status_item.hpp"


namespace firelight::input {
    GamepadStatusItem::GamepadStatusItem(QQuickItem *parent) : QQuickItem(parent) {
        auto controllerManager = getControllerManager();

        connect(controllerManager, &input::ControllerManager::controllerConnected, this, [this](int playerNumber) {
            if (playerNumber == m_playerNumber) {
                m_controller = getControllerManager()->getControllerForPlayerIndex(m_playerNumber - 1).
                        value_or(nullptr);
                if (m_controller) {
                    m_name = QString::fromStdString(m_controller->getName());
                    emit nameChanged();
                    emit profileIdChanged();

                    m_isConnected = true;
                    emit isConnectedChanged();
                } else {
                    m_isConnected = false;
                    emit isConnectedChanged();
                }
            }
        });
        connect(controllerManager, &input::ControllerManager::controllerDisconnected, this, [this](int playerNumber) {
            if (playerNumber == m_playerNumber) {
                m_controller = nullptr;
                m_name.clear();
                m_isConnected = false;
                emit isConnectedChanged();
                emit profileIdChanged();
            }
        });
        connect(controllerManager, &input::ControllerManager::buttonStateChanged, this,
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
        m_controller = getControllerManager()->getControllerForPlayerIndex(m_playerNumber - 1).value_or(nullptr);
        if (m_controller != nullptr) {
          if (!m_isConnected) {
            m_isConnected = true;
            emit isConnectedChanged();
          }
        }

        m_name = QString::fromStdString(m_controller->getName());
        emit nameChanged();

        emit profileIdChanged();
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

    QVariantMap GamepadStatusItem::getInputLabels() const {
        return {
            {QString::number(libretro::IRetroPad::Input::NorthFace), "North Face"},
            {QString::number(libretro::IRetroPad::Input::SouthFace), "South Face"},
            {QString::number(libretro::IRetroPad::Input::EastFace), "East Face"},
            {QString::number(libretro::IRetroPad::Input::WestFace), "West Face"},
            {QString::number(libretro::IRetroPad::Input::Start), "Start"},
            {QString::number(libretro::IRetroPad::Input::Select), "Select"},
            {QString::number(libretro::IRetroPad::Input::LeftBumper), "L1"},
            {QString::number(libretro::IRetroPad::Input::RightBumper), "R1"},
            {QString::number(libretro::IRetroPad::Input::LeftTrigger), "L2"},
            {QString::number(libretro::IRetroPad::Input::RightTrigger), "R2"},
            {QString::number(libretro::IRetroPad::Input::L3), "L3"},
            {QString::number(libretro::IRetroPad::Input::R3), "R3"},
            {QString::number(libretro::IRetroPad::Input::DpadDown), "D-Pad Down"},
            {QString::number(libretro::IRetroPad::Input::DpadLeft), "D-Pad Left"},
            {QString::number(libretro::IRetroPad::Input::DpadRight), "D-Pad Right"},
            {QString::number(libretro::IRetroPad::Input::DpadUp), "D-Pad Up"},
            {QString::number(libretro::IRetroPad::Input::LeftStickLeft), "Left Stick Left"},
            {QString::number(libretro::IRetroPad::Input::LeftStickRight), "Left Stick Right"},
            {QString::number(libretro::IRetroPad::Input::LeftStickUp), "Left Stick Up"},
            {QString::number(libretro::IRetroPad::Input::LeftStickDown), "Left Stick Down"},
            {QString::number(libretro::IRetroPad::Input::RightStickLeft), "Right Stick Left"},
            {QString::number(libretro::IRetroPad::Input::RightStickRight), "Right Stick Right"},
            {QString::number(libretro::IRetroPad::Input::RightStickUp), "Right Stick Up"},
            {QString::number(libretro::IRetroPad::Input::RightStickDown), "Right Stick Down"},
        };
    }

    int GamepadStatusItem::getProfileId() const {
        if (m_isConnected) {
            return m_controller->getProfileId();
        }

        return 0;
        return m_controller->getProfileId();
    }
} // input
// firelight

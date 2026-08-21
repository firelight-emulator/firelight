#include "gamepad_status_item.hpp"

#include <firelight/input/input_service.hpp>

namespace firelight::input {
GamepadStatusItem::GamepadStatusItem(QQuickItem *parent) : QQuickItem(parent) {
  m_gamepadInputHandler =
      EventDispatcher::instance().subscribe<GamepadInputEvent>([this](const GamepadInputEvent &event) {
        if (event.playerIndex == m_playerNumber) {
          emit inputChanged(event.input, event.pressed);
        }
      });
}

void GamepadStatusItem::setPlayerNumber(const int playerNumber) {
  if (m_playerNumber == playerNumber) {
    return;
  }

  m_playerNumber = playerNumber;
  m_controller = getInputService()->getPlayerGamepad(playerNumber).get();

  // An empty slot is normal — asking about player 2 with one pad plugged in, or
  // about any player with none
  const bool connected = m_controller != nullptr;
  if (m_isConnected != connected) {
    m_isConnected = connected;
    emit isConnectedChanged();
  }

  m_name = connected ? QString::fromStdString(m_controller->getName()) : QString();
  emit nameChanged();

  emit profileIdChanged();
  emit playerNumberChanged();
}

int GamepadStatusItem::getPlayerNumber() const { return m_playerNumber; }

bool GamepadStatusItem::isConnected() const { return m_isConnected; }

bool GamepadStatusItem::northFaceDown() const { return m_northFaceDown; }

QString GamepadStatusItem::getName() const { return m_name; }

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
  if (!m_controller) {
    return 0;
  }
  const auto profile = m_controller->getProfile();
  return profile ? profile->getId() : 0;
}

bool GamepadStatusItem::isButtonPressed(int input) const {
  if (!m_controller) {
    return false;
  }

  return m_controller->evaluateRawInput(static_cast<GamepadInput>(input));
}
} // namespace firelight::input

// firelight

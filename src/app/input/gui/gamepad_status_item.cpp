// TODO: NEEDS REVIEW
#include "gamepad_status_item.hpp"

#include <firelight/input/input_service.hpp>

namespace firelight::input {

namespace {
constexpr double AXIS_MAX = 32767.0;
} // namespace

GamepadStatusItem::GamepadStatusItem(QQuickItem *parent) : QQuickItem(parent) {
  auto &dispatcher = EventDispatcher::instance();

  m_gamepadInputHandler = dispatcher.subscribe<GamepadInputEvent>([this](const GamepadInputEvent &event) {
    const auto playerIndex = event.playerIndex;
    const auto input = static_cast<int>(event.input);
    const auto pressed = event.pressed;
    QMetaObject::invokeMethod(
        this,
        [this, playerIndex, input, pressed] {
          if (playerIndex == m_playerIndex) {
            emit inputChanged(input, pressed);
          }
        },
        Qt::QueuedConnection);
  });

  const auto resolveOnGuiThread = [this] {
    QMetaObject::invokeMethod(this, [this] { resolveDevice(); }, Qt::QueuedConnection);
  };
  m_connectedHandler = dispatcher.subscribe<GamepadConnectedEvent>(
      [resolveOnGuiThread](const GamepadConnectedEvent &) { resolveOnGuiThread(); });
  m_disconnectedHandler = dispatcher.subscribe<GamepadDisconnectedEvent>(
      [resolveOnGuiThread](const GamepadDisconnectedEvent &) { resolveOnGuiThread(); });
  m_orderChangedHandler = dispatcher.subscribe<GamepadOrderChangedEvent>(
      [resolveOnGuiThread](const GamepadOrderChangedEvent &) { resolveOnGuiThread(); });

  m_pollTimer.setInterval(POLL_INTERVAL_MS);
  connect(&m_pollTimer, &QTimer::timeout, this, &GamepadStatusItem::poll);
}

void GamepadStatusItem::setPlayerIndex(const int playerIndex) {
  if (m_playerIndex == playerIndex) {
    return;
  }

  m_playerIndex = playerIndex;
  resolveDevice();
  emit playerIndexChanged();
}

int GamepadStatusItem::getPlayerIndex() const { return m_playerIndex; }

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

bool GamepadStatusItem::isLive() const { return m_isLive; }

void GamepadStatusItem::setLive(const bool live) {
  if (m_isLive == live) {
    return;
  }

  m_isLive = live;
  updatePolling();
  emit liveChanged();
}

double GamepadStatusItem::getLeftStickX() const { return m_values.leftStickX; }

double GamepadStatusItem::getLeftStickY() const { return m_values.leftStickY; }

double GamepadStatusItem::getRightStickX() const { return m_values.rightStickX; }

double GamepadStatusItem::getRightStickY() const { return m_values.rightStickY; }

double GamepadStatusItem::getLeftTrigger() const { return m_values.leftTrigger; }

double GamepadStatusItem::getRightTrigger() const { return m_values.rightTrigger; }

QVariantMap GamepadStatusItem::getPressed() const {
  QVariantMap pressed;
  for (std::size_t i = 0; i < BUTTONS.size(); ++i) {
    pressed[BUTTON_NAMES[i]] = m_values.pressed[i];
  }

  return pressed;
}

bool GamepadStatusItem::isButtonPressed(const int input) const {
  if (!m_controller) {
    return false;
  }

  return m_controller->readPhysical(static_cast<GamepadInput>(input)) != 0;
}

void GamepadStatusItem::resolveDevice() {
  const auto inputService = getInputService();
  m_controller = inputService && m_playerIndex >= 0 ? inputService->getPlayerGamepad(m_playerIndex) : nullptr;

  const bool connected = m_controller != nullptr;
  if (m_isConnected != connected) {
    m_isConnected = connected;
    emit isConnectedChanged();
  }

  const auto name = connected ? QString::fromStdString(m_controller->getName()) : QString();
  if (name != m_name) {
    m_name = name;
    emit nameChanged();
  }

  emit profileIdChanged();
  updatePolling();
  poll();
}

void GamepadStatusItem::poll() {
  const auto values = m_controller ? readValues() : Values{};
  if (values == m_values) {
    return;
  }

  m_values = values;
  emit valuesChanged();
}

void GamepadStatusItem::updatePolling() {
  if (m_isLive && m_controller) {
    m_pollTimer.start();
    return;
  }

  m_pollTimer.stop();
}

GamepadStatusItem::Values GamepadStatusItem::readValues() const {
  const auto axis = [this](const GamepadInput negative, const GamepadInput positive) {
    return (m_controller->readPhysical(positive) - m_controller->readPhysical(negative)) / AXIS_MAX;
  };
  const auto trigger = [this](const GamepadInput input) { return m_controller->readPhysical(input) / AXIS_MAX; };

  Values values;
  values.leftStickX = axis(LeftStickLeft, LeftStickRight);
  values.leftStickY = axis(LeftStickUp, LeftStickDown);
  values.rightStickX = axis(RightStickLeft, RightStickRight);
  values.rightStickY = axis(RightStickUp, RightStickDown);
  values.leftTrigger = trigger(LeftTrigger);
  values.rightTrigger = trigger(RightTrigger);
  for (std::size_t i = 0; i < BUTTONS.size(); ++i) {
    values.pressed[i] = m_controller->readPhysical(BUTTONS[i]) != 0;
  }

  return values;
}

} // namespace firelight::input

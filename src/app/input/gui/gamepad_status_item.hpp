// TODO: NEEDS REVIEW
#pragma once
#include "firelight/event_dispatcher.hpp"
#include "service_accessor.hpp"

#include <firelight/input/igamepad.hpp>

#include <QQuickItem>
#include <QTimer>
#include <QVariantMap>
#include <array>
#include <memory>

namespace firelight::input {

// TODO
/** The pad in one player slot, with its physical buttons and axes read while live: sticks -1 to 1 with y down, triggers
 * 0 to 1 */
class GamepadStatusItem : public QQuickItem, public ServiceAccessor {
  Q_OBJECT
  Q_PROPERTY(int playerIndex READ getPlayerIndex WRITE setPlayerIndex NOTIFY playerIndexChanged)
  Q_PROPERTY(QString name READ getName NOTIFY nameChanged)
  Q_PROPERTY(int profileId READ getProfileId NOTIFY profileIdChanged)
  Q_PROPERTY(bool connected READ isConnected NOTIFY isConnectedChanged)
  Q_PROPERTY(bool northFaceDown READ northFaceDown NOTIFY northFaceDownChanged)
  Q_PROPERTY(QVariantMap inputLabels READ getInputLabels NOTIFY inputLabelsChanged)
  Q_PROPERTY(bool live READ isLive WRITE setLive NOTIFY liveChanged)
  Q_PROPERTY(double leftStickX READ getLeftStickX NOTIFY valuesChanged)
  Q_PROPERTY(double leftStickY READ getLeftStickY NOTIFY valuesChanged)
  Q_PROPERTY(double rightStickX READ getRightStickX NOTIFY valuesChanged)
  Q_PROPERTY(double rightStickY READ getRightStickY NOTIFY valuesChanged)
  Q_PROPERTY(double leftTrigger READ getLeftTrigger NOTIFY valuesChanged)
  Q_PROPERTY(double rightTrigger READ getRightTrigger NOTIFY valuesChanged)
  Q_PROPERTY(QVariantMap pressed READ getPressed NOTIFY valuesChanged)

public:
  explicit GamepadStatusItem(QQuickItem *parent = nullptr);

  ~GamepadStatusItem() override = default;

  void setPlayerIndex(int playerIndex);

  [[nodiscard]] int getPlayerIndex() const;

  [[nodiscard]] bool isConnected() const;

  [[nodiscard]] bool northFaceDown() const;

  [[nodiscard]] QString getName() const;

  QVariantMap getInputLabels() const;

  int getProfileId() const;

  [[nodiscard]] bool isLive() const;

  void setLive(bool live);

  [[nodiscard]] double getLeftStickX() const;

  [[nodiscard]] double getLeftStickY() const;

  [[nodiscard]] double getRightStickX() const;

  [[nodiscard]] double getRightStickY() const;

  [[nodiscard]] double getLeftTrigger() const;

  [[nodiscard]] double getRightTrigger() const;

  // TODO
  /** Each button by its input name, true while the pad reports it down */
  [[nodiscard]] QVariantMap getPressed() const;

  Q_INVOKABLE bool isButtonPressed(int input) const;

signals:
  void playerIndexChanged();

  void nameChanged();

  void isConnectedChanged();

  void inputLabelsChanged();

  void northFaceDownChanged();

  void profileIdChanged();

  void inputChanged(int input, bool pressed);

  void liveChanged();

  void valuesChanged();

private:
  static constexpr int POLL_INTERVAL_MS = 16;

  static constexpr std::array<GamepadInput, 15> BUTTONS = {SouthFace,  EastFace,    WestFace,  NorthFace, DpadUp,
                                                           DpadDown,   DpadLeft,    DpadRight, Start,     Select,
                                                           LeftBumper, RightBumper, L3,        R3,        Home};

  static constexpr std::array<const char *, 15> BUTTON_NAMES = {
      "SouthFace", "EastFace", "WestFace",   "NorthFace",   "DpadUp", "DpadDown", "DpadLeft", "DpadRight",
      "Start",     "Select",   "LeftBumper", "RightBumper", "L3",     "R3",       "Home"};

  // TODO
  /** What one poll read */
  struct Values {
    double leftStickX = 0;
    double leftStickY = 0;
    double rightStickX = 0;
    double rightStickY = 0;
    double leftTrigger = 0;
    double rightTrigger = 0;
    std::array<bool, BUTTONS.size()> pressed{};

    bool operator==(const Values &) const = default;
  };

  // TODO
  /** Takes the slot's current device and reports the name, connection and profile that changed */
  void resolveDevice();

  // TODO
  /** Reads the device and reports when anything moved */
  void poll();

  // TODO
  /** Runs the timer only while live and connected */
  void updatePolling();

  [[nodiscard]] Values readValues() const;

  std::shared_ptr<IGamepad> m_controller;
  QString m_name;
  int m_playerIndex = -1;
  bool m_isConnected = false;
  bool m_northFaceDown = false;
  bool m_isLive = true;
  Values m_values;
  QTimer m_pollTimer;

  ScopedConnection m_gamepadInputHandler;
  ScopedConnection m_connectedHandler;
  ScopedConnection m_disconnectedHandler;
  ScopedConnection m_orderChangedHandler;
};
} // namespace firelight::input

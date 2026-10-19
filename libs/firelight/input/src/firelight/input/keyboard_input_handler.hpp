#pragma once

#include <firelight/input/igamepad.hpp>
#include <firelight/libretro/retropad.hpp>

#include <QMap>
#include <QTimer>
#include <mutex>

namespace firelight::input {
class KeyboardInputHandler final : public QObject, public IGamepad {
  Q_OBJECT

public:
  KeyboardInputHandler();

  static const QMap<GamepadInput, Qt::Key> &defaultKeyMap();
  static Qt::Key getDefaultKey(GamepadInput input);
  static QString getKeyLabel(Qt::Key key);

  [[nodiscard]] std::shared_ptr<GamepadProfile> getProfile() const override;
  void setProfile(const std::shared_ptr<GamepadProfile> &profile) override;

  [[nodiscard]] int16_t evaluateRawInput(GamepadInput input) const override;

  [[nodiscard]] int16_t readPhysical(GamepadInput input) const override;

  bool isButtonPressed(int platformId, int controllerTypeId, Input t_button) override;

  int16_t getLeftStickXPosition(int platformId, int controllerTypeId) override;

  int16_t getLeftStickYPosition(int platformId, int controllerTypeId) override;

  int16_t getRightStickXPosition(int platformId, int controllerTypeId) override;

  int16_t getRightStickYPosition(int platformId, int controllerTypeId) override;

  void setStrongRumble(int platformId, uint16_t t_strength) override;

  void setWeakRumble(int platformId, uint16_t t_strength) override;

  [[nodiscard]] std::string getName() const override;

  [[nodiscard]] int getPlayerIndex() const override;

  void setPlayerIndex(int playerIndex) override;

  [[nodiscard]] bool isWired() const override;

  [[nodiscard]] GamepadType getType() const override;

  [[nodiscard]] DeviceType getDeviceType() const override;

  int getInstanceId() const override;
  bool disconnect() override;
  [[nodiscard]] DeviceIdentifier getDeviceIdentifier() const override;

protected:
  bool eventFilter(QObject *obj, QEvent *event) override;

private:
  QMap<Input, bool> m_buttonStates;
  QMap<Qt::Key, bool> m_keyStates;

  std::mutex m_keyStatesMutex;

  std::shared_ptr<GamepadProfile> m_profile;

  int m_playerIndex = -1;
};
} // namespace firelight::input

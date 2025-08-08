#pragma once

#include <QEvent>
#include <QMap>
#include <QPointF>
#include <QTimer>

#include "firelight/libretro/retropad.hpp"
#include "input2/gamepad.hpp"
#include "input2/sdl/sdl_controller.hpp"
#include "profiles/keyboard_mapping.hpp"

namespace firelight::input {
class KeyboardInputHandler final : public QObject, public SdlController {
  Q_OBJECT

public:
  KeyboardInputHandler();
  std::shared_ptr<GamepadProfile> getProfile() const override;
  void setProfile(const std::shared_ptr<GamepadProfile> &profile) override;

  bool isButtonPressed(int platformId, int controllerTypeId,
                       Input t_button) override;

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

protected:
  bool eventFilter(QObject *obj, QEvent *event) override;

private:
  QMap<Input, bool> m_buttonStates;
  QMap<Qt::Key, bool> m_keyStates;

  std::shared_ptr<KeyboardMapping> m_keyboardMapping;

  int m_playerIndex;
};
} // namespace firelight::input

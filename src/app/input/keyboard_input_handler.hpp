#pragma once

#include <QEvent>
#include <QMap>
#include <QTimer>
#include <QPointF>

#include "gamepad.hpp"
#include "firelight/libretro/retropad.hpp"

namespace firelight::input {
  class KeyboardInputHandler final : public QObject, public IGamepad {
    Q_OBJECT

  public:
    bool isButtonPressed(int platformId, Input t_button) override;

    int16_t getLeftStickXPosition(int platformId) override;

    int16_t getLeftStickYPosition(int platformId) override;

    int16_t getRightStickXPosition(int platformId) override;

    int16_t getRightStickYPosition(int platformId) override;

    void setStrongRumble(int platformId, uint16_t t_strength) override;

    void setWeakRumble(int platformId, uint16_t t_strength) override;

    std::string getName() const override;

    int getPlayerIndex() const override;

    void setPlayerIndex(int playerIndex) override;

    bool isWired() const override;

    GamepadType getType() const override;

  protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

  private:
    QMap<Input, bool> m_buttonStates;
  };
} // namespace firelight::gui

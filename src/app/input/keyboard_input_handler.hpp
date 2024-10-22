#pragma once

#include <QEvent>
#include <QMap>
#include <QTimer>
#include <QPointF>
#include "firelight/libretro/retropad.hpp"

namespace firelight::input {
  class KeyboardInputHandler final : public QObject, public libretro::IRetroPad {
    Q_OBJECT

  public:
    bool isButtonPressed(Button t_button) override;

    int16_t getLeftStickXPosition() override;

    int16_t getLeftStickYPosition() override;

    int16_t getRightStickXPosition() override;

    int16_t getRightStickYPosition() override;

    void setStrongRumble(uint16_t t_strength) override;

    void setWeakRumble(uint16_t t_strength) override;

  protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

  private:
    QMap<Button, bool> m_buttonStates;
  };
} // namespace firelight::gui

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
    bool isButtonPressed(int platformId, Input t_button) override;

    int16_t getLeftStickXPosition(int platformId) override;

    int16_t getLeftStickYPosition(int platformId) override;

    int16_t getRightStickXPosition(int platformId) override;

    int16_t getRightStickYPosition(int platformId) override;

    void setStrongRumble(int platformId, uint16_t t_strength) override;

    void setWeakRumble(int platformId, uint16_t t_strength) override;

  protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

  private:
    QMap<Input, bool> m_buttonStates;
  };
} // namespace firelight::gui

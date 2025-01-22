#pragma once

#include <QEvent>
#include <QMap>
#include <QTimer>
#include <QPointF>

#include "firelight/libretro/retropad.hpp"
#include "gamepad.hpp"
#include "profiles/keyboard_mapping.hpp"
#include "sdl_controller.hpp"

namespace firelight::input {
  class KeyboardInputHandler final : public QObject, public input::SdlController {
  Q_OBJECT

public:
    KeyboardInputHandler();

    bool isButtonPressed(int platformId, Input t_button) override;

    int16_t getLeftStickXPosition(int platformId) override;

    int16_t getLeftStickYPosition(int platformId) override;

    int16_t getRightStickXPosition(int platformId) override;

    int16_t getRightStickYPosition(int platformId) override;

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

    int m_playerIndex;
  };
} // namespace firelight::gui

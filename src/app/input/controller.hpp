#pragma once

#include <map>
#include <memory>
#include <QObject>
#include <QQuickItem>

#include "firelight/libretro/retropad.hpp"
#include <SDL_gamecontroller.h>
#include <string>

#include "profiles/controller_profile.hpp"
#include "gamepad_type.hpp"

namespace firelight::Input {
  class Controller final : public libretro::IRetroPad {
  public:
    Controller(SDL_GameController *t_controller, int32_t t_joystickIndex);

    ~Controller() override;

    void setControllerProfile(const std::shared_ptr<input::ControllerProfile> &profile);

    bool isButtonPressed(Button t_button) override;

    int16_t getLeftStickXPosition() override;

    int16_t getLeftStickYPosition() override;

    int16_t getRightStickXPosition() override;

    int16_t getRightStickYPosition() override;

    [[nodiscard]] int32_t getInstanceId() const;

    [[nodiscard]] int32_t getDeviceIndex() const;

    [[nodiscard]] std::string getControllerName() const;

    void setPlayerIndex(int t_newPlayerIndex) const;

    [[nodiscard]] int getPlayerIndex() const;

    void setStrongRumble(uint16_t t_strength) override;

    void setWeakRumble(uint16_t t_strength) override;

    [[nodiscard]] bool isWired() const;

    GamepadType getGamepadType() const;

  private:
    static std::map<int, int> controllerTypeMap;

    std::shared_ptr<input::ControllerProfile> m_profile = nullptr;

    SDL_GameController *m_SDLController = nullptr;
    SDL_Joystick *m_SDLJoystick = nullptr;
    int32_t m_SDLJoystickDeviceIndex = -1;
    int32_t m_SDLJoystickInstanceId = -1;
  };
} // namespace firelight::Input

#include "input_manager.hpp"

namespace firelight::input {
    void InputManager::addGamepad(const std::shared_ptr<IGamepad> &gamepad) {
        m_orderedGamepads.push_back(gamepad);
    }

    void InputManager::removeGamepad(const std::shared_ptr<IGamepad> &gamepad) {
        for (auto it = m_orderedGamepads.begin(); it != m_orderedGamepads.end(); ++it) {
            if (*it == gamepad) {
                m_orderedGamepads.erase(it);
                break;
            }
        }
    }

    std::optional<libretro::IRetroPad *> InputManager::getRetropadForPlayerIndex(int playerIndex, int platformId) {
        return {};
    }

    std::pair<int16_t, int16_t> InputManager::getPointerPosition() const {
        return {m_pointerX, m_pointerY};
    }

    // std::pair<int16_t, int16_t> InputManager::getPointerPosition() const {
    //     return {m_pointerX, m_pointerY};
    // }

    bool InputManager::isPressed() const {
        return m_pointerPressed;
    }

    void InputManager::updateMouseState(double x, double y, bool pressed) {
        m_pointerX = x * 32767;
        m_pointerY = y * 32767;
        m_pointerPressed = pressed;
    }

    void InputManager::updateMousePressed(bool pressed) {
        m_pointerPressed = pressed;
    }
}

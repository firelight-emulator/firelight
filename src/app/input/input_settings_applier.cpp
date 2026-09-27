#include "input_settings_applier.hpp"

#include <string>

namespace firelight::input {

namespace {
constexpr auto PRIORITIZE_CONTROLLER_OVER_KEYBOARD_KEY = "prioritize-controller-over-keyboard";
} // namespace

InputSettingsApplier::InputSettingsApplier(InputService &inputService, settings::SettingsService &settingsService)
    : m_inputService(inputService),
      m_prioritizeControllerOverKeyboard(
          settingsService, PRIORITIZE_CONTROLLER_OVER_KEYBOARD_KEY,
          [this](const std::string &value) { m_inputService.setPreferGamepadOverKeyboard(value == "true"); }) {}

} // namespace firelight::input

#pragma once

#include <firelight/input/input_service.hpp>
#include <firelight/settings/setting_watcher.hpp>
#include <firelight/settings/settings_service.hpp>

namespace firelight::input {

/**
 * Pushes the global input settings into the input service and keeps them current as they change
 */
class InputSettingsApplier {
public:
  /**
   * Applies every watched setting at once; throws if one is missing from the settings catalog
   */
  InputSettingsApplier(InputService &inputService, settings::SettingsService &settingsService);

private:
  InputService &m_inputService;

  settings::SettingWatcher m_prioritizeControllerOverKeyboard;
};

} // namespace firelight::input

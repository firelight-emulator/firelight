#pragma once

#include <firelight/event_dispatcher.hpp>
#include <firelight/input/input_service.hpp>
#include <firelight/settings/settings_service.hpp>

#include <cstdint>
#include <map>
#include <mutex>
#include <optional>
#include <string>

namespace firelight::input {

/**
 * Pushes each connected controller's rumble strength and light colour into the input service and keeps them current
 */
class ControllerSettingsApplier {
public:
  /**
   * Applies the current values at once; throws if either key is not a controller setting in the settings catalog
   */
  ControllerSettingsApplier(InputService &inputService, settings::SettingsService &settingsService);

  ControllerSettingsApplier(const ControllerSettingsApplier &) = delete;
  ControllerSettingsApplier &operator=(const ControllerSettingsApplier &) = delete;

private:
  /** The values last pushed for one profile */
  struct PushedValues {
    int rumblePercent;
    std::optional<uint32_t> lightColor;
  };

  /** Resolves every connected controller's values and pushes the ones that changed */
  void refresh();

  /** A profile's value for a key in the current game, or the default */
  std::string resolve(int profileId, const std::string &key, const std::string &defaultValue);

  InputService &m_inputService;
  settings::SettingsService &m_settingsService;
  std::string m_rumbleStrengthDefault;
  std::string m_lightBarColorDefault;

  std::mutex m_mutex;
  std::string m_contentHash;
  std::map<int, PushedValues> m_pushedByProfileId;

  ScopedConnection m_controllerSettingChangedConnection;
  ScopedConnection m_controllerSettingResetConnection;
  ScopedConnection m_gamepadConnectedConnection;
  ScopedConnection m_emulationStartedConnection;
  ScopedConnection m_emulationStoppedConnection;
};

} // namespace firelight::input

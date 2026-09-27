#pragma once

#include <firelight/event_dispatcher.hpp>
#include "settings_service.hpp"
#include <functional>

namespace firelight::settings {

class SettingWatcher {
public:
  SettingWatcher(SettingsService &settings, std::string key, std::function<void(const std::string &)> apply);
  SettingWatcher(const SettingWatcher &) = delete;
  SettingWatcher &operator=(const SettingWatcher &) = delete;
  ~SettingWatcher() = default;

private:
  void applyCurrentValue();

  SettingsService &m_settings;
  std::string m_key;
  std::function<void(const std::string &)> m_apply;

  ScopedConnection m_globalSettingChangedConnection;
  ScopedConnection m_globalSettingResetConnection;

  std::string m_defaultValue;
  std::string m_lastValue;
};

} // namespace firelight::settings

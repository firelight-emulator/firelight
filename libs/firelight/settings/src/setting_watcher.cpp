#include "firelight/settings/setting_watcher.hpp"

#include <firelight/settings/settings_catalog.hpp>
#include <spdlog/spdlog.h>
#include <stdexcept>

namespace firelight::settings {

SettingWatcher::SettingWatcher(SettingsService &settings, std::string key,
                               std::function<void(const std::string &)> apply)
    : m_settings(settings), m_key(std::move(key)), m_apply(std::move(apply)) {

  // Check catalog and get default value
  const auto &catalog = SettingsCatalog::instance();
  const auto def = catalog.findByKey(m_key);

  if (!def) {
    spdlog::warn("[SettingWatcher] Key '{}' not found in catalog", m_key);
    throw std::runtime_error("Key not found in catalog: " + m_key);
  }

  m_defaultValue = def->defaultValue;

  // Set initial value
  const auto value = m_settings.getGlobalEffectiveValue(m_key).value_or(m_defaultValue);
  m_lastValue = value;
  m_apply(value);

  m_globalSettingChangedConnection =
      EventDispatcher::instance().subscribe<GlobalSettingChangedEvent>([this](const GlobalSettingChangedEvent &event) {
        if (event.key != m_key) {
          return;
        }

        applyCurrentValue();
      });

  m_globalSettingResetConnection =
      EventDispatcher::instance().subscribe<GlobalSettingResetEvent>([this](const GlobalSettingResetEvent &event) {
        if (event.key != m_key) {
          return;
        }

        applyCurrentValue();
      });
}

void SettingWatcher::applyCurrentValue() {
  const auto &newValue = m_settings.getGlobalEffectiveValue(m_key).value_or(m_defaultValue);
  if (newValue != m_lastValue) {
    m_lastValue = newValue;
    m_apply(newValue);
  }
}

} // namespace firelight::settings
#include "core_settings_applier.hpp"

#include "emulator_instance.hpp"

#include <firelight/settings/settings_catalog.hpp>
#include <firelight/settings/settings_service.hpp>

#include <utility>

namespace firelight::emulation {

CoreSettingsApplier::CoreSettingsApplier(EmulatorInstance &instance, const EmulationContext &context,
                                         std::string contentHash, const int platformId)
    : m_instance(instance), m_context(context), m_contentHash(std::move(contentHash)), m_platformId(platformId) {
  // Settings resolve by inheritance (game -> platform -> global -> default), so
  // a change at ANY tier can alter this game's effective value
  m_platformSettingChangedConnection = EventDispatcher::instance().subscribe<settings::PlatformSettingChangedEvent>(
      [this](const settings::PlatformSettingChangedEvent &e) {
        if (e.platformId != m_platformId) {
          return;
        }
        refresh();
      });

  m_gameSettingChangedConnection = EventDispatcher::instance().subscribe<settings::GameSettingChangedEvent>(
      [this](const settings::GameSettingChangedEvent &e) {
        if (e.contentHash != m_contentHash) {
          return;
        }
        refresh();
      });

  m_globalSettingChangedConnection = EventDispatcher::instance().subscribe<settings::GlobalSettingChangedEvent>(
      [this](const settings::GlobalSettingChangedEvent &) { refresh(); });

  refresh();
}

void CoreSettingsApplier::refresh() {
  auto *service = m_context.settingsService;
  if (!service) {
    return;
  }
  const auto &catalog = settings::SettingsCatalog::instance();

  // Effective value for a common setting: the resolved override
  // (game -> platform -> global), else the catalog-declared default. The catalog
  // JSON is the single source of truth for these defaults
  const auto value = [&](const std::string &key) {
    return service->getEffectiveValue(m_contentHash, m_platformId, key).value_or(catalog.defaultForCommonKey(key));
  };
  const auto intValue = [&](const std::string &key) {
    try {
      return std::stoi(value(key));
    } catch (const std::exception &) {
      return 0; // missing/non-numeric (e.g. catalog not loaded) -> neutral
    }
  };
  const auto apply = [&](const std::string &key, auto &&setter) {
    setter();
    EventDispatcher::instance().publish(
        settings::EmulationSettingChangedEvent{.contentHash = m_contentHash, .key = key});
  };
  // Maps the friendly pointer-speed choice to a per-frame glide step (fraction
  // of the full ±32767 cursor range moved per frame at full stick deflection)
  const auto pointerSpeed = [&](const std::string &v) -> double {
    if (v == "slow") {
      return 0.015;
    }
    if (v == "fast") {
      return 0.040;
    }
    return 0.025; // medium / default
  };

  apply("rewind-enabled", [&] { m_instance.setRewindEnabled(value("rewind-enabled") == "true"); });
  apply("hotkeys-disabled", [&] { m_instance.setHotkeysDisabled(value("hotkeys-disabled") == "true"); });
  apply("picture-mode", [&] { m_instance.setPictureMode(value("picture-mode")); });
  apply("aspect-ratio", [&] { m_instance.setAspectRatioMode(value("aspect-ratio")); });
  apply("integer-scale", [&] { m_instance.setIntegerScale(intValue("integer-scale")); });
  apply("sync-method", [&] { m_instance.setSyncMethod(value("sync-method")); });
  apply("target-framerate", [&] { m_instance.setTargetFramerate(intValue("target-framerate")); });
  apply("dynamic-rate-control",
        [&] { m_instance.setDynamicRateControlEnabled(value("dynamic-rate-control") != "false"); });
  apply("instant-replay-enabled",
        [&] { m_instance.setInstantReplayEnabled(value("instant-replay-enabled") == "true"); });
  apply("analog-pointer-speed", [&] { m_instance.setAnalogPointerSpeed(pointerSpeed(value("analog-pointer-speed"))); });
  apply("mouse-controls-lightgun",
        [&] { m_instance.setMouseControlsPointerDevices(value("mouse-controls-lightgun") == "true"); });
}

} // namespace firelight::emulation

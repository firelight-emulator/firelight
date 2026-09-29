#include "controller_settings_applier.hpp"

#include "../emulation/emulation_service.hpp"

#include <firelight/settings/settings_catalog.hpp>

#include <algorithm>
#include <charconv>
#include <cstddef>
#include <set>
#include <stdexcept>
#include <system_error>

namespace firelight::input {

namespace {
constexpr auto RUMBLE_STRENGTH_KEY = "rumble-strength";
constexpr auto LIGHT_BAR_COLOR_KEY = "light-bar-color";
constexpr auto MIN_RUMBLE_PERCENT = 0;
constexpr auto MAX_RUMBLE_PERCENT = 100;
constexpr std::size_t HEX_COLOR_LENGTH = 7;

// The catalog default for a controller setting; throws if the key is not a controller setting
std::string getControllerDefault(const std::string &key) {
  const auto &catalog = settings::SettingsCatalog::instance();
  if (!catalog.isControllerSetting(key)) {
    throw std::runtime_error("Controller setting not found in catalog: " + key);
  }

  return catalog.findByKey(key)->defaultValue;
}

// Whether a key is one of the settings pushed here
bool isAppliedKey(const std::string &key) { return key == RUMBLE_STRENGTH_KEY || key == LIGHT_BAR_COLOR_KEY; }

// A stored percent clamped to 0..100, or full strength when it is not a number
int parseRumblePercent(const std::string &value) {
  try {
    return std::clamp(std::stoi(value), MIN_RUMBLE_PERCENT, MAX_RUMBLE_PERCENT);
  } catch (const std::exception &) {
    return MAX_RUMBLE_PERCENT;
  }
}

// A stored "#rrggbb" as 0xRRGGBB, or empty for the player color when it is anything else
std::optional<uint32_t> parseLightColor(const std::string &value) {
  if (value.size() != HEX_COLOR_LENGTH || value.front() != '#') {
    return std::nullopt;
  }

  uint32_t rgb = 0;
  const auto *last = value.data() + value.size();
  const auto [end, error] = std::from_chars(value.data() + 1, last, rgb, 16);
  if (error != std::errc{} || end != last) {
    return std::nullopt;
  }

  return rgb;
}
} // namespace

ControllerSettingsApplier::ControllerSettingsApplier(InputService &inputService,
                                                     settings::SettingsService &settingsService)
    : m_inputService(inputService), m_settingsService(settingsService),
      m_rumbleStrengthDefault(getControllerDefault(RUMBLE_STRENGTH_KEY)),
      m_lightBarColorDefault(getControllerDefault(LIGHT_BAR_COLOR_KEY)) {
  auto &dispatcher = EventDispatcher::instance();

  m_controllerSettingChangedConnection = dispatcher.subscribe<settings::ControllerSettingChangedEvent>(
      [this](const settings::ControllerSettingChangedEvent &event) {
        if (!isAppliedKey(event.key)) {
          return;
        }

        refresh();
      });

  m_controllerSettingResetConnection = dispatcher.subscribe<settings::ControllerSettingResetEvent>(
      [this](const settings::ControllerSettingResetEvent &event) {
        if (!isAppliedKey(event.key)) {
          return;
        }

        refresh();
      });

  m_gamepadConnectedConnection =
      dispatcher.subscribe<GamepadConnectedEvent>([this](const GamepadConnectedEvent &event) {
        if (!event.gamepad || event.gamepad->getDeviceType() == DeviceType::Keyboard) {
          return;
        }

        if (const auto profile = event.gamepad->getProfile()) {
          std::lock_guard lock(m_mutex);
          m_pushedByProfileId.erase(profile->getId());
        }

        refresh();
      });

  m_emulationStartedConnection =
      dispatcher.subscribe<emulation::EmulationStartedEvent>([this](const emulation::EmulationStartedEvent &event) {
        {
          std::lock_guard lock(m_mutex);
          m_contentHash = event.contentHash;
          m_pushedByProfileId.clear();
        }

        refresh();
      });

  m_emulationStoppedConnection =
      dispatcher.subscribe<emulation::EmulationStoppedEvent>([this](const emulation::EmulationStoppedEvent &) {
        {
          std::lock_guard lock(m_mutex);
          m_contentHash.clear();
          m_pushedByProfileId.clear();
        }

        refresh();
      });

  refresh();
}

void ControllerSettingsApplier::refresh() {
  std::lock_guard lock(m_mutex);

  std::set<int> profileIds;
  for (const auto &gamepad : m_inputService.listGamepads()) {
    if (!gamepad || gamepad->getDeviceType() == DeviceType::Keyboard) {
      continue;
    }

    if (const auto profile = gamepad->getProfile()) {
      profileIds.insert(profile->getId());
    }
  }

  for (const auto profileId : profileIds) {
    const auto rumblePercent = parseRumblePercent(resolve(profileId, RUMBLE_STRENGTH_KEY, m_rumbleStrengthDefault));
    const auto lightColor = parseLightColor(resolve(profileId, LIGHT_BAR_COLOR_KEY, m_lightBarColorDefault));

    const auto pushed = m_pushedByProfileId.find(profileId);
    const auto hasPushed = pushed != m_pushedByProfileId.end();
    if (!hasPushed || pushed->second.rumblePercent != rumblePercent) {
      m_inputService.setRumbleScale(profileId, rumblePercent);
    }

    if (!hasPushed || pushed->second.lightColor != lightColor) {
      m_inputService.setLightColor(profileId, lightColor);
    }

    m_pushedByProfileId[profileId] = PushedValues{.rumblePercent = rumblePercent, .lightColor = lightColor};
  }
}

std::string ControllerSettingsApplier::resolve(const int profileId, const std::string &key,
                                               const std::string &defaultValue) {
  return m_settingsService.getControllerEffectiveValue(m_contentHash, profileId, key).value_or(defaultValue);
}

} // namespace firelight::input

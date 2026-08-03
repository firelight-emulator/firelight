#pragma once

#include <firelight/settings/settings_service.hpp>

#include <QAudioDevice>

namespace firelight::audio {

/**
 * Resolves the output the user picked in settings, or the system default when the setting is unset
 * or names a device that is no longer present.
 *
 * Shared by every sink the app opens so game audio and UI sound always land on the same device.
 */
[[nodiscard]] QAudioDevice selectOutputDevice(settings::SettingsService &settingsService);

} // namespace firelight::audio

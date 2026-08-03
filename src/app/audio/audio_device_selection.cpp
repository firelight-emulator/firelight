#include "audio_device_selection.hpp"

#include "audio_settings.hpp"

#include <QMediaDevices>
#include <spdlog/spdlog.h>

namespace firelight::audio {

QAudioDevice selectOutputDevice(settings::SettingsService &settingsService) {
  const auto chosen = settingsService.getGlobalValue(OUTPUT_DEVICE_KEY).value_or("");

  if (!chosen.empty()) {
    const auto description = QString::fromStdString(chosen);

    for (const auto &device : QMediaDevices::audioOutputs()) {
      if (device.description() == description) {
        return device;
      }
    }

    // The chosen device is gone (unplugged, or renamed by the OS). Fall back
    // rather than play nothing
    spdlog::warn("Audio output '{}' not found; using the system default", chosen);
  }

  return QMediaDevices::defaultAudioOutput();
}

} // namespace firelight::audio

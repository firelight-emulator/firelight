#include "platform_input_preferences.hpp"

#include <firelight/input/controller_repository.hpp>
#include <firelight/input/gamepad_type.hpp>

#include <QVariantMap>

namespace firelight::input {

PlatformInputPreferences::PlatformInputPreferences(QObject *parent)
    : QObject(parent) {}

int PlatformInputPreferences::preferredType(const int platformId) const {
  const auto repo = getControllerProfileRepository();
  if (!repo) {
    return -1;
  }
  return repo->getPlatformPreferredType(platformId).value_or(-1);
}

void PlatformInputPreferences::setPreferredType(const int platformId,
                                                const int gamepadType) {
  const auto repo = getControllerProfileRepository();
  if (!repo) {
    return;
  }
  if (gamepadType < 0) {
    repo->clearPlatformPreferredType(platformId);
  } else {
    repo->setPlatformPreferredType(platformId, gamepadType);
  }
}

QVariantList PlatformInputPreferences::controllerTypeOptions() const {
  static const std::vector<std::pair<int, const char *>> options = {
      {-1, "No preference"},
      {MICROSOFT_XBOX_360, "Xbox 360"},
      {MICROSOFT_XBOX_ONE, "Xbox One"},
      {SONY_DUALSHOCK_3, "DualShock 3"},
      {SONY_DUALSHOCK_4, "DualShock 4"},
      {SONY_DUALSENSE, "DualSense"},
      {NINTENDO_SWITCH_PRO, "Switch Pro"},
      {NINTENDO_NSO_N64, "Nintendo 64"},
      {NINTENDO_NSO_SNES, "SNES (NSO)"},
      {NINTENDO_NSO_GENESIS, "Genesis (NSO)"},
  };

  QVariantList result;
  for (const auto &[value, label] : options) {
    QVariantMap entry;
    entry["value"] = value;
    entry["label"] = QString::fromUtf8(label);
    result.append(entry);
  }
  return result;
}

} // namespace firelight::input

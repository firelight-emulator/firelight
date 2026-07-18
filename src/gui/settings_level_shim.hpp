#pragma once

#include <firelight/settings/settings_service.hpp>

#include <QObject>
#include <QtQml>

namespace firelight::gui {

// Names the settings tiers for QML, so pages read `level: SettingsLevel.Global`
// rather than `level: 2`
//
// The enum itself lives in firelight_settings, which is deliberately Qt-free and
// so can't carry Q_ENUM — hence a shim on the app side rather than a change to
// the lib. The values mirror settings::SettingsLevel and are asserted below, so
// the two can't drift apart silently
namespace SettingsLevelShim {
Q_NAMESPACE

enum Level {
  Game = settings::Game,
  Platform = settings::Platform,
  Global = settings::Global,
  Unknown = settings::Unknown,
};
Q_ENUM_NS(Level)

static_assert(Game == settings::Game && Platform == settings::Platform && Global == settings::Global &&
                  Unknown == settings::Unknown,
              "SettingsLevel and its QML shim have drifted apart");

} // namespace SettingsLevelShim

} // namespace firelight::gui

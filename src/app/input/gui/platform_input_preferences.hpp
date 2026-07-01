#pragma once

#include "service_accessor.hpp"

#include <QObject>
#include <QVariantList>
#include <qqmlintegration.h>

namespace firelight::input {

// QML-facing helper for the "preferred controller per platform" setting: when a
// controller of the chosen GamepadType is connected, it is promoted to player
// one whenever a game for that platform launches.
class PlatformInputPreferences : public QObject, public ServiceAccessor {
  Q_OBJECT

public:
  explicit PlatformInputPreferences(QObject *parent = nullptr);

  // Returns the stored preferred GamepadType for a platform, or -1 if none.
  Q_INVOKABLE int preferredType(int platformId) const;
  // Sets the preferred type; a negative value clears the preference.
  Q_INVOKABLE void setPreferredType(int platformId, int gamepadType);
  // The selectable options as [{value, label}], value -1 meaning "no preference".
  Q_INVOKABLE QVariantList controllerTypeOptions() const;
};

} // namespace firelight::input

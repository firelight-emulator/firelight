#pragma once
#include <QObject>
#include <QVariantMap>

namespace firelight::gui {
// Samples representative colors from an image so the theme can tint itself to
// match a user's background image. Exposed to QML as `ImageUtils`
class ImageUtils : public QObject {
  Q_OBJECT

public:
  // TODO
  // Loads the image at `url` (plain path, file://, or qrc:/) and returns the
  // average color of its top band, bottom band, and whole, as "#rrggbb"
  // strings: { top, bottom, average, ok }. `ok` is false when the image can't
  // be loaded
  Q_INVOKABLE QVariantMap samplePalette(const QString &url) const;
};
} // namespace firelight::gui

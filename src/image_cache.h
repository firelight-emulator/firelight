#pragma once

#include <QString>

namespace firelight::media {

class ImageCache {
public:
  explicit ImageCache(const QString &path);
  std::optional<QString> get(const QString &key);

private:
  QString m_path;
};

} // namespace firelight::media
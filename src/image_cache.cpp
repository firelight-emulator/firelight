#include "image_cache.h"

namespace firelight::media {
ImageCache::ImageCache(const QString &path) : m_path(path) {}

std::optional<QString> ImageCache::get(const QString &key) {
  return std::nullopt;
}
} // namespace firelight::media
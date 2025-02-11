#include <QUrl>

#include "image_cache_url_interceptor.h"

#include <spdlog/spdlog.h>

namespace firelight {
  ImageCacheUrlInterceptor::ImageCacheUrlInterceptor(const QString &path) {}

  QUrl ImageCacheUrlInterceptor::intercept(const QUrl &path, DataType type) {
    spdlog::debug("Intercepting url: {}", path.toString().toStdString());
    return path;
  }
} // firelight
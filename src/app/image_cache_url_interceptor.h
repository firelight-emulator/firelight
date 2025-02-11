#pragma once
#include <qqmlabstracturlinterceptor.h>

namespace firelight {

class ImageCacheUrlInterceptor final : public QQmlAbstractUrlInterceptor {
public:
  explicit ImageCacheUrlInterceptor(const QString &path);
  QUrl intercept(const QUrl &path, DataType type) override;
};

} // firelight
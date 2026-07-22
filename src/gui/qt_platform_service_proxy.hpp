#pragma once
#include "service_accessor.hpp"

#include <QObject>

namespace firelight::platforms {
class IPlatformService;
}

namespace firelight::gui {
class QtPlatformServiceProxy : public QObject, public ServiceAccessor {
  Q_OBJECT

public:
  explicit QtPlatformServiceProxy(platforms::IPlatformService *platformService, QObject *parent = nullptr);
  Q_INVOKABLE QString getPlatformIconSource(int platformId) const;

private:
  platforms::IPlatformService *m_platformService;
};

} // namespace firelight::gui
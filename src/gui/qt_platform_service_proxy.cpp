//
// Created by alexs on 7/20/2026.
//

#include "qt_platform_service_proxy.hpp"

#include <firelight/platforms/platform_service.hpp>

namespace firelight::gui {
QtPlatformServiceProxy::QtPlatformServiceProxy(platforms::IPlatformService *platformService, QObject *parent)
    : QObject(parent), m_platformService(platformService) {}

QString QtPlatformServiceProxy::getPlatformIconSource(const int platformId) const {
  if (const auto platformOpt = m_platformService->getPlatform(platformId); platformOpt.has_value()) {
    return QString("qrc:/icons/%1").arg(QString::fromStdString(platformOpt->slug));
  }

  return QString();
}
} // namespace firelight::gui
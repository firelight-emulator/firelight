#include "platform_list_model.hpp"

#include <QJsonObject>
#include <firelight/libretro/retropad.hpp>

#include "../app/platform_metadata.hpp"
#include "platforms/platform_service.hpp"

namespace firelight::gui {
PlatformListModel::PlatformListModel() {
  auto platforms = platforms::PlatformService::getInstance().listPlatforms();
  for (const auto &platform : platforms) {
    m_items.push_back(platform);
  }
}

int PlatformListModel::rowCount(const QModelIndex &parent) const {
  return m_items.length();
}

QVariant PlatformListModel::data(const QModelIndex &index, int role) const {
  if (role < Qt::UserRole || index.row() >= m_items.size()) {
    return QVariant{};
  }

  auto item = m_items.at(index.row());

  switch (role) {
  case PlatformId:
    return item.id;
  case DisplayName:
    return QString::fromStdString(item.name);
  case NumControllerTypes:
    return QVariant::fromValue(item.controllerTypes.size());
  case ControllerTypeNames: {
    QStringList controllerTypeNames;
    for (const auto &controllerType : item.controllerTypes) {
      controllerTypeNames.append(QString::fromStdString(controllerType.name));
    }
    return controllerTypeNames;
  }
  case ControllerImages: {
    QStringList controllerImageUrls;
    for (const auto &controllerType : item.controllerTypes) {
      if (controllerType.imageUrl.empty()) {
        continue;
      }
      controllerImageUrls.append(
          QString::fromStdString(controllerType.imageUrl));
    }
    return controllerImageUrls;
  }
  case IconName:
    return QString::fromStdString(item.slug);
  default:
    return QVariant{};
  }
}

QHash<int, QByteArray> PlatformListModel::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[PlatformId] = "platform_id";
  roles[DisplayName] = "display_name";
  roles[IconName] = "icon_name";
  roles[NumControllerTypes] = "num_controller_types";
  roles[ControllerTypeNames] = "controller_type_names";
  roles[ControllerImages] = "controller_images";
  return roles;
}

Q_INVOKABLE QString
PlatformListModel::getPlatformIconName(int platformId) const {
  for (const auto &item : m_items) {
    if (item.id == platformId) {
      // return item.iconName;
    }
  }

  return {};
}

Q_INVOKABLE QString
PlatformListModel::getPlatformDisplayName(int platformId) const {
  for (const auto &item : m_items) {
    if (item.id == platformId) {
      // return item.displayName;
    }
  }

  return {};
}

} // namespace firelight::gui

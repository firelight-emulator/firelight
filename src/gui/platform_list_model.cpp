#include "platform_list_model.hpp"

#include <QJsonObject>
#include <QVariant>
#include <firelight/libretro/retropad.hpp>

#include <firelight/platforms/platform_service.hpp>

namespace firelight::gui {
  PlatformListModel::PlatformListModel(
      platforms::IPlatformService &platformService) {
    for (const auto &platform: platformService.listPlatforms()) {
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
        for (const auto &controllerType: item.controllerTypes) {
          controllerTypeNames.append(QString::fromStdString(controllerType.name));
        }
        return controllerTypeNames;
      }
      case ControllerTypeIds: {
        QVariantList controllerTypeIds;
        for (const auto &controllerType: item.controllerTypes) {
          controllerTypeIds.append(static_cast<int>(controllerType.id));
        }
        return controllerTypeIds;
      }
      case ControllerImages: {
        // TODO
        // Aligned 1:1 with ControllerTypeIds / ControllerTypeNames: index i is
        // the image for controllerTypes[i]. Types without an image get an empty
        // placeholder so callers can index via controllerTypeIds.indexOf(type)
        // instead of assuming contiguous 1-based ids
        QStringList controllerImageUrls;
        for (const auto &controllerType: item.controllerTypes) {
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
    roles[PlatformId] = "platformId";
    roles[DisplayName] = "displayName";
    roles[IconName] = "iconName";
    roles[NumControllerTypes] = "numControllerTypes";
    roles[ControllerTypeNames] = "controllerTypeNames";
    roles[ControllerTypeIds] = "controllerTypeIds";
    roles[ControllerImages] = "controllerImages";
    return roles;
  }

  QString PlatformListModel::getPlatformDisplayName(int platformId) const {
    for (const auto &item: m_items) {
      if (item.id == platformId) {
        return QString::fromStdString(item.name);
      }
    }

    return {};
  }
} // namespace firelight::gui

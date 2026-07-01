#include "profile_list_model.hpp"

#include <firelight/input/controller_repository.hpp>

#include <QFile>
#include <spdlog/spdlog.h>

namespace firelight::input {

ProfileListModel::ProfileListModel(QObject *parent)
    : QAbstractListModel(parent) {
  refresh();
}

int ProfileListModel::rowCount(const QModelIndex &parent) const {
  if (parent.isValid()) {
    return 0;
  }
  return static_cast<int>(m_items.size());
}

QVariant ProfileListModel::data(const QModelIndex &index, int role) const {
  if (index.row() < 0 || index.row() >= m_items.size()) {
    return {};
  }
  const auto &item = m_items.at(index.row());
  switch (role) {
  case ProfileId:
    return item.id;
  case Name:
    return item.name;
  case Builtin:
    return item.builtin;
  case BasedOnType:
    return item.basedOnType;
  case Icon:
    return item.icon;
  default:
    return {};
  }
}

QHash<int, QByteArray> ProfileListModel::roleNames() const {
  return {
      {ProfileId, "profileId"}, {Name, "name"},
      {Builtin, "builtin"},     {BasedOnType, "basedOnType"},
      {Icon, "icon"},
  };
}

void ProfileListModel::refresh() {
  const auto service = getControllerProfileRepository();
  if (!service) {
    return;
  }

  beginResetModel();
  m_items.clear();
  for (const auto &profile : service->listProfiles()) {
    if (!profile) {
      continue;
    }
    m_items.append(Item{
        profile->getId(),
        QString::fromStdString(profile->getName()),
        profile->isBuiltin(),
        profile->getBasedOnType(),
        QString::fromStdString(profile->getIcon()),
    });
  }
  endResetModel();
}

int ProfileListModel::createProfile(const QString &name) {
  const auto service = getControllerProfileRepository();
  if (!service) {
    return -1;
  }
  const auto profile = service->createProfile(name.toStdString());
  refresh();
  return profile ? profile->getId() : -1;
}

int ProfileListModel::cloneProfile(const int sourceId, const QString &name) {
  const auto service = getControllerProfileRepository();
  if (!service) {
    return -1;
  }
  const auto profile = service->cloneProfile(sourceId, name.toStdString());
  refresh();
  return profile ? profile->getId() : -1;
}

bool ProfileListModel::renameProfile(const int id, const QString &name) {
  const auto service = getControllerProfileRepository();
  if (!service) {
    return false;
  }
  const auto ok = service->renameProfile(id, name.toStdString());
  refresh();
  return ok;
}

bool ProfileListModel::deleteProfile(const int id) {
  const auto service = getControllerProfileRepository();
  if (!service) {
    return false;
  }
  const auto ok = service->deleteProfile(id);
  refresh();
  return ok;
}

bool ProfileListModel::exportProfile(const int id, const QUrl &fileUrl) {
  const auto service = getControllerProfileRepository();
  if (!service) {
    return false;
  }
  const auto json = service->exportProfile(id);
  if (json.empty()) {
    return false;
  }

  const auto path = fileUrl.isLocalFile() ? fileUrl.toLocalFile()
                                          : fileUrl.toString();
  QFile file(path);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    spdlog::error("Failed to open {} for profile export",
                  path.toStdString());
    return false;
  }
  file.write(QByteArray::fromStdString(json));
  return true;
}

int ProfileListModel::importProfile(const QUrl &fileUrl) {
  const auto service = getControllerProfileRepository();
  if (!service) {
    return -1;
  }

  const auto path = fileUrl.isLocalFile() ? fileUrl.toLocalFile()
                                          : fileUrl.toString();
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    spdlog::error("Failed to open {} for profile import",
                  path.toStdString());
    return -1;
  }
  const auto json = file.readAll().toStdString();
  const auto profile = service->importProfile(json);
  refresh();
  return profile ? profile->getId() : -1;
}

} // namespace firelight::input

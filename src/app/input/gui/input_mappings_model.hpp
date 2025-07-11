#pragma once

#include <QAbstractListModel>
#include <firelight/libretro/retropad.hpp>
#include <manager_accessor.hpp>
#include <qqmlintegration.h>

namespace firelight::input {

class InputMappingsModel : public QAbstractListModel, public ManagerAccessor {
  Q_OBJECT
  Q_PROPERTY(int profileId READ getProfileId WRITE setProfileId NOTIFY
                 profileIdChanged)
  Q_PROPERTY(int platformId READ getPlatformId WRITE setPlatformId NOTIFY
                 platformIdChanged)

public:
  explicit InputMappingsModel(QObject *parent = nullptr);

  int rowCount(const QModelIndex &parent) const override;
  QVariant data(const QModelIndex &index, int role) const override;
  QHash<int, QByteArray> roleNames() const override;

  int getProfileId() const;
  void setProfileId(int profileId);

  int getPlatformId() const;
  void setPlatformId(int platformId);

signals:
  void profileIdChanged();
  void platformIdChanged();

private:
  enum Roles {
    OriginalInput = Qt::UserRole + 1,
    OriginalInputName,
    MappedInput,
    MappedInputName,
    IsDefault,
    HasConflict,
    ConflictingInputNames
  };

  struct Item {
    bool isDefault = false;
    libretro::IRetroPad::Input originalInput;
    QString originalInputName;
    libretro::IRetroPad::Input mappedInput;
    QString mappedInputName;
    bool hasConflict = false;
    QStringList conflictingInputNames;
  };

  void refreshMappings();

  QList<Item> m_items;
  int m_profileId = -1;
  int m_platformId = -1;
};

} // namespace firelight::input

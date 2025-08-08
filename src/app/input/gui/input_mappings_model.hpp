#pragma once

#include "input2/input_service.hpp"

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
  Q_PROPERTY(int controllerTypeId READ getControllerTypeId WRITE
                 setControllerTypeId NOTIFY controllerTypeIdChanged)

public:
  explicit InputMappingsModel(QObject *parent = nullptr);

  int rowCount(const QModelIndex &parent) const override;
  QVariant data(const QModelIndex &index, int role) const override;
  QHash<int, QByteArray> roleNames() const override;

  int getProfileId() const;
  void setProfileId(int profileId);

  int getPlatformId() const;
  void setPlatformId(int platformId);

  int getControllerTypeId() const;
  void setControllerTypeId(int controllerTypeId);

  bool setData(const QModelIndex &index, const QVariant &value,
               int role) override;
  [[nodiscard]] Qt::ItemFlags flags(const QModelIndex &index) const override;

  Q_INVOKABLE void setMapping(int originalInput, int mappedInput);
  Q_INVOKABLE void resetToDefault(int originalInput);
  Q_INVOKABLE void resetAllToDefault();
  Q_INVOKABLE void clearMapping(int originalInput);

signals:
  void profileIdChanged();
  void platformIdChanged();
  void controllerTypeIdChanged();

private:
  enum Roles {
    OriginalInput = Qt::UserRole + 1,
    OriginalInputName,
    MappedInput,
    MappedInputName,
    IsDefault,
    HasConflict,
    ConflictingInputNames,
    HasMapping
  };

  struct Item {
    bool isDefault = false;
    GamepadInput originalInput;
    QString originalInputName;
    GamepadInput mappedInput;
    QString mappedInputName;
    bool hasConflict = false;
    QStringList conflictingInputNames;
  };

  void checkForConflicts();
  void refreshMappings();

  InputService *m_inputService = nullptr;

  std::shared_ptr<GamepadProfile> m_currentProfile;
  std::shared_ptr<InputMapping> m_inputMapping;

  QList<Item> m_items;
  int m_profileId = -1;
  int m_platformId = -1;
  int m_controllerTypeId = -1;
};

} // namespace firelight::input

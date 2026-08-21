#pragma once

#include "service_accessor.hpp"

#include <firelight/input/binding.hpp>
#include <firelight/input/gamepad_profile.hpp>
#include <firelight/input/input_mapping.hpp>

#include <QAbstractListModel>
#include <memory>
#include <qqmlintegration.h>

namespace firelight::input {

// QML-facing list of the Bindings attached to a single emulated input (the
// "target") within a profile's (platform, controller-type) mapping. Supports
// multiple alternate bindings per target plus per-binding turbo/toggle options
// Bind profileId/platformId/controllerTypeId/targetInput, then edit rows
class BindingListModel : public QAbstractListModel, public ServiceAccessor {
  Q_OBJECT
  Q_PROPERTY(int profileId READ profileId WRITE setProfileId NOTIFY contextChanged)
  Q_PROPERTY(int platformId READ platformId WRITE setPlatformId NOTIFY contextChanged)
  Q_PROPERTY(int controllerTypeId READ controllerTypeId WRITE setControllerTypeId NOTIFY contextChanged)
  Q_PROPERTY(int targetInput READ targetInput WRITE setTargetInput NOTIFY contextChanged)

public:
  explicit BindingListModel(QObject *parent = nullptr);

  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  QVariant data(const QModelIndex &index, int role) const override;
  QHash<int, QByteArray> roleNames() const override;

  int profileId() const;
  void setProfileId(int id);
  int platformId() const;
  void setPlatformId(int id);
  int controllerTypeId() const;
  void setControllerTypeId(int id);
  int targetInput() const;
  void setTargetInput(int input);

  // Appends a binding to the target's list from a physical source code
  Q_INVOKABLE void addBinding(int sourceCode);
  Q_INVOKABLE void removeBinding(int index);
  Q_INVOKABLE void setToggle(int index, bool on);
  Q_INVOKABLE void setTurboEnabled(int index, bool on);
  Q_INVOKABLE void setTurboRate(int index, double hz);

signals:
  void contextChanged();

private:
  enum Roles {
    SourceLabel = Qt::UserRole + 1,
    SourceCode,
    Toggle,
    TurboEnabled,
    TurboRate,
  };

  void reload();
  std::shared_ptr<InputMapping> currentMapping() const;
  void commit();

  int m_profileId = -1;
  int m_platformId = -1;
  int m_controllerTypeId = -1;
  int m_targetInput = -1;

  std::shared_ptr<GamepadProfile> m_profile;
  std::vector<Binding> m_bindings;
};

} // namespace firelight::input

#pragma once
#include "shortcuts_model.hpp"

#include <firelight/input/gamepad_profile.hpp>

#include <QAbstractListModel>
#include <QQuickItem>
#include <input/gui/input_mappings_model.hpp>

namespace firelight::gui {

class GamepadProfileItem : public QQuickItem, public ServiceAccessor {
  Q_OBJECT
  Q_PROPERTY(int profileId READ id WRITE setId NOTIFY profileIdChanged)
  Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
  Q_PROPERTY(QAbstractListModel *shortcutsModel READ getShortcutsModel NOTIFY
                 profileIdChanged)
  Q_PROPERTY(
      bool isKeyboardProfile READ isKeyboardProfile NOTIFY profileIdChanged)

public:
  int id() const;
  void setId(int id);
  QString name() const;
  void setName(const QString &name);
  QAbstractListModel *getShortcutsModel() const;
  bool isKeyboardProfile() const;

  Q_INVOKABLE QAbstractListModel *
  getInputMappings(const int platformId, const int controllerTypeId) const {
    const auto model = new input::InputMappingsModel();
    model->setProfileId(m_profile->getId());
    model->setPlatformId(platformId);
    model->setControllerTypeId(controllerTypeId);

    return model;
  }

signals:
  void profileIdChanged(int id);
  void nameChanged(const QString &name);
  void isKeyboardProfileChanged();

private:
  int m_id = -1;
  std::shared_ptr<input::GamepadProfile> m_profile;
  // One model per profile, rebuilt when the profile changes. Handing out a new
  // one per read would leave every caller mutating a copy the view isn't
  // showing, and let the JS engine own (and collect) it.
  std::unique_ptr<ShortcutsModel> m_shortcutsModel;
};

} // namespace firelight::gui
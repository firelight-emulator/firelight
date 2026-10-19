// TODO: NEEDS REVIEW
#pragma once

#include "analog_fields.hpp"
#include "service_accessor.hpp"

#include <firelight/event_dispatcher.hpp>
#include <firelight/input/analog_settings.hpp>
#include <firelight/input/gamepad_profile.hpp>

#include <QAbstractListModel>
#include <QUrl>
#include <optional>
#include <qqmlintegration.h>

namespace firelight::input {

// TODO
/** One row per profile, with its settings writable through setData and the player slots its devices hold */
class ProfileListModel : public QAbstractListModel, public ServiceAccessor {
  Q_OBJECT

public:
  explicit ProfileListModel(QObject *parent = nullptr);

  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  QVariant data(const QModelIndex &index, int role) const override;
  bool setData(const QModelIndex &index, const QVariant &value, int role) override;
  Qt::ItemFlags flags(const QModelIndex &index) const override;
  QHash<int, QByteArray> roleNames() const override;

  Q_INVOKABLE void refresh();
  Q_INVOKABLE int createProfile(const QString &name);
  Q_INVOKABLE int cloneProfile(int sourceId, const QString &name);
  Q_INVOKABLE bool renameProfile(int id, const QString &name);
  Q_INVOKABLE bool deleteProfile(int id);
  // Writes a profile to a .json file (fileUrl may be a file:// URL or path)
  Q_INVOKABLE bool exportProfile(int id, const QUrl &fileUrl);
  // Reads a profile from a .json file; returns the new profile id or -1
  Q_INVOKABLE int importProfile(const QUrl &fileUrl);

  Q_INVOKABLE QVariantList getAllProfiles();

private:
  enum Roles {
    ProfileId = Qt::UserRole + 1,
    Name,
    Icon,
    IsKeyboard,
    PlayerSlots,
    // TODO
    // One role per entry of ANALOG_FIELDS, in that order
    AnalogRoleBase = Qt::UserRole + 100,
  };

  struct Item {
    int id = -1;
    QString name;
    QString icon;
    bool isKeyboard = false;
    AnalogSettings analog;
    QList<int> playerSlots;
  };

  // TODO
  /** The field an analog role edits, or empty for any other role */
  static std::optional<AnalogField> getAnalogFieldForRole(int role);

  // TODO
  /** A row built from a stored profile */
  Item readItem(const GamepadProfile &profile) const;

  // TODO
  /** The player slots of the connected devices using a profile, in order */
  QList<int> getPlayerSlotsFor(int profileId) const;

  // TODO
  /** Recomputes every row's player slots and reports the rows that changed */
  void refreshPlayerSlots();

  // TODO
  /** Re-reads one profile's name, icon and tuning and reports what changed */
  void reloadProfile(int profileId);

  // TODO
  /** The row showing a profile, or -1 */
  int getRowOf(int profileId) const;

  QList<Item> m_items;

  ScopedConnection m_connectedHandler;
  ScopedConnection m_disconnectedHandler;
  ScopedConnection m_orderChangedHandler;
  ScopedConnection m_settingsChangedHandler;
};

} // namespace firelight::input

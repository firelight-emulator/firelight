#pragma once
#include "service_accessor.hpp"

#include <QAbstractListModel>
#include <event_dispatcher.hpp>
#include <settings/settings_service.hpp>

namespace firelight::settings {

class EmulationSettingsModel : public QAbstractListModel,
                               public ServiceAccessor {
  Q_OBJECT
  Q_PROPERTY(int platformId READ getPlatformId WRITE setPlatformId NOTIFY
                 platformIdChanged)
  Q_PROPERTY(int level READ getLevel WRITE setLevel NOTIFY levelChanged)
  Q_PROPERTY(QString contentHash READ getContentHash WRITE setContentHash NOTIFY
                 contentHashChanged)
public:
  explicit EmulationSettingsModel(QObject *parent = nullptr);

  int getPlatformId() const;
  void setPlatformId(int platformId);

  int getLevel() const;
  void setLevel(int level);

  QString getContentHash() const;
  void setContentHash(const QString &contentHash);

  int rowCount(const QModelIndex &parent) const override;
  QVariant data(const QModelIndex &index, int role) const override;
  QHash<int, QByteArray> roleNames() const override;
  Qt::ItemFlags flags(const QModelIndex &index) const override;
  bool setData(const QModelIndex &index, const QVariant &value,
               int role) override;

signals:
  void platformIdChanged();
  void levelChanged();
  void contentHashChanged();

private:
  enum Roles {
    LabelRole = Qt::UserRole + 1,
    KeyRole,
    DescriptionRole,
    SectionRole,
    TypeRole,
    ValueRole,
    OptionsRole,
    RequiresRestartRole
  };
  struct Item {
    QString label;
    QString key;
    QString section;
    QString description;
    QString type;

    // Text, filepath, combobox, etc
    QString stringValue;

    // Number, slider, etc
    int intValue;

    // Toggle
    bool boolValue = true;
    QString trueValue = "true";
    QString falseValue = "false";

    QString defaultValue;

    // Combo-box specific
    QVector<QVariantHash> options;
    bool requiresRestart = false;
  };

  void refreshValues();
  void setItemValue(int itemIndex, Item &item, const std::string &value);

  SettingsService *m_settingsService = SettingsService::instance();
  ScopedConnection m_platformSettingChangedConnection;
  ScopedConnection m_gameSettingChangedConnection;

  QString m_contentHash;
  int m_platformId = -1;
  SettingsLevel m_level = Unknown;

  QVector<Item> m_items;
};
} // namespace firelight::settings
#pragma once
#include <QAbstractListModel>
#include <settings/settings_service.hpp>

namespace firelight::settings {

class EmulationSettingsModel : public QAbstractListModel {
  Q_OBJECT
  Q_PROPERTY(int platformId READ getPlatformId WRITE setPlatformId NOTIFY
                 platformIdChanged)
  Q_PROPERTY(int level READ getLevel WRITE setLevel NOTIFY levelChanged)
public:
  explicit EmulationSettingsModel(QObject *parent = nullptr);

  int getPlatformId() const;
  void setPlatformId(int platformId);

  int getLevel() const;
  void setLevel(int level);

  int rowCount(const QModelIndex &parent) const override;
  QVariant data(const QModelIndex &index, int role) const override;
  QHash<int, QByteArray> roleNames() const override;
  Qt::ItemFlags flags(const QModelIndex &index) const override;
  bool setData(const QModelIndex &index, const QVariant &value,
               int role) override;

signals:
  void platformIdChanged();
  void levelChanged();

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

    // Combo-box specific
    QVector<QVariantHash> options;
    bool requiresRestart = false;
  };

  int m_platformId = -1;
  SettingsLevel m_level = Platform;

  QVector<Item> m_items;
};
} // namespace firelight::settings
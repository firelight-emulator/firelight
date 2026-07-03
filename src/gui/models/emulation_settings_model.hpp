#pragma once
#include "service_accessor.hpp"

#include <QAbstractListModel>
#include <firelight/event_dispatcher.hpp>
#include <firelight/settings/emulation_setting.hpp>
#include <firelight/settings/settings_service.hpp>

#include <optional>
#include <vector>

namespace firelight::settings {

// Backs the friendly emulation-settings surface at any tier (Global / Platform /
// Game). Setting definitions come from the SettingsCatalog (common settings plus
// the platform's core-specific ones); values come from SettingsService and are
// shown as the *effective* value for the tier (this tier's override, else the
// inherited value), with `overridden` marking whether this tier set it.
class EmulationSettingsModel : public QAbstractListModel,
                               public ServiceAccessor {
  Q_OBJECT
  Q_PROPERTY(int platformId READ getPlatformId WRITE setPlatformId NOTIFY
                 platformIdChanged)
  Q_PROPERTY(int level READ getLevel WRITE setLevel NOTIFY levelChanged)
  Q_PROPERTY(QString contentHash READ getContentHash WRITE setContentHash NOTIFY
                 contentHashChanged)
  // When false, settings marked `advanced` in the catalog are hidden. Bound in
  // QML to the global "Show advanced settings" preference.
  Q_PROPERTY(bool showAdvanced READ getShowAdvanced WRITE setShowAdvanced NOTIFY
                 showAdvancedChanged)
public:
  explicit EmulationSettingsModel(QObject *parent = nullptr);

  int getPlatformId() const;
  void setPlatformId(int platformId);

  int getLevel() const;
  void setLevel(int level);

  QString getContentHash() const;
  void setContentHash(const QString &contentHash);

  bool getShowAdvanced() const;
  void setShowAdvanced(bool showAdvanced);

  int rowCount(const QModelIndex &parent) const override;
  QVariant data(const QModelIndex &index, int role) const override;
  QHash<int, QByteArray> roleNames() const override;
  Qt::ItemFlags flags(const QModelIndex &index) const override;
  bool setData(const QModelIndex &index, const QVariant &value,
               int role) override;

  // Clears this tier's override for the row so it falls back to the inherited
  // value (lower tier -> catalog default).
  Q_INVOKABLE void resetValue(int row);

signals:
  void platformIdChanged();
  void levelChanged();
  void contentHashChanged();
  void showAdvancedChanged();

private:
  enum Roles {
    LabelRole = Qt::UserRole + 1,
    KeyRole,
    DescriptionRole,
    SectionRole,
    WidgetRole,
    ValueRole,
    DefaultValueRole,
    OptionsRole,
    MinimumRole,
    MaximumRole,
    StepRole,
    OverriddenRole,
    VisibleRole,
    EnabledRole,
    RequiresRestartRole,
    PlaceholderRole,    // text widget hint
    FileExtensionsRole, // file-picker filter (QStringList)
    DirectoryModeRole   // file/folder picker: pick a directory
  };
  struct Item {
    QString label;
    QString key;
    QString section;
    QString description;
    QString widget; // UI control id: toggle / dropdown / slider / spinbox / ...

    QString stringValue; // effective value (string form)
    bool boolValue = true; // effective value (for toggle widgets)
    QString trueValue = "true";
    QString falseValue = "false";

    QString defaultValue;
    QVector<QVariantHash> options; // {label, value}
    double minimumValue = 0;
    double maximumValue = 0;
    double stepValue = 1;
    bool requiresRestart = false;

    QString placeholder;          // text widget hint
    QStringList fileExtensions;   // file-picker filter
    bool directoryMode = false;   // file/folder picker: pick a directory

    bool overridden = false; // has an override at this tier
    bool visible = true;
    bool enabled = true;
    bool advanced = false; // hidden unless "Show advanced settings" is on
    std::vector<SettingCondition> visibleWhen;
    std::vector<SettingCondition> enabledWhen;
  };

  void rebuildItems();
  // Builds a library-game-picker's options from the user's library (a leading
  // "None" plus each eligible entry as {label: display name, value: content
  // hash}). Returns just "None" when no library is wired.
  QVector<QVariantHash> buildGameOptions(const EmulationSetting &setting) const;
  void refreshValues();
  void recomputeConditions();
  void setItemValue(int itemIndex, Item &item, const std::string &value);
  // First override at m_level or a lower tier (toward Global); nullopt if none.
  std::optional<std::string> resolveValue(const std::string &key);
  // Current effective value of a sibling setting (for condition evaluation).
  std::string currentValueOf(const std::string &key) const;

  SettingsService *m_settingsService = SettingsService::instance();
  ScopedConnection m_globalSettingChangedConnection;
  ScopedConnection m_platformSettingChangedConnection;
  ScopedConnection m_gameSettingChangedConnection;

  QString m_contentHash;
  int m_platformId = -1;
  SettingsLevel m_level = Unknown;
  bool m_showAdvanced = false;

  QVector<Item> m_items;
};
} // namespace firelight::settings

#pragma once
#include "service_accessor.hpp"

#include <firelight/event_dispatcher.hpp>
#include <firelight/settings/setting_definition.hpp>
#include <firelight/settings/settings_service.hpp>

#include <QAbstractListModel>
#include <QAudioDevice>
#include <QMediaDevices>
#include <optional>
#include <vector>

namespace firelight::settings {

class SettingsModel : public QAbstractListModel, public ServiceAccessor {
  Q_OBJECT
  Q_PROPERTY(int platformId READ getPlatformId WRITE setPlatformId NOTIFY platformIdChanged)
  Q_PROPERTY(int level READ getLevel WRITE setLevel NOTIFY levelChanged)
  Q_PROPERTY(QString contentHash READ getContentHash WRITE setContentHash NOTIFY contentHashChanged)
  Q_PROPERTY(QString group READ getGroup WRITE setGroup NOTIFY groupChanged)
  Q_PROPERTY(QString groupLabel READ getGroupLabel NOTIFY groupChanged)
  Q_PROPERTY(bool showAdvanced READ getShowAdvanced WRITE setShowAdvanced NOTIFY showAdvancedChanged)

public:
  explicit SettingsModel(QObject *parent = nullptr);

  [[nodiscard]] int getPlatformId() const;
  void setPlatformId(int platformId);

  [[nodiscard]] QString getGroup() const;
  void setGroup(const QString &group);

  [[nodiscard]] QString getGroupLabel() const;

  [[nodiscard]] int getLevel() const;
  void setLevel(int level);

  [[nodiscard]] QString getContentHash() const;
  void setContentHash(const QString &contentHash);

  [[nodiscard]] bool getShowAdvanced() const;
  void setShowAdvanced(bool showAdvanced);

  [[nodiscard]] int rowCount(const QModelIndex &parent) const override;
  [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
  [[nodiscard]] QHash<int, QByteArray> roleNames() const override;
  [[nodiscard]] Qt::ItemFlags flags(const QModelIndex &index) const override;
  bool setData(const QModelIndex &index, const QVariant &value, int role) override;

  // Resets the value of the row so it inherits the value from platform or global
  Q_INVOKABLE void resetValue(int row);

signals:
  void platformIdChanged();
  void levelChanged();
  void contentHashChanged();
  void groupChanged();
  void showAdvancedChanged();

private:
  enum Roles {
    LabelRole = Qt::UserRole + 1,
    KeyRole,
    DescriptionRole,
    LongDescriptionRole,
    WidgetRole,
    ValueRole,
    DefaultValueRole,
    OptionsRole,
    MinimumRole,
    MaximumRole,
    StepRole,
    ResettableRole,
    SubItemRole,
    VisibleRole,
    EnabledRole,
    RequiresRestartRole,
    PlaceholderRole,    // text widget hint
    FileExtensionsRole, // file-picker filter (QStringList)
    DirectoryModeRole,  // file/folder picker: pick a directory
    RouteRole           // link widget: where the row goes
  };

  struct Item {
    QString label;
    QString key;
    QString description;
    QString longDescription;
    QString widget; // UI control id: toggle / dropdown / slider / spinbox / ...

    bool isBoolean = false;
    QString stringValue; // effective value (string form)
    bool boolValue = true; // effective value (for boolean settings)
    QString trueValue = "true";
    QString falseValue = "false";

    QString defaultValue;
    QVector<QVariantHash> options; // {label, value}
    double minimumValue = 0;
    double maximumValue = 0;
    double stepValue = 1;
    bool requiresRestart = false;

    QString placeholder;        // text widget hint
    QStringList fileExtensions; // file-picker filter
    bool directoryMode = false; // file/folder picker: pick a directory
    QString route;              // link widget: where the row goes

    // Reset clears this tier's override so the row falls back to what it
    // inherits. Only matters for game and platform settings
    bool resettable = false;
    bool subItem = false;
    bool visible = true;
    bool enabled = true;
    bool advanced = false; // hidden unless "Show advanced settings" is on
    bool appScope = false;
    std::vector<SettingCondition> visibleWhen;
    std::vector<SettingCondition> enabledWhen;
    std::optional<bool> subItemOverride;
  };

  void rebuildItems();

  void markSubItems();

  // Builds a library-game-picker's options from the user's library (a leading
  // "None" plus each eligible entry as {label: display name, value: content
  // hash})
  [[nodiscard]] QVector<QVariantHash> buildGameOptions(const SettingDefinition &setting) const;

  // The machine's audio outputs, plus a leading "System default" (stored as "")
  [[nodiscard]] QVector<QVariantHash> buildAudioDeviceOptions() const;
  void refreshValues();
  void recomputeConditions();
  void setItemValue(int itemIndex, Item &item, const std::string &value);

  [[nodiscard]] bool overridesInheritedValue(const Item &item) const;

  [[nodiscard]] std::optional<std::string> resolveValueFrom(const std::string &key, SettingsLevel level) const;

  [[nodiscard]] SettingsLevel levelFor(const Item &item) const;

  [[nodiscard]] bool canResolve(const Item &item) const;

  [[nodiscard]] std::optional<std::string> resolveValue(const std::string &key, SettingsLevel level) const;

  [[nodiscard]] std::string currentValueOf(const std::string &key) const;

  SettingsService *m_settingsService = SettingsService::instance();
  QMediaDevices *m_mediaDevices = nullptr;
  ScopedConnection m_globalSettingChangedConnection;
  ScopedConnection m_platformSettingChangedConnection;
  ScopedConnection m_gameSettingChangedConnection;
  ScopedConnection m_globalSettingResetConnection;
  ScopedConnection m_platformSettingResetConnection;
  ScopedConnection m_gameSettingResetConnection;

  QString m_contentHash;
  QString m_group;
  int m_platformId = -1;
  SettingsLevel m_level = Unknown;
  bool m_showAdvanced = false;

  QVector<Item> m_items;
};
} // namespace firelight::settings

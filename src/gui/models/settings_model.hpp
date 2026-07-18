#pragma once
#include "service_accessor.hpp"

#include <QAbstractListModel>
#include <QAudioDevice>
#include <QMediaDevices>
#include <firelight/event_dispatcher.hpp>
#include <firelight/settings/setting_definition.hpp>
#include <firelight/settings/settings_service.hpp>

#include <optional>
#include <vector>

namespace firelight::settings {

// The rows of one declared settings group. Definitions come from the
// SettingsCatalog; values come from SettingsService and are shown as the
// *effective* value (this tier's override, else the inherited one), with
// `resettable` marking a row whose override actually differs from what it would
// inherit
//
// A group can mix app and emulation settings. App settings are single-valued:
// they always read and write the global tier and ignore `level` entirely
class SettingsModel : public QAbstractListModel,
                               public ServiceAccessor {
  Q_OBJECT
  Q_PROPERTY(int platformId READ getPlatformId WRITE setPlatformId NOTIFY
                 platformIdChanged)
  Q_PROPERTY(int level READ getLevel WRITE setLevel NOTIFY levelChanged)
  Q_PROPERTY(QString contentHash READ getContentHash WRITE setContentHash NOTIFY
                 contentHashChanged)
  // Which declared group to show. Required — every settings surface renders one
  // group at a time
  Q_PROPERTY(QString group READ getGroup WRITE setGroup NOTIFY groupChanged)
  // The group's declared title, so a group renders its own card heading
  Q_PROPERTY(QString groupLabel READ getGroupLabel NOTIFY groupChanged)
  // When false, settings marked `advanced` in the catalog are hidden. Bound in
  // QML to the global "Show advanced settings" preference
  Q_PROPERTY(bool showAdvanced READ getShowAdvanced WRITE setShowAdvanced NOTIFY
                 showAdvancedChanged)
public:
  explicit SettingsModel(QObject *parent = nullptr);

  int getPlatformId() const;
  void setPlatformId(int platformId);

  QString getGroup() const;
  void setGroup(const QString &group);

  QString getGroupLabel() const;

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
  // value (lower tier -> catalog default)
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
    DirectoryModeRole   // file/folder picker: pick a directory
  };
  struct Item {
    QString label;
    QString key;
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

    // Reset clears this tier's override so the row falls back to what it
    // inherits. Only meaningful at a tier with something beneath it — the
    // global tier and app settings are the base, so there's nothing to fall
    // back to and no Reset to show
    bool resettable = false;
    // Depends on another row in the same group, so it renders indented and
    // welded beneath it
    bool subItem = false;
    bool visible = true;
    bool enabled = true;
    bool advanced = false; // hidden unless "Show advanced settings" is on
    // Single-valued: reads and writes the global tier whatever `level` says
    bool appScope = false;
    std::vector<SettingCondition> visibleWhen;
    std::vector<SettingCondition> enabledWhen;
  };

  void rebuildItems();
  // Flags rows that depend on another row in the same view
  void markSubItems();
  // Builds a library-game-picker's options from the user's library (a leading
  // "None" plus each eligible entry as {label: display name, value: content
  // hash}). Returns just "None" when no library is wired
  QVector<QVariantHash> buildGameOptions(const SettingDefinition &setting) const;
  // The machine's audio outputs, plus a leading "System default" (stored as "")
  [[nodiscard]] QVector<QVariantHash> buildAudioDeviceOptions() const;
  void refreshValues();
  void recomputeConditions();
  void setItemValue(int itemIndex, Item &item, const std::string &value);
  // Whether this row has an override at the current tier that actually differs
  // from what it would otherwise inherit — the only case where Reset does
  // anything visible
  [[nodiscard]] bool overridesInheritedValue(const Item &item) const;
  // The value this row would resolve to starting at `level`, ignoring anything
  // above it
  [[nodiscard]] std::optional<std::string>
  resolveValueFrom(const std::string &key, SettingsLevel level) const;
  // The tier this row reads and writes: `level`, except app settings which are
  // always Global.
  [[nodiscard]] SettingsLevel levelFor(const Item &item) const;
  // Whether the row's tier is actually addressable yet (a Game-tier row needs a
  // content hash, a Platform-tier row needs a platform)
  [[nodiscard]] bool canResolve(const Item &item) const;
  // First override at `level` or a lower tier (toward Global); nullopt if none
  std::optional<std::string> resolveValue(const std::string &key,
                                          SettingsLevel level);
  // Current effective value of a sibling setting (for condition evaluation)
  std::string currentValueOf(const std::string &key) const;

  SettingsService *m_settingsService = SettingsService::instance();
  QMediaDevices *m_mediaDevices = nullptr;
  ScopedConnection m_globalSettingChangedConnection;
  ScopedConnection m_platformSettingChangedConnection;
  ScopedConnection m_gameSettingChangedConnection;

  QString m_contentHash;
  QString m_group;
  int m_platformId = -1;
  SettingsLevel m_level = Unknown;
  bool m_showAdvanced = false;

  QVector<Item> m_items;
};
} // namespace firelight::settings

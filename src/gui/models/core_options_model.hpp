#pragma once
#include "service_accessor.hpp"

#include <firelight/event_dispatcher.hpp>
#include <firelight/settings/settings_service.hpp>

#include <QAbstractListModel>
#include <optional>

namespace firelight::settings {

// Backs the "advanced" core-options editor: lists the raw libretro options a
// core declared (cached by ICoreOptionRepository, populated after a core loads)
// and lets the user override any of them at the current tier (Game / Platform /
// Global). Definitions come from the repository; values from SettingsService
class CoreOptionsModel : public QAbstractListModel, public ServiceAccessor {
  Q_OBJECT
  Q_PROPERTY(int platformId READ getPlatformId WRITE setPlatformId NOTIFY platformIdChanged)
  Q_PROPERTY(int level READ getLevel WRITE setLevel NOTIFY levelChanged)
  Q_PROPERTY(QString contentHash READ getContentHash WRITE setContentHash NOTIFY contentHashChanged)
  // Only options in this category are listed ("" = top-level/uncategorized)
  Q_PROPERTY(QString categoryFilter READ getCategoryFilter WRITE setCategoryFilter NOTIFY categoryFilterChanged)
  // Distinct non-empty categories the core declares, as [{key, label}]
  Q_PROPERTY(QVariantList categories READ categories NOTIFY categoriesChanged)
public:
  explicit CoreOptionsModel(QObject *parent = nullptr);

  int getPlatformId() const;
  void setPlatformId(int platformId);

  int getLevel() const;
  void setLevel(int level);

  QString getContentHash() const;
  void setContentHash(const QString &contentHash);

  QString getCategoryFilter() const;
  void setCategoryFilter(const QString &categoryFilter);

  // Distinct non-empty categories this core declares, as [{key, label}] — used
  // to build the drill-in entries in the top-level advanced view
  [[nodiscard]] QVariantList categories() const;

  int rowCount(const QModelIndex &parent) const override;
  QVariant data(const QModelIndex &index, int role) const override;
  QHash<int, QByteArray> roleNames() const override;
  Qt::ItemFlags flags(const QModelIndex &index) const override;
  bool setData(const QModelIndex &index, const QVariant &value, int role) override;

  // Clears this tier's override for the row so it falls back to the inherited
  // value (lower tier -> the core's declared default)
  Q_INVOKABLE void resetValue(int row);

signals:
  void platformIdChanged();
  void levelChanged();
  void contentHashChanged();
  void categoryFilterChanged();
  void categoriesChanged();

private:
  enum Roles {
    KeyRole = Qt::UserRole + 1,
    LabelRole,
    DescriptionRole,
    ValueRole,
    DefaultValueRole,
    OptionsRole,
    OverriddenRole,
    CategoryKeyRole,
    CategoryLabelRole
  };

  struct Item {
    QString key;
    QString label;
    QString description;
    QString defaultValue;
    QString value;
    QVector<QVariantHash> options; // {label, value}
    bool overridden = false;
    QString category;
    QString categoryLabel;
  };

  // Reloads option definitions from the repository (platform's core)
  void rebuild();
  // Recomputes each row's effective value + whether it's overridden here
  void refreshValues();
  // First override at m_level or a lower tier (toward Global); nullopt if none
  std::optional<std::string> resolveValue(const std::string &key);

  SettingsService *m_settingsService = SettingsService::instance();
  ScopedConnection m_platformSettingChangedConnection;
  ScopedConnection m_gameSettingChangedConnection;
  QString m_contentHash;
  int m_platformId = -1;
  SettingsLevel m_level = Unknown;
  QString m_categoryFilter;
  QVector<Item> m_items;
  QVector<QVariantHash> m_categories; // distinct {key, label}
};

} // namespace firelight::settings

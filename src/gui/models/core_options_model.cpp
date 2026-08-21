#include "core_options_model.hpp"

#include "libretro/core_registry.hpp"

#include <firelight/event_dispatcher.hpp>
#include <firelight/settings/core_option_repository.hpp>

#include <spdlog/spdlog.h>

namespace firelight::settings {

CoreOptionsModel::CoreOptionsModel(QObject *parent) : QAbstractListModel(parent) {
  // Changing the resolved core (the "core" key) at a matching scope changes
  // which raw options exist, so rebuild the list
  m_platformSettingChangedConnection =
      EventDispatcher::instance().subscribe<PlatformSettingChangedEvent>([this](const PlatformSettingChangedEvent &e) {
        if (e.platformId == m_platformId && e.key == CoreRegistry::CORE_SETTING_KEY) {
          rebuild();
          refreshValues();
        }
      });
  m_gameSettingChangedConnection =
      EventDispatcher::instance().subscribe<GameSettingChangedEvent>([this](const GameSettingChangedEvent &e) {
        if (e.contentHash == m_contentHash.toStdString() && e.key == CoreRegistry::CORE_SETTING_KEY) {
          rebuild();
          refreshValues();
        }
      });
}

int CoreOptionsModel::getPlatformId() const { return m_platformId; }

void CoreOptionsModel::setPlatformId(const int platformId) {
  if (platformId == m_platformId) {
    return;
  }
  m_platformId = platformId;
  emit platformIdChanged();
  rebuild();
  refreshValues();
}

int CoreOptionsModel::getLevel() const { return m_level; }

void CoreOptionsModel::setLevel(int level) {
  if (level == static_cast<int>(m_level)) {
    return;
  }
  m_level = static_cast<SettingsLevel>(level);
  emit levelChanged();
  refreshValues();
}

QString CoreOptionsModel::getContentHash() const { return m_contentHash; }

void CoreOptionsModel::setContentHash(const QString &contentHash) {
  if (contentHash == m_contentHash) {
    return;
  }
  m_contentHash = contentHash;
  emit contentHashChanged();
  refreshValues();
}

void CoreOptionsModel::rebuild() {
  beginResetModel();
  m_items.clear();
  m_categories.clear();

  const auto coreName =
      CoreRegistry::instance().resolveCoreName(m_platformId, m_contentHash.toStdString(), m_settingsService);
  const auto repository = getCoreOptionRepository();
  if (!coreName.empty() && repository) {
    const auto filter = m_categoryFilter.toStdString();
    for (const auto &option : repository->getCoreOptions(coreName)) {
      // Collect the distinct non-empty categories across all options (for the
      // top-level drill-in list), regardless of the current filter
      if (!option.category.empty()) {
        const auto key = QString::fromStdString(option.category);
        bool seen = false;
        for (const auto &c : m_categories) {
          if (c.value("key").toString() == key) {
            seen = true;
            break;
          }
        }
        if (!seen) {
          m_categories.emplace_back(
              QVariantHash{{"key", key}, {"label", QString::fromStdString(option.categoryLabel)}});
        }
      }

      if (option.category != filter) {
        continue;
      }

      Item item;
      item.key = QString::fromStdString(option.key);
      item.label = QString::fromStdString(option.label);
      item.description = QString::fromStdString(option.description);
      item.defaultValue = QString::fromStdString(option.defaultValue);
      item.value = item.defaultValue;
      item.category = QString::fromStdString(option.category);
      item.categoryLabel = QString::fromStdString(option.categoryLabel);
      for (const auto &value : option.values) {
        item.options.emplace_back(QVariantHash{{"label", QString::fromStdString(value.label)},
                                               {"value", QString::fromStdString(value.value)}});
      }
      m_items.emplace_back(std::move(item));
    }
  }

  endResetModel();
  emit categoriesChanged();
}

QString CoreOptionsModel::getCategoryFilter() const { return m_categoryFilter; }

void CoreOptionsModel::setCategoryFilter(const QString &categoryFilter) {
  if (categoryFilter == m_categoryFilter) {
    return;
  }
  m_categoryFilter = categoryFilter;
  emit categoryFilterChanged();
  rebuild();
  refreshValues();
}

QVariantList CoreOptionsModel::categories() const {
  QVariantList result;
  for (const auto &c : m_categories) {
    result.append(c);
  }
  return result;
}

std::optional<std::string> CoreOptionsModel::resolveValue(const std::string &key) {
  if (!m_settingsService) {
    return std::nullopt;
  }
  const auto hash = m_contentHash.toStdString();
  // From this tier toward the base (Global); first override wins
  if (m_level <= Game) {
    if (auto v = m_settingsService->getValueAtLevel(Game, hash, m_platformId, key)) {
      return v;
    }
  }
  if (m_level <= Platform) {
    if (auto v = m_settingsService->getValueAtLevel(Platform, hash, m_platformId, key)) {
      return v;
    }
  }
  if (auto v = m_settingsService->getValueAtLevel(Global, hash, m_platformId, key)) {
    return v;
  }
  return std::nullopt;
}

void CoreOptionsModel::refreshValues() {
  if (m_level == Unknown || m_items.isEmpty()) {
    return;
  }

  for (auto &item : m_items) {
    const auto key = item.key.toStdString();
    const auto resolved = resolveValue(key);
    item.value = resolved ? QString::fromStdString(*resolved) : item.defaultValue;
    item.overridden =
        m_settingsService &&
        m_settingsService->getValueAtLevel(m_level, m_contentHash.toStdString(), m_platformId, key).has_value();
  }

  if (!m_items.isEmpty()) {
    emit dataChanged(index(0), index(m_items.size() - 1), {ValueRole, OverriddenRole});
  }
}

int CoreOptionsModel::rowCount(const QModelIndex &parent) const { return parent.isValid() ? 0 : m_items.size(); }

QVariant CoreOptionsModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid() || index.row() >= m_items.size()) {
    return {};
  }
  const auto &item = m_items.at(index.row());
  switch (role) {
  case KeyRole:
    return item.key;
  case LabelRole:
    return item.label;
  case DescriptionRole:
    return item.description;
  case ValueRole:
    return item.value;
  case DefaultValueRole:
    return item.defaultValue;
  case OptionsRole:
    return QVariant::fromValue(item.options);
  case OverriddenRole:
    return item.overridden;
  case CategoryKeyRole:
    return item.category;
  case CategoryLabelRole:
    return item.categoryLabel;
  default:
    return {};
  }
}

QHash<int, QByteArray> CoreOptionsModel::roleNames() const {
  return {{KeyRole, "key"},
          {LabelRole, "label"},
          {DescriptionRole, "description"},
          {ValueRole, "value"},
          {DefaultValueRole, "defaultValue"},
          {OptionsRole, "options"},
          {OverriddenRole, "overridden"},
          {CategoryKeyRole, "categoryKey"},
          {CategoryLabelRole, "categoryLabel"}};
}

Qt::ItemFlags CoreOptionsModel::flags(const QModelIndex &index) const {
  return QAbstractListModel::flags(index) | Qt::ItemIsEditable;
}

bool CoreOptionsModel::setData(const QModelIndex &index, const QVariant &value, const int role) {
  if (!index.isValid() || index.row() >= m_items.size() || role != ValueRole || m_level == Unknown ||
      !m_settingsService) {
    return false;
  }

  auto &item = m_items[index.row()];
  const auto newValue = value.toString();
  m_settingsService->setValueAtLevel(m_level, m_contentHash.toStdString(), m_platformId, item.key.toStdString(),
                                     newValue.toStdString());
  item.value = newValue;
  item.overridden = true;
  emit dataChanged(index, index, {ValueRole, OverriddenRole});
  return true;
}

void CoreOptionsModel::resetValue(int row) {
  if (row < 0 || row >= m_items.size() || m_level == Unknown || !m_settingsService) {
    return;
  }

  auto &item = m_items[row];
  m_settingsService->resetValueAtLevel(m_level, m_contentHash.toStdString(), m_platformId, item.key.toStdString());
  const auto resolved = resolveValue(item.key.toStdString());
  item.value = resolved ? QString::fromStdString(*resolved) : item.defaultValue;
  item.overridden = false;
  emit dataChanged(index(row), index(row), {ValueRole, OverriddenRole});
}

} // namespace firelight::settings

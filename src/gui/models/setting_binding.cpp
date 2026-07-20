#include "setting_binding.hpp"

#include <firelight/settings/settings_catalog.hpp>

namespace firelight::settings {

namespace {
const SettingDefinition *definitionFor(const QString &key) {
  if (key.isEmpty()) {
    return nullptr;
  }
  return SettingsCatalog::instance().findByKey(key.toStdString());
}
} // namespace

SettingBinding::SettingBinding(QObject *parent) : QObject(parent) {
  const auto onKey = [this](const std::string &key) {
    if (key == m_key.toStdString()) {
      refresh();
    }
  };
  m_changedConnection = EventDispatcher::instance().subscribe<GlobalSettingChangedEvent>(
      [onKey](const GlobalSettingChangedEvent &e) { onKey(e.key); });
  m_resetConnection = EventDispatcher::instance().subscribe<GlobalSettingResetEvent>(
      [onKey](const GlobalSettingResetEvent &e) { onKey(e.key); });
}

QString SettingBinding::key() const { return m_key; }

void SettingBinding::setKey(const QString &key) {
  if (m_key == key) {
    return;
  }
  m_key = key;
  refresh();
  emit keyChanged();
}

void SettingBinding::refresh() {
  auto *service = SettingsService::instance();
  const auto *definition = definitionFor(m_key);

  std::optional<std::string> stored;
  if (service && !m_key.isEmpty()) {
    stored = service->getGlobalEffectiveValue(m_key.toStdString());
  }

  const auto resolved = QString::fromStdString(stored.value_or(definition ? definition->defaultValue : std::string{}));
  const bool overridden = stored.has_value();

  if (resolved == m_value && overridden == m_overridden) {
    return;
  }
  m_value = resolved;
  m_overridden = overridden;
  emit valueChanged();
}

QString SettingBinding::value() const { return m_value; }

void SettingBinding::setValue(const QString &value) {
  if (m_value == value) {
    return;
  }
  auto *service = SettingsService::instance();
  if (!service || m_key.isEmpty()) {
    return;
  }
  // The write publishes a change event, which lands back in refresh() and emits
  // valueChanged
  service->setGlobalValue(m_key.toStdString(), value.toStdString());
  refresh();
}

void SettingBinding::reset() {
  auto *service = SettingsService::instance();
  if (!service || m_key.isEmpty()) {
    return;
  }
  service->resetGlobalValue(m_key.toStdString());
  refresh();
}

QString SettingBinding::label() const {
  const auto *definition = definitionFor(m_key);
  return definition ? QString::fromStdString(definition->label) : QString();
}

QString SettingBinding::description() const {
  const auto *definition = definitionFor(m_key);
  return definition ? QString::fromStdString(definition->description) : QString();
}

QString SettingBinding::defaultValue() const {
  const auto *definition = definitionFor(m_key);
  return definition ? QString::fromStdString(definition->defaultValue) : QString();
}

QString SettingBinding::widget() const {
  const auto *definition = definitionFor(m_key);
  return definition ? QString::fromStdString(definition->widget) : QString();
}

QVariantList SettingBinding::options() const {
  QVariantList options;
  const auto *definition = definitionFor(m_key);
  if (!definition) {
    return options;
  }
  for (const auto &option : definition->options) {
    options.append(
        QVariantMap{{"label", QString::fromStdString(option.label)}, {"value", QString::fromStdString(option.value)}});
  }
  return options;
}

bool SettingBinding::overridden() const { return m_overridden; }

bool SettingBinding::declared() const { return definitionFor(m_key) != nullptr; }

} // namespace firelight::settings

#pragma once

#include <QObject>
#include <QVariantList>

#include <firelight/event_dispatcher.hpp>
#include <firelight/settings/settings_service.hpp>

namespace firelight::settings {

// One declared setting, bound by key: its value plus the metadata the catalog
// declares for it. Reads and writes the global tier, which is the whole of an
// app setting's story — no core mapping, nothing overriding it per-platform or
// per-game.
//
// It backs the typed facades (AppearanceSettings, GeneralSettings) so the rest
// of the app keeps binding to named properties rather than string keys, and it
// stays in sync when the value changes elsewhere — the settings page writing the
// same key, say — by way of the settings-changed events.
class SettingBinding : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString key READ key WRITE setKey NOTIFY keyChanged)
  Q_PROPERTY(QString value READ value WRITE setValue NOTIFY valueChanged)
  Q_PROPERTY(QString label READ label NOTIFY keyChanged)
  Q_PROPERTY(QString description READ description NOTIFY keyChanged)
  Q_PROPERTY(QString defaultValue READ defaultValue NOTIFY keyChanged)
  Q_PROPERTY(QString widget READ widget NOTIFY keyChanged)
  Q_PROPERTY(QVariantList options READ options NOTIFY keyChanged)
  // Whether a value is stored for this key at all; false means it's showing the
  // catalog default.
  Q_PROPERTY(bool overridden READ overridden NOTIFY valueChanged)
  // False when the key isn't declared — a typo, or a stale facade property.
  Q_PROPERTY(bool declared READ declared NOTIFY keyChanged)

public:
  explicit SettingBinding(QObject *parent = nullptr);

  [[nodiscard]] QString key() const;
  void setKey(const QString &key);

  [[nodiscard]] QString value() const;
  void setValue(const QString &value);

  [[nodiscard]] QString label() const;
  [[nodiscard]] QString description() const;
  [[nodiscard]] QString defaultValue() const;
  [[nodiscard]] QString widget() const;
  [[nodiscard]] QVariantList options() const;
  [[nodiscard]] bool overridden() const;
  [[nodiscard]] bool declared() const;

  // Clears the stored value so the setting falls back to the catalog default.
  Q_INVOKABLE void reset();

signals:
  void keyChanged();
  void valueChanged();

private:
  void refresh();

  QString m_key;
  QString m_value;
  bool m_overridden = false;
  ScopedConnection m_changedConnection;
  ScopedConnection m_resetConnection;
};

} // namespace firelight::settings

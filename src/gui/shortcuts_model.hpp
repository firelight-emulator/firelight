#pragma once
#include "service_accessor.hpp"

#include <firelight/input/gamepad_type.hpp>
#include <firelight/input/input_source.hpp>
#include <firelight/input/shortcut_mapping.hpp>

#include <QAbstractListModel>
#include <memory>

namespace firelight::gui {

// Lists the global shortcut catalog (from ShortcutRegistry) alongside the
// bindings a single profile has for each action. One row per action
class ShortcutsModel : public QAbstractListModel, public ServiceAccessor {
  Q_OBJECT

  Q_PROPERTY(QString presetId READ presetId NOTIFY presetIdChanged)

public:
  ShortcutsModel(int profileId, bool isKeyboard, std::shared_ptr<input::ShortcutMapping> shortcutMapping,
                 std::string presetId);

  int rowCount(const QModelIndex &parent) const override;
  QVariant data(const QModelIndex &index, int role) const override;
  [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

  // Appends a binding to a shortcut. `modifiers`/`input` are GamepadInput codes
  // for a controller profile, or Qt::Key codes for a keyboard profile
  Q_INVOKABLE void addBinding(const QString &shortcutId, QList<int> modifiers, int input);
  // Unbinds a shortcut, which is how a shortcut is turned off — there is no
  // separate disabled state
  Q_INVOKABLE void clearBindings(const QString &shortcutId);

  // Puts one row back to what the profile's preset ships for it (a write, not an
  // erase: a preset that leaves it unbound resets it to unbound)
  Q_INVOKABLE void resetToDefault(const QString &shortcutId);

  // Reseeds every row from a preset and adopts it as the one rows are measured
  // against. Overwrites edits, so the UI should confirm first
  Q_INVOKABLE void applyPreset(const QString &presetId);

  [[nodiscard]] QString presetId() const;

  // The presets that apply to this profile's device, as {id, label} rows for a
  // picker: a keyboard is never offered a gamepad preset
  Q_INVOKABLE QVariantList presetOptions() const;

  // The input codes the assign prompt should watch for being held, so it can
  // record a combo. Empty for a keyboard profile — Qt reports those modifiers
  // on the key event itself
  Q_INVOKABLE QList<int> modifierCandidates() const;

signals:
  void presetIdChanged();

private:
  enum Roles {
    ShortcutIdRole = Qt::UserRole + 1,
    NameRole,
    CategoryRole,
    ActivationRole,
    HasBindingRole,
    BindingsLabelRole,
    IsModifiedRole,
  };

  struct Item {
    QString id;
    QString name;
    QString category;
    int activation = 0;
    bool hasBinding = false;
    QString bindingsLabel;
    bool isModified = false;
  };

  void rebuild();
  void refreshRow(const QString &id);
  int indexOfId(const QString &id) const;
  QString labelForBindings(const std::vector<input::InputSource> &sources) const;
  [[nodiscard]] DeviceType deviceType() const;
  // What the profile's preset ships for an action; empty when it ships nothing
  // or the preset is gone
  [[nodiscard]] std::vector<input::InputSource> presetSourcesFor(const input::ShortcutId &id) const;
  [[nodiscard]] bool differsFromPreset(const input::ShortcutId &id) const;

  int m_profileId = -1;
  std::shared_ptr<input::ShortcutMapping> m_shortcutMapping;
  bool m_isKeyboard;
  std::string m_presetId;
  QList<Item> m_items;
};

} // namespace firelight::gui

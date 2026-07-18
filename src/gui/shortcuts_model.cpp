#include "shortcuts_model.hpp"

#include <firelight/input/keyboard_input_handler.hpp>
#include <firelight/input/controller_repository.hpp>
#include <firelight/input/shortcut_registry.hpp>
#include <firelight/input/gamepad_input.hpp>

#include <algorithm>
#include <utility>

namespace firelight::gui {

ShortcutsModel::ShortcutsModel(
    const int profileId, const bool isKeyboard,
    std::shared_ptr<input::ShortcutMapping> shortcutMapping,
    std::string presetId)
    : QAbstractListModel(nullptr), m_profileId(profileId),
      m_shortcutMapping(std::move(shortcutMapping)), m_isKeyboard(isKeyboard),
      m_presetId(std::move(presetId)) {
  rebuild();
}

DeviceType ShortcutsModel::deviceType() const {
  return m_isKeyboard ? DeviceType::Keyboard : DeviceType::Gamepad;
}

std::vector<input::InputSource>
ShortcutsModel::presetSourcesFor(const input::ShortcutId &id) const {
  const auto *preset =
      input::ShortcutRegistry::instance().findPreset(m_presetId);
  if (!preset) {
    return {};
  }
  return preset->sourcesFor(deviceType(), id);
}

bool ShortcutsModel::differsFromPreset(const input::ShortcutId &id) const {
  if (!m_shortcutMapping) {
    return false;
  }
  return m_shortcutMapping->getBindings(id) != presetSourcesFor(id);
}

QString ShortcutsModel::labelForBindings(
    const std::vector<input::InputSource> &sources) const {
  const auto label = [this](const int code) {
    return m_isKeyboard
               ? input::KeyboardInputHandler::getKeyLabel(
                     static_cast<Qt::Key>(code))
               : QString::fromStdString(firelight::input::displayName(
                     static_cast<input::GamepadInput>(code)));
  };

  QStringList combos;
  for (const auto &source : sources) {
    QStringList parts;
    for (const auto mod : source.modifiers) {
      parts << label(mod);
    }
    parts << label(source.code);
    combos << parts.join(" + ");
  }
  return combos.join(",  ");
}

void ShortcutsModel::rebuild() {
  beginResetModel();
  m_items.clear();

  auto actions = input::ShortcutRegistry::instance().listActions();
  std::sort(actions.begin(), actions.end(),
            [](const input::ShortcutAction &a, const input::ShortcutAction &b) {
              if (a.category != b.category) {
                return a.category < b.category;
              }
              return a.displayName < b.displayName;
            });

  for (const auto &action : actions) {
    Item item;
    item.id = QString::fromStdString(action.id);
    item.name = QString::fromStdString(action.displayName);
    item.category = QString::fromStdString(action.category);
    item.activation = static_cast<int>(action.activation);
    if (m_shortcutMapping) {
      const auto &bindings = m_shortcutMapping->getBindings(action.id);
      item.hasBinding = !bindings.empty();
      item.bindingsLabel =
          item.hasBinding ? labelForBindings(bindings) : QStringLiteral("Not bound");
      item.isModified = differsFromPreset(action.id);
    } else {
      item.bindingsLabel = QStringLiteral("Not bound");
    }
    m_items.append(item);
  }
  endResetModel();
}

void ShortcutsModel::refreshRow(const QString &id) {
  const int row = indexOfId(id);
  if (row < 0 || !m_shortcutMapping) {
    return;
  }
  auto &item = m_items[row];
  const auto &bindings = m_shortcutMapping->getBindings(id.toStdString());
  item.hasBinding = !bindings.empty();
  item.bindingsLabel =
      item.hasBinding ? labelForBindings(bindings) : QStringLiteral("Not bound");
  item.isModified = differsFromPreset(id.toStdString());
  emit dataChanged(index(row), index(row));
}

int ShortcutsModel::indexOfId(const QString &id) const {
  for (int i = 0; i < m_items.size(); ++i) {
    if (m_items[i].id == id) {
      return i;
    }
  }
  return -1;
}

int ShortcutsModel::rowCount(const QModelIndex &parent) const {
  if (parent.isValid()) {
    return 0;
  }
  return static_cast<int>(m_items.size());
}

QVariant ShortcutsModel::data(const QModelIndex &index, const int role) const {
  if (!index.isValid() || index.row() < 0 || index.row() >= m_items.size()) {
    return {};
  }
  const auto &item = m_items.at(index.row());
  switch (role) {
  case ShortcutIdRole:
    return item.id;
  case NameRole:
    return item.name;
  case CategoryRole:
    return item.category;
  case ActivationRole:
    return item.activation;
  case HasBindingRole:
    return item.hasBinding;
  case BindingsLabelRole:
    return item.bindingsLabel;
  case IsModifiedRole:
    return item.isModified;
  default:
    return {};
  }
}

QHash<int, QByteArray> ShortcutsModel::roleNames() const {
  return {
      {ShortcutIdRole, "shortcutId"}, {NameRole, "name"},
      {CategoryRole, "category"},     {ActivationRole, "activation"},
      {HasBindingRole, "hasBinding"}, {BindingsLabelRole, "bindingsLabel"},
      {IsModifiedRole, "isModified"},
  };
}

void ShortcutsModel::addBinding(const QString &shortcutId, QList<int> modifiers,
                                const int input) {
  if (!m_shortcutMapping) {
    return;
  }
  input::InputSource source;
  source.type =
      m_isKeyboard ? input::SourceType::Key : input::SourceType::Button;
  source.code = input;
  for (const auto mod : modifiers) {
    source.modifiers.push_back(mod);
  }
  m_shortcutMapping->addBinding(shortcutId.toStdString(), source);
  m_shortcutMapping->sync();
  refreshRow(shortcutId);
}

void ShortcutsModel::clearBindings(const QString &shortcutId) {
  if (!m_shortcutMapping) {
    return;
  }
  m_shortcutMapping->setBindings(shortcutId.toStdString(), {});
  m_shortcutMapping->sync();
  refreshRow(shortcutId);
}

void ShortcutsModel::resetToDefault(const QString &shortcutId) {
  if (!m_shortcutMapping) {
    return;
  }
  const auto id = shortcutId.toStdString();
  m_shortcutMapping->setBindings(id, presetSourcesFor(id));
  m_shortcutMapping->sync();
  refreshRow(shortcutId);
}

void ShortcutsModel::applyPreset(const QString &presetId) {
  const auto id = presetId.toStdString();
  const auto *preset = input::ShortcutRegistry::instance().findPreset(id);
  if (!preset || !m_shortcutMapping) {
    return;
  }

  m_presetId = id;
  if (m_profileId >= 0) {
    if (const auto repository = getControllerProfileRepository()) {
      repository->setProfilePresetId(m_profileId, id);
    }
  }

  // Every action, not just the ones the preset names: one it leaves out is one
  // it wants unbound, so a leftover binding from the previous preset would
  // survive as a phantom
  for (const auto &action : input::ShortcutRegistry::instance().listActions()) {
    m_shortcutMapping->setBindings(action.id,
                                   preset->sourcesFor(deviceType(), action.id));
  }
  m_shortcutMapping->sync();

  rebuild();
  emit presetIdChanged();
}

QString ShortcutsModel::presetId() const {
  return QString::fromStdString(m_presetId);
}

QList<int> ShortcutsModel::modifierCandidates() const {
  if (m_isKeyboard) {
    return {};
  }
  QList<int> codes;
  for (const auto input : input::modifierCandidates()) {
    codes.append(static_cast<int>(input));
  }
  return codes;
}

QVariantList ShortcutsModel::presetOptions() const {
  QVariantList options;
  for (const auto &preset : input::ShortcutRegistry::instance().presets()) {
    if (!preset.appliesTo(deviceType())) {
      continue;
    }
    options.append(QVariantMap{
        {"id", QString::fromStdString(preset.id)},
        {"label", QString::fromStdString(preset.label)},
    });
  }
  return options;
}

} // namespace firelight::gui

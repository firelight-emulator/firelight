#include "gamepad_profile_item.hpp"

#include <firelight/input/controller_repository.hpp>
#include "shortcuts_model.hpp"

namespace firelight::gui {
int GamepadProfileItem::id() const { return m_id; }
void GamepadProfileItem::setId(const int id) {
  if (m_id != id) {
    m_id = id;

    m_profile = getControllerProfileRepository()->getProfile(m_id);
    m_shortcutsModel =
        m_profile ? std::make_unique<ShortcutsModel>(
                        m_profile->getId(), m_profile->isKeyboardProfile(),
                        m_profile->getShortcutMapping(),
                        m_profile->getPresetId())
                  : nullptr;
    emit profileIdChanged(m_id);
    emit nameChanged(name());
    emit isKeyboardProfileChanged();
  }
}

QString GamepadProfileItem::name() const {
  if (!m_profile) {
    return {};
  }

  return QString::fromStdString(m_profile->getName());
}

void GamepadProfileItem::setName(const QString &name) {
  if (!m_profile) {
    return;
  }

  auto profileName = QString::fromStdString(m_profile->getName());
  if (profileName != name) {
    m_profile->setName(name.toStdString());
    emit nameChanged(name);
  }
}

QAbstractListModel *GamepadProfileItem::getShortcutsModel() const {
  return m_shortcutsModel.get();
}
bool GamepadProfileItem::isKeyboardProfile() const {
  if (!m_profile) {
    return false;
  }

  return m_profile->isKeyboardProfile();
}
} // namespace firelight::gui
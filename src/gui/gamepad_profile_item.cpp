#include "gamepad_profile_item.hpp"

#include "input2/input_service.hpp"
#include "shortcuts_model.hpp"

namespace firelight::gui {
int GamepadProfileItem::id() const { return m_id; }
void GamepadProfileItem::setId(const int id) {
  if (m_id != id) {
    m_id = id;

    m_profile = input::InputService::instance()->getProfile(m_id);
    emit profileIdChanged(m_id);
  }
}

QString GamepadProfileItem::name() const {
  if (!m_profile) {
    return QString();
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
  if (!m_profile) {
    return nullptr;
  }

  return new ShortcutsModel(m_profile->getShortcutMapping());
}
} // namespace firelight::gui
#include "shortcut_dispatcher.hpp"

namespace firelight::emulation {

ShortcutDispatcher::ShortcutDispatcher(ShortcutActions &actions,
                                       std::function<bool(int)> playerAllowed,
                                       QObject *parent)
    : QObject(parent), m_actions(actions),
      m_playerAllowed(std::move(playerAllowed)) {
  m_shortcutConnection =
      EventDispatcher::instance().subscribe<input::ShortcutEvent>(
          [this](const input::ShortcutEvent &event) {
            if (m_playerAllowed && !m_playerAllowed(event.playerIndex)) {
              return;
            }
            // Published on whichever thread saw the input — the SDL event loop
            // for a pad. Queued so the actions always run on the GUI thread
            QMetaObject::invokeMethod(
                this, "run", Qt::QueuedConnection,
                Q_ARG(QString, QString::fromStdString(event.id)),
                Q_ARG(int, static_cast<int>(event.phase)));
          });

  m_hotkeysToggledConnection =
      EventDispatcher::instance().subscribe<input::HotkeysToggledEvent>(
          [this](const input::HotkeysToggledEvent &event) {
            QMetaObject::invokeMethod(this, "announceHotkeysToggled",
                                      Qt::QueuedConnection,
                                      Q_ARG(int, event.playerIndex),
                                      Q_ARG(bool, event.enabled));
          });
}

void ShortcutDispatcher::run(const QString &id, const int phase) {
  const auto shortcutId = id.toStdString();

  m_actions.handle(shortcutId, static_cast<input::ShortcutPhase>(phase));
}

void ShortcutDispatcher::trigger(const QString &id) {
  // Already on the GUI thread (a click), so no hop — and no player gate: the
  // menu is whoever is holding the mouse
  m_actions.handle(id.toStdString(), input::ShortcutPhase::Started);
}

void ShortcutDispatcher::announceHotkeysToggled(const int playerIndex,
                                                const bool enabled) {
  // Naming the player matters: the switch is per-device, so "off" on one pad
  // says nothing about the others
  const auto who = playerIndex >= 0
                       ? QString(" — Player %1").arg(playerIndex + 1)
                       : QString();
  emit notified((enabled ? QStringLiteral("Hotkeys on")
                         : QStringLiteral("Hotkeys off")) + who);
}

} // namespace firelight::emulation

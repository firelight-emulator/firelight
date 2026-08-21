#pragma once

#include "shortcut_actions.hpp"

#include <firelight/event_dispatcher.hpp>
#include <firelight/input/shortcut_action.hpp>

#include <QObject>
#include <functional>

namespace firelight::emulation {

// Turns shortcut events into actions, and is the only place that has to know
// where they came from
//
// ShortcutEvents are published on the SDL event thread (gamepads) or the GUI
// thread (keyboard), so this hops to the GUI thread before doing anything —
// which is where ShortcutActions and everything it touches live. Owned for the
// life of the app, so the event dispatcher can never call into a dead
// subscriber
class ShortcutDispatcher final : public QObject {
  Q_OBJECT
public:
  // `playerAllowed` decides whether a player's shortcuts are honoured at all
  // Here rather than in ShortcutActions because who pressed it is a routing
  // question — the actions themselves are the same whoever fires them
  explicit ShortcutDispatcher(ShortcutActions &actions, std::function<bool(int)> playerAllowed = {},
                              QObject *parent = nullptr);

  // Runs an action as if a hotkey had fired it, for menus that offer the same
  // thing. The point is that they route here rather than reimplementing it: a
  // second copy of "which slot, and does hardcore allow it" is how the menu and
  // the hotkey come to disagree. Press-only — a menu button isn't held
  Q_INVOKABLE void trigger(const QString &id);

signals:
  // The three actions that need the UI. Everything else is handled in C++
  void openQuickMenuRequested();
  void openRewindMenuRequested();
  void toggleFullscreenRequested();

  // Something a hotkey did, in words, for the UI to show briefly. Already on the
  // GUI thread by the time it's emitted
  void notified(const QString &message);

private:
  Q_INVOKABLE void run(const QString &id, int phase);
  // The engine owns the per-device kill switch, so the message about it comes
  // from there rather than from ShortcutActions
  Q_INVOKABLE void announceHotkeysToggled(int playerIndex, bool enabled);

  ShortcutActions &m_actions;
  std::function<bool(int)> m_playerAllowed;
  ScopedConnection m_shortcutConnection;
  ScopedConnection m_hotkeysToggledConnection;
};

} // namespace firelight::emulation

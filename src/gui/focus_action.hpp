// TODO: NEEDS REVIEW
#pragma once

#include <QDateTime>
#include <QList>
#include <QObject>
#include <QString>
#include <qqmlintegration.h>

namespace firelight::gui {

/**
 * What a handler reports back about a press it was given.
 *
 * Accepted unless the handler says otherwise, so an action that does nothing needs to say nothing.
 * A handler that declines — a navigation a screen refused, a control that could not act — sets
 * accepted false, and the press makes no sound
 */
class FocusActionEvent : public QObject {
  Q_OBJECT
  Q_PROPERTY(bool accepted READ isAccepted WRITE setAccepted)
  QML_ANONYMOUS

public:
  explicit FocusActionEvent(QObject *parent = nullptr) : QObject(parent) {}

  [[nodiscard]] bool isAccepted() const { return m_accepted; }

  void setAccepted(const bool accepted) { m_accepted = accepted; }

private:
  bool m_accepted = true;
};

/**
 * One thing a focused item can do, bound to the keys that trigger it.
 *
 * An item lists several of these so a single focus target can offer more than one outcome — a game
 * tile launching on one button and opening its options on another. The keys are ordinary Qt::Key
 * values; which controller button produces which key is decided elsewhere. An action answers only
 * when the modifiers it declares are exactly the ones held, so a bare key and the same key under
 * Shift are separate outcomes.
 */
class FocusAction : public QObject {
  Q_OBJECT
  Q_PROPERTY(QList<int> keys READ getKeys WRITE setKeys NOTIFY keysChanged)
  Q_PROPERTY(int modifiers READ getModifiers WRITE setModifiers NOTIFY modifiersChanged)
  Q_PROPERTY(QString label READ getLabel WRITE setLabel NOTIFY labelChanged)
  Q_PROPERTY(bool hidden READ isHidden WRITE setHidden NOTIFY hiddenChanged)
  Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)
  Q_PROPERTY(
      bool triggerOnAutoRepeat READ triggerOnAutoRepeat WRITE setTriggerOnAutoRepeat NOTIFY triggerOnAutoRepeatChanged)
  Q_PROPERTY(QObject *sound READ getSound WRITE setSound NOTIFY soundChanged)
  QML_ELEMENT

public:
  explicit FocusAction(QObject *parent = nullptr) : QObject(parent) {}

  /**
   * Fires this action and makes its sound, and answers whether the handler accepted the press.
   * A press the handler declined made nothing happen, so it makes no sound either
   */
  Q_INVOKABLE bool trigger() {
    m_event.setAccepted(true);

    // TODO
    // Claimed before the handler runs, so a screen the handler builds cannot sound in its place
    if (m_sound != nullptr) {
      QMetaObject::invokeMethod(m_sound, "claim");
    }

    emit triggered(&m_event);

    if (m_event.isAccepted() && m_sound != nullptr) {
      QMetaObject::invokeMethod(m_sound, "playClaimed");
    }

    return m_event.isAccepted();
  }

  /**
   * Fires this action for one press. A held key runs it only when it opted into repeating, and no
   * faster than the interval, so a repeating action paces the way a held direction does.
   *
   * @param isAutoRepeat Whether this press comes from a held key
   * @return Whether the action ran and the handler accepted it
   */
  Q_INVOKABLE bool triggerForPress(const bool isAutoRepeat) {
    const auto nowMs = QDateTime::currentMSecsSinceEpoch();

    if (isAutoRepeat && (!m_triggerOnAutoRepeat || nowMs - m_lastTriggerMs < REPEAT_INTERVAL_MS)) {
      return false;
    }

    m_lastTriggerMs = nowMs;
    return trigger();
  }

  /**
   * @return The Qt::Key values that trigger this action
   */
  [[nodiscard]] QList<int> getKeys() const { return m_keys; }

  [[nodiscard]] int getModifiers() const { return m_modifiers; }

  [[nodiscard]] QString getLabel() const { return m_label; }

  [[nodiscard]] bool isEnabled() const { return m_enabled; }

  [[nodiscard]] bool triggerOnAutoRepeat() const { return m_triggerOnAutoRepeat; }

  [[nodiscard]] bool isHidden() const { return m_hidden; }

  [[nodiscard]] QObject *getSound() const { return m_sound; }

  /**
   * @return Whether this action is enabled and answers to key pressed with modifiers held
   */
  [[nodiscard]] bool handles(const int key, const int modifiers = Qt::NoModifier) const {
    return m_enabled && m_keys.contains(key) && (modifiers & HANDLED_MODIFIERS) == (m_modifiers & HANDLED_MODIFIERS);
  }

  void setKeys(const QList<int> &keys) {
    if (m_keys != keys) {
      m_keys = keys;
      emit keysChanged();
    }
  }

  void setModifiers(const int modifiers) {
    if (m_modifiers != modifiers) {
      m_modifiers = modifiers;
      emit modifiersChanged();
    }
  }

  void setLabel(const QString &label) {
    if (m_label != label) {
      m_label = label;
      emit labelChanged();
    }
  }

  void setEnabled(const bool enabled) {
    if (m_enabled != enabled) {
      m_enabled = enabled;
      emit enabledChanged();
    }
  }

  void setTriggerOnAutoRepeat(const bool triggerOnAutoRepeat) {
    if (m_triggerOnAutoRepeat != triggerOnAutoRepeat) {
      m_triggerOnAutoRepeat = triggerOnAutoRepeat;
      emit triggerOnAutoRepeatChanged();
    }
  }

  void setHidden(const bool hidden) {
    if (m_hidden != hidden) {
      m_hidden = hidden;
      emit hiddenChanged();
    }
  }

  void setSound(QObject *sound) {
    if (m_sound != sound) {
      m_sound = sound;
      emit soundChanged();
    }
  }

signals:
  void keysChanged();

  void modifiersChanged();

  void labelChanged();

  void enabledChanged();

  void triggerOnAutoRepeatChanged();

  void hiddenChanged();

  void soundChanged();

  /**
   * One of this action's keys was pressed while its item held focus
   */
  void triggered(FocusActionEvent *event);

private:
  static constexpr int HANDLED_MODIFIERS = Qt::ShiftModifier | Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier;

  // TODO
  // Matches the pacing a held direction navigates at, so the two kinds of repeat feel the same
  static constexpr qint64 REPEAT_INTERVAL_MS = 60;

  QList<int> m_keys;
  int m_modifiers = Qt::NoModifier;
  QString m_label;
  bool m_enabled = true;
  bool m_triggerOnAutoRepeat = false;
  bool m_hidden = false;
  QObject *m_sound = nullptr;
  FocusActionEvent m_event;
  qint64 m_lastTriggerMs = 0;
};

} // namespace firelight::gui

#pragma once

#include <QList>
#include <QObject>
#include <QString>
#include <qqmlintegration.h>

namespace firelight::gui {

/**
 * One thing a focused item can do, bound to the keys that trigger it.
 *
 * An item lists several of these so a single focus target can offer more than one outcome — a game
 * tile launching on one button and opening its options on another. The keys are ordinary Qt::Key
 * values; which controller button produces which key is decided elsewhere.
 */
class FocusAction : public QObject {
  Q_OBJECT
  Q_PROPERTY(QList<int> keys READ getKeys WRITE setKeys NOTIFY keysChanged)
  Q_PROPERTY(QString label READ getLabel WRITE setLabel NOTIFY labelChanged)
  Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)
  Q_PROPERTY(QObject *sound READ getSound WRITE setSound NOTIFY soundChanged)
  QML_ELEMENT

public:
  explicit FocusAction(QObject *parent = nullptr) : QObject(parent) {}

  /**
   * @return The Qt::Key values that trigger this action
   */
  [[nodiscard]] QList<int> getKeys() const { return m_keys; }

  /**
   * @return Text describing the action, for a prompt showing what the buttons do
   */
  [[nodiscard]] QString getLabel() const { return m_label; }

  /**
   * @return Whether the action can currently be triggered
   */
  [[nodiscard]] bool isEnabled() const { return m_enabled; }

  /**
   * @return The sound to play when the action fires, or null for none
   */
  [[nodiscard]] QObject *getSound() const { return m_sound; }

  /**
   * @return Whether this action is enabled and answers to key
   */
  [[nodiscard]] bool handles(const int key) const { return m_enabled && m_keys.contains(key); }

  void setKeys(const QList<int> &keys) {
    if (m_keys != keys) {
      m_keys = keys;
      emit keysChanged();
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

  void setSound(QObject *sound) {
    if (m_sound != sound) {
      m_sound = sound;
      emit soundChanged();
    }
  }

signals:
  void keysChanged();

  void labelChanged();

  void enabledChanged();

  void soundChanged();

  /**
   * One of this action's keys was pressed while its item held focus
   */
  void triggered();

private:
  QList<int> m_keys;
  QString m_label;
  bool m_enabled = true;
  QObject *m_sound = nullptr;
};

} // namespace firelight::gui

#pragma once

#include <QObject>
#include <qqmlintegration.h>

namespace firelight::gui {

class SoundEffect : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString focus READ focus WRITE setFocus NOTIFY focusChanged)
  QML_ELEMENT
  QML_ATTACHED(SoundEffect)

public:
  static SoundEffect *qmlAttachedProperties(QObject *object) { return new SoundEffect(object); }

  explicit SoundEffect(QObject *parent = nullptr) : QObject(parent) {}

  [[nodiscard]] QString focus() const { return m_focusedSfx; }

  void setFocus(const QString &focusedSfx) {
    if (m_focusedSfx != focusedSfx) {
      m_focusedSfx = focusedSfx;
      emit focusChanged();
    }
  }

signals:
  void focusChanged();

private:
  QString m_focusedSfx;
};

} // namespace firelight::gui
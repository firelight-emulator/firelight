// TODO: NEEDS REVIEW
#pragma once

#include "audio/ui_sound_player.hpp"

#include <QApplication>
#include <QEvent>

namespace firelight {

// TODO
/**
 * The application, extended so that a UI sound can be attributed to the input that caused it.
 *
 * A press and everything it sets off — a screen being built, a cursor being placed — run inside one
 * delivery, so bracketing that delivery is what lets one sound stand for one press
 */
class FirelightApplication final : public QApplication {
public:
  FirelightApplication(int &argc, char **argv) : QApplication(argc, argv) {}

  // TODO
  /**
   * Points sound attribution at the player, which is built later than the application
   */
  void setSoundPlayer(audio::UiSoundPlayer *soundPlayer) { m_soundPlayer = soundPlayer; }

  // TODO
  /**
   * Delivers an event, bracketing the ones a person caused
   */
  bool notify(QObject *receiver, QEvent *event) override {
    if (m_soundPlayer == nullptr || !isUserInput(event)) {
      return QApplication::notify(receiver, event);
    }

    audio::UiSoundPlayer::GuiInputScope scope(m_soundPlayer);

    return QApplication::notify(receiver, event);
  }

private:
  // TODO
  /**
   * Whether an event is a person acting rather than the application moving. Pointer motion is not:
   * it arrives continuously and would open a span under every hover
   */
  static bool isUserInput(const QEvent *event) {
    switch (event->type()) {
    case QEvent::KeyPress:
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::Wheel:
      return true;
    default:
      return false;
    }
  }

  audio::UiSoundPlayer *m_soundPlayer = nullptr;
};

} // namespace firelight

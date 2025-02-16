#include "SfxPlayer.hpp"

namespace firelight::audio {
    SfxPlayer::SfxPlayer() {
        m_sfxs["rewindscroll"] = new QSoundEffect();
        m_sfxs["rewindscroll"]->setSource(QUrl("qrc:sfx/click"));

        m_sfxs["rewindopen"] = new QSoundEffect();
        m_sfxs["rewindopen"]->setSource(QUrl("qrc:sfx/open-rewind"));

        m_sfxs["nope"] = new QSoundEffect();
        m_sfxs["nope"]->setSource(QUrl("qrc:sfx/nope"));

        m_sfxs["quickopen"] = new QSoundEffect();
        m_sfxs["quickopen"]->setSource(QUrl("qrc:sfx/open-quick"));

        m_sfxs["quickclose"] = new QSoundEffect();
        m_sfxs["quickclose"]->setSource(QUrl("qrc:sfx/close-quick"));

        m_sfxs["menumove"] = new QSoundEffect();
        m_sfxs["menumove"]->setSource(QUrl("qrc:sfx/menu-move"));

        m_sfxs["startgame"] = new QSoundEffect();
        m_sfxs["startgame"]->setSource(QUrl("qrc:sfx/start-game"));

        m_sfxs["defaultnotification"] = new QSoundEffect();
        m_sfxs["defaultnotification"]->setSource(QUrl("qrc:sfx/default-notification"));

        m_sfxs["switchon"] = new QSoundEffect();
        m_sfxs["switchon"]->setSource(QUrl("qrc:sfx/switch-on"));

        m_sfxs["switchoff"] = new QSoundEffect();
        m_sfxs["switchoff"]->setSource(QUrl("qrc:sfx/switch-off"));

        m_sfxs["uibump"] = new QSoundEffect();
        m_sfxs["uibump"]->setSource(QUrl("qrc:sfx/ui-bump"));
    }

    SfxPlayer::~SfxPlayer() {
    }

    void SfxPlayer::play() {
    }

    void SfxPlayer::play(const QString &id) {
        if (m_sfxs.contains(id)) {
            // m_sfxs[id]->stop();
            m_sfxs[id]->play();
        }
    }
} // audio
// firelight

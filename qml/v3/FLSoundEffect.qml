import QtQuick
import QtMultimedia

// TODO
// A UI sound. Built on SoundEffect, which decodes the file up front and plays
// with low latency, over a pool of voices so rapid triggers overlap instead of
// cutting each other off
//
//   FLSoundEffect { id: click; source: "qrc:/sfx/blip" }
//   ... click.play()
QtObject {
    id: root

    property url source
    property real volume: 0.4
    property bool muted: false

    // TODO
    // How many plays can overlap before the oldest is recycled; scrolling fast
    // enough to outrun the tail of the sound needs more than one
    property int voices: 4

    // TODO
    // What an auto-repeated play is scaled by
    property real repeatGain: 0.3

    readonly property bool ready: _pool.count > 0 && _pool.objectAt(0) !== null && _pool.objectAt(0).status === SoundEffect.Ready

    property int _next: 0
    property bool _priming: false
    property bool _primed: false

    // TODO
    // Opening the audio device costs more than playing does, so every voice is
    // run once silently as soon as the sample is decoded; the first real play
    // then hits a device that is already open
    onReadyChanged: {
        if (root.ready && !root._primed) {
            root._primed = true;
            root._priming = true;

            for (let i = 0; i < _pool.count; i++) {
                const voice = _pool.objectAt(i);

                if (voice) {
                    voice.play();
                }
            }

            primeTimer.start();
        }
    }

    readonly property Timer _primeTimer: Timer {
        id: primeTimer
        interval: 1
        onTriggered: {
            root.stop();
            root._priming = false;
        }
    }

    // TODO
    // Plays on an idle voice when there is one, so an overlapping trigger adds
    // to the sound rather than restarting it; recycles the oldest when all are
    // busy. A play the caller marks as auto-repeat is scaled by repeatGain
    function play(autoRepeat: bool) {
        const count = _pool.count;

        if (count === 0 || root.muted) {
            return;
        }

        if (root._priming) {
            primeTimer.stop();
            root.stop();
            root._priming = false;
        }

        const gain = autoRepeat ? root.repeatGain : 1;

        for (let i = 0; i < count; i++) {
            const index = (root._next + i) % count;
            const voice = _pool.objectAt(index);

            if (voice && !voice.playing) {
                root._next = (index + 1) % count;
                voice.gain = gain;
                voice.play();
                return;
            }
        }

        const oldest = _pool.objectAt(root._next);
        root._next = (root._next + 1) % count;

        if (oldest) {
            oldest.gain = gain;
            oldest.play();
        }
    }

    function stop() {
        for (let i = 0; i < _pool.count; i++) {
            const voice = _pool.objectAt(i);

            if (voice) {
                voice.stop();
            }
        }
    }

    readonly property Instantiator _pool: Instantiator {
        model: Math.max(1, root.voices)

        delegate: SoundEffect {
            id: voice

            // TODO
            // Per-play scale, kept off the shared volume so setting it never
            // disturbs what the owner bound volume to
            property real gain: 1

            source: root.source
            volume: root.volume * voice.gain
            muted: root.muted || root._priming
        }
    }
}

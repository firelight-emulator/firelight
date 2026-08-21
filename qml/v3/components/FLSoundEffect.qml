import QtQuick

// TODO
// A UI sound. The samples are decoded once at startup and mixed into the app's
// single output stream, so a sound is never waiting on an audio device to wake
// up and overlapping triggers add rather than cut each other off
//
//   FLSoundEffect { id: click; source: "qrc:/sfx/blip" }
//   ... click.play()
QtObject {
    id: root

    property url source
    property real volume: 0.5
    property bool muted: false

    // TODO
    // How many plays can overlap before the oldest is recycled; scrolling fast
    // enough to outrun the tail of the sound needs more than one
    property int voices: 4

    // TODO
    // What an auto-repeated play is scaled by
    property real repeatGain: 0.3

    // TODO
    // How far each play is pitched from the recorded pitch, as a fraction: 0.06
    // picks somewhere between -6% and +6% every time. 0 plays it untouched
    property real pitchVariation: 0

    // TODO
    // Whether the sound decoded, and so whether play() will be heard
    readonly property bool ready: root._clipId >= 0

    property int _clipId: -1

    onSourceChanged: root._clipId = UiSoundPlayer.registerClip(root.source)

    // TODO
    // Plays on an idle voice when there is one, recycling the oldest when they
    // are all busy. A play the caller marks as auto-repeat is scaled by
    // repeatGain
    function play(autoRepeat: bool) {
        if (root.muted || root._clipId < 0) {
            return;
        }

        const gain = root.volume * (autoRepeat ? root.repeatGain : 1);
        const spread = root.pitchVariation;
        const pitch = spread > 0 ? 1 + (Math.random() * 2 - 1) * spread : 1;

        UiSoundPlayer.play(root._clipId, gain, root.voices, pitch);
    }

    // TODO
    // Fades out every voice this sound is currently playing
    function stop() {
        if (root._clipId < 0) {
            return;
        }

        UiSoundPlayer.stop(root._clipId);
    }
}

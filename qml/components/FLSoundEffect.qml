import QtQuick
import QtMultimedia

Item {
    id: root

    property var defaultVolume: 0.5

    property alias source: player.source
    property alias audioOutput: player.audioOutput
    property alias playbackState: player.playbackState

    function play() { volumeFadeOut.start() }
    function pause() { player.pause() }
    function stop() { player.stop() }

    MediaPlayer {
        id: player
        audioOutput: AudioOutput {
            id: theAudioOutput
            volume: root.defaultVolume
        }
    }

    PropertyAnimation {
        id: volumeFadeOut
        target: player.audioOutput
        property: "volume"
        duration: 50
        to: 0.0

        onStopped: {
            player.stop();
            player.audioOutput.volume = root.defaultVolume;  // Reset volume
            player.playbackRate = (Math.random() * 0.5) + 1;
            player.play();
        }  // Stop sound once faded out
    }
}

import QtQuick
import QtQuick.Controls
import QtMultimedia
import Firelight 1.0

Popup {
    id: viewer
    modal: true
    dim: true
    parent: Overlay.overlay
    anchors.centerIn: Overlay.overlay
    width: (parent ? parent.width : 800) * 0.9
    height: (parent ? parent.height : 600) * 0.9
    padding: 0

    property string mediaUrl: ""
    property string mediaKind: "" // "screenshot" | "clip"

    // Opens the viewer for a capture row (from the model).
    function openWith(item) {
        mediaUrl = item.fileUrl;
        mediaKind = item.captureType;
        open();
    }

    onClosed: {
        player.stop();
        mediaUrl = "";
        mediaKind = "";
    }

    background: Rectangle {
        color: "#000000"
        radius: 6
    }

    contentItem: Item {
        Image {
            anchors.fill: parent
            anchors.margins: 8
            visible: viewer.mediaKind === "screenshot"
            source: viewer.mediaKind === "screenshot" ? viewer.mediaUrl : ""
            fillMode: Image.PreserveAspectFit
            asynchronous: true
        }

        VideoOutput {
            id: videoOut
            anchors.fill: parent
            anchors.margins: 8
            visible: viewer.mediaKind === "clip"
        }

        MediaPlayer {
            id: player
            source: viewer.mediaKind === "clip" ? viewer.mediaUrl : ""
            videoOutput: videoOut
            audioOutput: AudioOutput {}
            loops: MediaPlayer.Infinite
            onSourceChanged: if (source !== "") play()
        }

        Button {
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.margins: 8
            text: "✕"
            onClicked: viewer.close()
        }

        Button {
            visible: viewer.mediaKind === "clip"
            anchors.bottom: parent.bottom
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.margins: 12
            text: player.playbackState === MediaPlayer.PlayingState ? "Pause"
                                                                    : "Play"
            onClicked: player.playbackState === MediaPlayer.PlayingState
                       ? player.pause() : player.play()
        }
    }
}

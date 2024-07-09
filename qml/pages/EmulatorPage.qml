import QtQuick
import QtQuick.Controls
// import QtQuick.Controls.Material
import QtQuick.Dialogs
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Effects
import Firelight 1.0

Rectangle {
    id: emulatorContainer
    // property bool blurred: false
    // property var blurAmount: 0.0
    color: "black"

    property alias currentGameName: emulatorView.currentGameName
    property alias running: emulatorView.running

    function loadGame(entryId) {
        emulatorView.loadLibraryEntry(entryId)
    }

    function setPictureMode(pictureMode) {
        emulatorView.pictureMode = pictureMode
    }

    function resetGame() {
        emulatorView.resetEmulation()
    }

    function pauseGame() {
        emulatorView.pauseGame()
    }

    function resumeGame() {
        emulatorView.resumeGame()
    }

    function isRunning() {
        return emulatorView.isRunning()
    }

    EmulatorView {
        id: emulatorView

        property string pictureMode: "aspect-ratio"
        property bool isFullScreen: false
        anchors.centerIn: parent
        smooth: false

        // onOrphanPatchDetected: {
        //     console.log("orphan patch detected")
        //     everything.pop()
        // }

        // Component.onCompleted: {
        //     this.load(currentLibraryEntryId, romData, saveData, corePath)
        // }

        // onReadyToStart: function () {
        //     emulatorContainer.readyToStart()
        // }

        states: [
            State {
                name: "FullScreenState"
                when: emulatorView.pictureMode === "aspect-ratio"
                PropertyChanges {
                    target: emulatorView
                    width: emulatorView.nativeAspectRatio > 0 ? parent.height * emulatorView.nativeAspectRatio : parent.width
                    height: parent.height
                }
            },
            State {
                name: "CenterInState"
                when: emulatorView.pictureMode === "original"
                PropertyChanges {
                    target: emulatorView
                    width: emulatorView.nativeWidth > 0 ? emulatorView.nativeWidth : 640
                    height: emulatorView.nativeHeight > 0 ? emulatorView.nativeHeight : 480
                }
            },
            State {
                name: "StretchedState"
                when: emulatorView.pictureMode === "stretched"
                PropertyChanges {
                    target: emulatorView
                    width: parent.width
                    height: parent.height
                }
            }
        ]

        transitions: [
            Transition {
                from: "*"
                to: "*"
                ParallelAnimation {
                    NumberAnimation {
                        properties: "width"
                        duration: 100
                        easing.type: Easing.InOutQuad
                    }
                    NumberAnimation {
                        properties: "height"
                        duration: 100
                        easing.type: Easing.InOutQuad
                    }
                }
            }
        ]
    }
}
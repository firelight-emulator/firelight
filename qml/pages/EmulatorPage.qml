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
    property bool paused: false

    onPausedChanged: function () {
        if (paused) {
            pauseGame()
        } else {
            resumeGame()
        }
    }

    signal emulationStarted()

    signal rewindPointsReady(var points)

    signal rewindPointLoaded()

    function setPlaySpeedMultiplier(multiplier) {
        emulatorView.playSpeedMultiplier = multiplier
    }

    function loadRewindPoint(index) {
        emulatorView.loadRewindPoint(index)
    }

    function createRewindPoints() {
        emulatorView.createRewindPoints()
    }

    function createSuspendPoint() {
        emulatorView.createSuspendPoint()
    }

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

    function writeSuspendPoint(index) {
        emulatorView.writeSuspendPoint(index)
    }

    function loadSuspendPoint(index) {
        emulatorView.loadSuspendPoint(index)
    }

    EmulatorView {
        id: emulatorView

        property string pictureMode: "aspect-ratio"
        property bool isFullScreen: false
        anchors.centerIn: parent
        smooth: false

        onEmulationStarted: {
            emulatorContainer.emulationStarted()
        }

        onRewindPointsReady: function (points) {
            emulatorContainer.rewindPointsReady(points)
        }

        onRewindPointLoaded: function () {
            emulatorContainer.rewindPointLoaded()
        }

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
                    width: emulatorView.nativeWidth > 0 ? emulatorView.nativeWidth * 2 : 640
                    height: emulatorView.nativeHeight > 0 ? emulatorView.nativeHeight * 2 : 480
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
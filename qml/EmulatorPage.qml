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

    signal gameLoaded()

    function loadTheThing(currentLibraryEntryId, romData, saveData, corePath) {
        emulatorView.loadGame(currentLibraryEntryId, romData, saveData, corePath)
        gameLoaded()
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

    function startEmulation() {
        emulatorView.startEmulation()
    }

    function stopEmulation() {
        emulatorView.stopEmulation()
    }

    // layer.enabled: true
    // layer.effect: MultiEffect {
    //     source: emulatorContainer
    //     anchors.fill: emulatorContainer
    //     blurEnabled: emulatorContainer.blurred
    //     blurMultiplier: 1.0
    //     blurMax: 64
    //     blur: emulatorContainer.blurAmount
    // }


    EmulatorView {
        id: emulatorView

        property bool isFullScreen: false
        anchors.centerIn: parent

        // onOrphanPatchDetected: {
        //     console.log("orphan patch detected")
        //     everything.pop()
        // }

        // Component.onCompleted: {
        //     this.load(currentLibraryEntryId, romData, saveData, corePath)
        // }

        states: [
            State {
                name: "FullScreenState"
                when: emulatorView.isFullScreen
                PropertyChanges {
                    target: emulatorView
                    width: parent.height * 1.333
                    height: parent.height
                }
            },
            State {
                name: "CenterInState"
                when: !emulatorView.isFullScreen
                PropertyChanges {
                    target: emulatorView
                    width: 640
                    height: 480
                }
            }
        ]

        transitions: [
            Transition {
                from: "FullScreenState"
                to: "CenterInState"
                NumberAnimation {
                    properties: "width, height"
                    duration: 100
                    easing.type: Easing.InOutQuad
                }
            },
            Transition {
                from: "CenterInState"
                to: "FullScreenState"
                NumberAnimation {
                    properties: "width, height"
                    duration: 100
                    easing.type: Easing.InOutQuad
                }
            }
        ]
    }


    Button {
        text: "Back"
        onClicked: function () {
            emulatorView.isFullScreen = !emulatorView.isFullScreen;
            console.log("back clicked")
        }
    }
}
// color: "black"
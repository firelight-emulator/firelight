import QtQuick
import QtQuick.Controls
// import QtQuick.Controls.Material
import QtQuick.Dialogs
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Effects
import Firelight 1.0

Rectangle {
    property int currentLibraryEntryId
    property var romData
    property var saveData
    property string corePath
    color: "black"

    focus: true

    // Keys.onEscapePressed: {
    //     // background.visible = false
    //     everything.push(mainMenu)
    // }

    StackView.visible: true

    StackView.onActivated: function () {
        emulatorContainer.state = "NotBlurred";
    }

    StackView.onDeactivating: function () {
        emulatorContainer.state = "Blurred";
    }

    Keys.onDigit1Pressed: {
        emulatorView.loadGame(currentLibraryEntryId, romData, saveData, corePath)
    }

    Keys.onDigit2Pressed: {
        emulatorView.startEmulation()
    }

    // Keys.onEscapePressed: {
    //     emulatorView.pause()
    //     // everything.push(quickMenu)
    // }


    Item {
        id: emulatorContainer
        anchors.fill: parent

        // color: "black"

        property bool blurred: false
        property var blurAmount: 0.0

        layer.enabled: true
        layer.effect: MultiEffect {
            source: emulatorContainer
            anchors.fill: emulatorContainer
            blurEnabled: emulatorContainer.blurred
            blurMultiplier: 3.0
            blurMax: 64
            blur: emulatorContainer.blurAmount
        }

        states: [
            State {
                name: "Blurred"
                when: emulatorContainer.blurred
                PropertyChanges {
                    target: emulatorContainer
                    blurAmount: 1.0
                }
            },
            State {
                name: "NotBlurred"
                when: !emulatorContainer.blurred
                PropertyChanges {
                    target: emulatorContainer
                    blurAmount: 0.0
                }
            }
        ]

        transitions: [
            Transition {
                from: "NotBlurred"
                to: "Blurred"
                SequentialAnimation {
                    ScriptAction {
                        script: {
                            emulatorView.pauseGame()
                            emulatorContainer.blurred = true;
                        }
                    }
                    NumberAnimation {
                        properties: "blurAmount"
                        duration: 200
                        easing.type: Easing.InOutQuad
                    }
                }
            },
            Transition {
                // from: "Blurred"
                to: "NotBlurred"
                SequentialAnimation {
                    NumberAnimation {
                        properties: "blurAmount"
                        duration: 150
                        easing.type: Easing.InOutQuad
                    }
                    PauseAnimation {
                        duration: 100
                    }
                    ScriptAction {
                        script: {
                            emulatorView.resumeGame()
                        }
                    }
                    onStopped: {
                        emulatorContainer.blurred = false;
                    }
                }
            }
        ]


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
    }


    Button {
        text: "Back"
        onClicked: function () {
            emulatorView.isFullScreen = !emulatorView.isFullScreen;
            console.log("back clicked")
        }
    }
}
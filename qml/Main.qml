import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Window
import QtQuick.Layouts 1.0
import QtQuick.Effects
import Firelight 1.0
import FirelightStyle 1.0

ApplicationWindow {
    id: window

    width: 1280
    height: 720

    minimumHeight: 720
    minimumWidth: 1280

    visible: true

    title: qsTr("Firelight")
    // background: Image {
    //     source: "file:orange.jpg"
    //     width: 2560
    //     height: 1440
    //     fillMode: Image.Stretch
    // }

    background: Rectangle {
        color: "#e4e4e4"
    }

    // Rectangle {
    //     anchors.fill: parent
    //     parent: window.activeFocusItem
    //     border.color: "blue"
    //     color: "transparent"
    // }

    MainMenu {
        id: mainMenu
        visible: false

        onGameStartRequested: function (entryId) {
            gameLoader.loadGame(entryId)
        }
    }

    SequentialAnimation {
        id: overlayFadeIn
        PropertyAction {
            target: overlay
            property: "opacity"
            value: 0
        }
        PropertyAction {
            target: overlay
            property: "scale"
            value: 1
        }

        PropertyAction {
            target: overlay
            property: "visible"
            value: true
        }

        PropertyAnimation {
            target: overlay
            property: "opacity"
            from: 0
            to: 1
            duration: 350
            easing.type: Easing.InQuad
        }

        ScriptAction {
            script: {
                stackView.replace(emulator, {}, StackView.Immediate)
                // emulator.resumeGame()
            }
        }

        PropertyAnimation {
            target: overlay
            property: "opacity"
            from: 1
            to: 0
            duration: 200
            easing.type: Easing.InQuad
        }

        ScriptAction {
            script: {
                emulator.startEmulation()
                // emulator.resumeGame()
            }
        }
    }

    GameLoader {
        id: gameLoader

        onGameLoaded: function (entryId, romData, saveData, corePath) {
            console.log("loaded game!")
            emulator.loadTheThing(entryId, romData, saveData, corePath)
            overlayFadeIn.start()
        }

        onGameLoadFailedOrphanedPatch: function (entryId) {
            patchClickedDialog.open()
        }
    }

    EmulatorPage {
        id: emulator
        visible: false

        StackView.visible: true

        StackView.onActivated: {
            layer.enabled = false
            emulator.resumeGame()
        }

        StackView.onActivating: {
            // emulatorDimmer.opacity = 0
        }

        StackView.onDeactivating: {
            layer.enabled = true
            emulator.pauseGame()
            // emulatorDimmer.opacity = 0.4
        }
        // smooth: false

        property double blurAmount: 0

        layer.enabled: false
        layer.effect: MultiEffect {
            source: emulator
            anchors.fill: emulator
            blurEnabled: true
            blurMultiplier: 1.0
            blurMax: 64
            blur: emulator.blurAmount
        }

        // Keys.onEscapePressed: {
        //     appRoot.state = "gameSuspended"
        // }
        //
        // onGameLoaded: {
        //     appRoot.state = "playingGame"
        // }

        Rectangle {
            id: emulatorDimmer
            anchors.fill: parent
            color: "black"
            opacity: 0

            Behavior on opacity {
                NumberAnimation {
                    duration: 250
                    easing.type: Easing.InOutQuad
                }
            }
        }

        Connections {
            target: window_resize_handler

            function onWindowResizeStarted() {
                if (stackView.currentItem === emulator) {
                    emulator.pauseGame()
                }
            }

            function onWindowResizeFinished() {
                if (stackView.currentItem === emulator) {
                    emulator.resumeGame()
                }
            }
        }
    }

    StackView {
        id: stackView
        anchors.fill: parent
        initialItem: mainMenu

        Keys.onEscapePressed: {
            if (stackView.currentItem === mainMenu) {
                stackView.pop()
            } else {
                stackView.push(mainMenu)
            }
        }

        pushEnter: Transition {
            ParallelAnimation {
                PropertyAnimation {
                    property: "opacity"
                    from: 0
                    to: 1
                    duration: 250
                    easing.type: Easing.InOutQuad
                }
                PropertyAnimation {
                    property: "scale"
                    from: 1.05
                    to: 1
                    duration: 250
                    easing.type: Easing.InOutQuad
                }
            }
        }
        pushExit: Transition {
            ParallelAnimation {
                PropertyAnimation {
                    property: "blurAmount"
                    from: 0
                    to: 1
                    duration: 250
                    easing.type: Easing.InOutQuad
                }
                PropertyAnimation {
                    property: "scale"
                    from: 1
                    to: 0.92
                    duration: 250
                    easing.type: Easing.OutQuad
                }
            }
        }
        popEnter: Transition {
            ParallelAnimation {
                PropertyAnimation {
                    property: "blurAmount"
                    from: 1
                    to: 0
                    duration: 250
                    easing.type: Easing.InOutQuad
                }
                PropertyAnimation {
                    property: "scale"
                    from: 0.92
                    to: 1
                    duration: 250
                    easing.type: Easing.InQuad
                }
            }
        }
        popExit: Transition {
            ParallelAnimation {
                PropertyAnimation {
                    property: "opacity"
                    from: 1
                    to: 0
                    duration: 250
                    easing.type: Easing.InOutQuad
                }
                PropertyAnimation {
                    property: "scale"
                    from: 1
                    to: 1.05
                    duration: 250
                    easing.type: Easing.InOutQuad
                }
            }
        }
    }

    Rectangle {
        id: overlay
        anchors.fill: parent
        color: "black"
        visible: false
    }
}

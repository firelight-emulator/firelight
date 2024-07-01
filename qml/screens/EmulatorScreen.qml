import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Window
import QtQuick.Layouts 1.0
import QtQuick.Effects
import Firelight 1.0

FocusScope {
    id: root

    property bool emulatorIsRunning: emulator.running

    signal gameReady()

    function loadGame(id) {
        emulator.loadGame(id)
    }

    function startEmulator() {
        emulator.startEmulation()
    }

    function stopEmulator() {
        emulatorStack.pop()
        emulator.stopEmulation()
    }

    StackView {
        id: emulatorStack
        anchors.fill: parent

        initialItem: emulator

        Keys.onEscapePressed: function (event) {
            if (event.isAutoRepeat) {
                return
            }

            if (emulatorStack.currentItem === emulator) {
                // emulatorStack.pop()
                emulatorStack.push(nowPlayingPage)
            } else {
                emulatorStack.pop()
                // emulatorStack.push(homeScreen)
            }
        }

        property bool suspended: false
        property bool running: false

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
                    property: "x"
                    from: -20
                    to: 0
                    duration: 250
                    easing.type: Easing.InOutQuad
                }
            }
        }
        pushExit: Transition {

        }
        popEnter: Transition {
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
                    property: "x"
                    from: 0
                    to: -20
                    duration: 250
                    easing.type: Easing.InOutQuad
                }
            }
        }
        replaceEnter: Transition {
        }
        replaceExit: Transition {
        }
    }

    EmulatorPage {
        id: emulator
        visible: false

        onReadyToStart: function () {
            root.gameReady()
        }

        ChallengeIndicatorList {
            id: challengeIndicators
            visible: achievement_manager.challengeIndicatorsEnabled

            anchors.top: parent.top
            anchors.right: parent.right
            anchors.topMargin: 16
            anchors.rightMargin: 16
            height: 100
            width: 300
        }

        states: [
            State {
                name: "stopped"
            },
            State {
                name: "suspended"
                PropertyChanges {
                    target: emulatorDimmer
                    opacity: 0.4
                }
                PropertyChanges {
                    emulator {
                        layer.enabled: true
                        blurAmount: 1
                    }
                }
            },
            State {
                name: "running"
                PropertyChanges {
                    target: emulatorDimmer
                    opacity: 0
                }
                PropertyChanges {
                    emulator {
                        layer.enabled: false
                        blurAmount: 0
                    }
                }
            }
        ]

        transitions: [
            Transition {
                from: "*"
                to: "suspended"
                SequentialAnimation {
                    ScriptAction {
                        script: {
                            emulator.pauseGame()
                        }
                    }
                    PropertyAction {
                        target: emulator
                        property: "layer.enabled"
                        value: true
                    }
                    ParallelAnimation {
                        NumberAnimation {
                            properties: "blurAmount"
                            duration: 250
                            easing.type: Easing.InOutQuad
                        }
                        NumberAnimation {
                            target: emulatorDimmer
                            properties: "opacity"
                            duration: 250
                            easing.type: Easing.InOutQuad
                        }
                    }
                }
            },
            Transition {
                from: "*"
                to: "running"
                SequentialAnimation {
                    ParallelAnimation {
                        NumberAnimation {
                            properties: "blurAmount"
                            duration: 250
                            easing.type: Easing.InOutQuad
                        }
                        NumberAnimation {
                            target: emulatorDimmer
                            properties: "opacity"
                            duration: 250
                            easing.type: Easing.InOutQuad
                        }
                    }
                    PropertyAction {
                        target: emulator
                        property: "layer.enabled"
                        value: false
                    }

                    ScriptAction {
                        script: {
                            emulator.resumeGame()
                        }

                    }
                }
            }
        ]

        StackView.visible: true

        StackView.onActivating: {
            state = "running"
        }

        StackView.onDeactivating: {
            state = "suspended"
        }

        property double blurAmount: 0

        Behavior on blurAmount {
            NumberAnimation {
                duration: 250
                easing.type: Easing.InOutQuad
            }
        }

        layer.enabled: false
        layer.effect: MultiEffect {
            source: emulator
            anchors.fill: emulator
            blurEnabled: true
            blurMultiplier: 1.0
            blurMax: 64
            blur: emulator.blurAmount
        }

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
                if (root.StackView.view.currentItem === emulator) {
                    emulator.pauseGame()
                }
            }

            function onWindowResizeFinished() {
                if (root.StackView.view.currentItem === emulator) {
                    emulator.resumeGame()
                }
            }
        }
    }


    Component {
        id: nowPlayingPage
        NowPlayingPage {
            id: me
            property bool topLevel: true
            property string topLevelName: "nowPlaying"

            onBackToMainMenuPressed: function () {
                stackView.push(homeScreen)
            }

            onResumeGamePressed: function () {
                emulatorStack.pop()
            }

            onRestartGamePressed: function () {
                emulator.resetGame()
                emulatorStack.pop()
            }

            onCloseGamePressed: function () {
                closeGameAnimation.start()
            }
        }
    }

    AchievementProgressIndicator {
        id: achievementProgressIndicator

        Connections {
            target: achievement_manager

            function onAchievementProgressUpdated(imageUrl, id, name, description, current, desired) {
                if (achievement_manager.progressNotificationsEnabled) {
                    achievementProgressIndicator.openWith(imageUrl, name, description, current, desired)
                }
            }
        }
    }

    AchievementUnlockIndicator {
        id: achievementUnlockIndicator

        Connections {
            target: achievement_manager

            function onAchievementUnlocked(imageUrl, name, description) {
                if (achievement_manager.unlockNotificationsEnabled) {
                    achievementUnlockIndicator.openWith(imageUrl, name, description)
                }
            }
        }
    }
}
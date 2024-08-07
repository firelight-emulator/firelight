import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Window
import QtQuick.Layouts 1.0
import QtQuick.Effects
import Firelight 1.0

FocusScope {
    id: root

    property bool emulatorIsRunning: emulatorStack.depth > 0

    signal gameReady()

    signal gameAboutToStop()

    signal gameStopped()

    function loadGame(id) {
        emulatorStack.pushItem(emulatorComponent, {entryId: id}, StackView.Immediate)
        // emulator.loadGame(id)
    }

    Rectangle {
        id: background
        color: "black"
        anchors.fill: parent
        visible: root.emulatorIsRunning
    }

    StackView {
        id: emulatorStack
        anchors.fill: parent
        focus: true

        property bool suspended: false
        property bool running: false

        // Keys.onEscapePressed: function (event) {
        //     if (event.isAutoRepeat) {
        //         return
        //     }
        //
        //     if (emulatorStack.depth === 1) {
        //         emulatorStack.pushItem(nowPlayingPage, {}, StackView.PushTransition)
        //     } else if (emulatorStack.depth === 2) {
        //         emulatorStack.pop()
        //     }
        // }

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

    Component {
        id: emulatorComponent

        EmulatorPage {
            id: emulator

            property bool loaded: false

            required property int entryId

            Component.onCompleted: {
                emulator.loadGame(entryId)
            }

            onEmulationStarted: function () {
                emulator.loaded = true
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

            Keys.onEscapePressed: function (event) {
                if (event.isAutoRepeat || !emulator.loaded) {
                    return
                }

                if (emulator.StackView.status === StackView.Active) {
                    emulatorStack.pushItem(nowPlayingPage, {}, StackView.PushTransition)
                }
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
                    if (emulator.StackView.status === StackView.Active) {
                        emulator.pauseGame()
                    }
                }

                function onWindowResizeFinished() {
                    if (emulator.StackView.status === StackView.Active) {
                        emulator.resumeGame()
                    }
                }
            }
        }
    }

    SequentialAnimation {
        id: closeGameAnimation

        ScriptAction {
            script: {
                root.gameAboutToStop()
            }
        }

        PauseAnimation {
            duration: 500
        }

        ScriptAction {
            script: {
                emulatorStack.clear()
            }
        }

        ScriptAction {
            script: {
                root.gameStopped()
            }
        }


    }


    Component {
        id: nowPlayingPage
        NowPlayingPage {
            id: me
            property bool topLevel: true
            property string topLevelName: "nowPlaying"

            Keys.onEscapePressed: function (event) {
                if (event.isAutoRepeat) {
                    return
                }

                if (me.StackView.status === StackView.Active) {
                    // emulatorStack.pop()
                    me.StackView.view.popCurrentItem()
                }
            }

            onBackToMainMenuPressed: function () {
                // root.StackView.view.popCurrentItem()
                // stackView.push(homeScreen)
            }

            onResumeGamePressed: function () {
                emulatorStack.pop()
            }

            onRestartGamePressed: function () {
                const emu = emulatorStack.get(0)
                emu.resetGame()
                // emulator.resetGame()
                emulatorStack.pop()
            }

            onCloseGamePressed: function () {
                closeGameAnimation.running = true
                // emulatorStack.popCurrentItem()
                // closeGameAnimation.start()
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
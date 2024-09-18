import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Window
import QtQuick.Layouts 1.0
import QtMultimedia
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
            ParallelAnimation {
                NumberAnimation {
                    properties: "blurAmount"
                    to: 0.5
                    duration: 250
                    easing.type: Easing.InOutQuad
                }
                NumberAnimation {
                    properties: "dimmerOpacity"
                    to: 0.5
                    duration: 250
                    easing.type: Easing.InOutQuad
                }
            }
        }
        popEnter: Transition {
            ParallelAnimation {
                NumberAnimation {
                    properties: "blurAmount"
                    to: 0
                    duration: 250
                    easing.type: Easing.InOutQuad
                }
                NumberAnimation {
                    properties: "dimmerOpacity"
                    to: 0
                    duration: 250
                    easing.type: Easing.InOutQuad
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

        FocusScope {
            id: scope
            required property int entryId

            property real blurAmount: 0
            property real dimmerOpacity: 0

            StackView.visible: true

            StackView.onDeactivating: function () {
                emulator.paused = true
            }
            StackView.onActivated: function () {
                emulator.paused = false
            }

            SoundEffect {
                id: scroll
                source: "file:system/sfx/click.wav"
            }
            SoundEffect {
                id: open
                source: "file:system/sfx/openrewind.wav"
            }

            function createRewindPoints() {
                emulator.createRewindPoints()
            }

            Keys.onSpacePressed: function () {
                emulator.paused = true
                emulator.createRewindPoints()
                // openRewindMenuAnimation.start()
            }

            Keys.onDigit1Pressed: function () {
                emulator.resumeGame()
            }


            Keys.onDigit9Pressed: function (event) {
                emulator.setPlaySpeedMultiplier(2)
            }


            Keys.onDigit8Pressed: function (event) {
                emulator.setPlaySpeedMultiplier(1)
            }

            Keys.onDigit6Pressed: function (event) {
                emulator.writeSuspendPoint(1)
            }

            function loadRewindPoint(index) {
                emulator.loadRewindPoint(index)
            }

            function writeSuspendPoint(index) {
                emulator.writeSuspendPoint(index)
            }

            function loadSuspendPoint(index) {
                emulator.loadSuspendPoint(index)
            }

            function loadLastSuspendPoint() {
                emulator.loadLastSuspendPoint()
            }

            Connections {
                target: emulator

                function onRewindPointsReady(points) {
                    // console.log("Points ready: " + JSON.stringify(points))
                    if (scope.StackView.status === StackView.Active) {
                        emulatorStack.pushItem(rewindPage, {
                            model: points,
                            scrollSfx: scroll,
                            openSfx: open
                        }, StackView.Immediate)
                    } else {
                        emulatorStack.replaceCurrentItem(rewindPage, {
                            model: points,
                            scrollSfx: scroll,
                            openSfx: open
                        }, StackView.Immediate)
                    }
                }
            }


            Keys.onEscapePressed: function (event) {
                emulatorStack.pushItem(nowPlayingPage, {
                    entryId: scope.entryId,
                    contentHash: ""
                }, StackView.PushTransition)
                event.accepted = true
            }

            Keys.onPressed: function (event) {
                if (event.key === Qt.Key_Home) {
                    emulatorStack.pushItem(nowPlayingPage, {
                        entryId: scope.entryId,
                        contentHash: ""
                    }, StackView.PushTransition)
                    event.accepted = true
                }
            }

            // SequentialAnimation {
            //     id: openRewindMenuAnimation
            //     ScriptAction {
            //         script: {
            //             emulator.createRewindPoints()
            //         }
            //     }
            //     PauseAnimation {
            //         duration: 100
            //     }
            //     ScriptAction {
            //         script: {
            //             emulatorStack.pushItem(rewindPage, {}, StackView.Immediate)
            //         }
            //     }
            // }

            EmulatorPage {
                id: emulator
                width: parent.width
                height: parent.height
                anchors.horizontalCenter: parent.horizontalCenter

                property bool loaded: false
                // property bool blurred: false
                // property bool blurred: emulatorStack.currentItem === nowPlayingPage

                property int entryId: scope.entryId

                Component.onCompleted: {
                    emulator.loadGame(entryId)
                }

                onEmulationStarted: function () {
                    emulator.loaded = true
                }

                onRewindPointLoaded: function () {
                    console.log("Rewind point loaded; closing")
                    const rewind = emulatorStack.get(1)
                    rewind.close()
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

                layer.enabled: true
                layer.effect: MultiEffect {
                    source: emulator
                    anchors.fill: emulator
                    blurEnabled: true
                    blurMultiplier: 1.0
                    blurMax: 64
                    blur: scope.blurAmount
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

                Rectangle {
                    id: emulatorDimmer
                    anchors.fill: parent
                    color: "black"
                    opacity: scope.dimmerOpacity

                    // Behavior on opacity {
                    //     NumberAnimation {
                    //         duration: 250
                    //         easing.type: Easing.InOutQuad
                    //     }
                    // }
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
        id: rewindPage
        RewindMenu {
            id: theActualMenu
            onRewindPointSelected: function (index) {
                const emu = emulatorStack.get(0)
                emu.loadRewindPoint(index)
                // emulator.resetGame()

            }
        }
    }


    Component {
        id: nowPlayingPage
        NowPlayingPage {
            id: me
            property bool topLevel: true
            property string topLevelName: "nowPlaying"

            StackView.onActivating: function () {
                // sfx_player.play()
            }

            Keys.onEscapePressed: function (event) {
                if (event.isAutoRepeat) {
                    return
                }

                if (me.StackView.status === StackView.Active) {
                    // emulatorStack.pop()
                    me.StackView.view.popCurrentItem()
                    event.accepted = true
                }
            }

            Keys.onBackPressed: function (event) {
                if (event.isAutoRepeat) {
                    return
                }

                if (me.StackView.status === StackView.Active) {
                    // emulatorStack.pop()
                    me.StackView.view.popCurrentItem()
                    event.accepted = true
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

            onRewindPressed: function () {
                const emu = emulatorStack.get(0)
                // emulator.resetGame()
                // emu.paused = true
                emu.createRewindPoints()
                // emulatorStack.replaceCurrentItem(rewindPage, {})
            }

            onCloseGamePressed: function () {
                closeGameAnimation.running = true
                // emulatorStack.popCurrentItem()
                // closeGameAnimation.start()
            }

            onWriteSuspendPoint: function (index) {
                const emu = emulatorStack.get(0)
                emu.writeSuspendPoint(index)
            }

            onLoadSuspendPoint: function (index) {
                const emu = emulatorStack.get(0)
                emu.loadSuspendPoint(index)
            }

            onLoadLastSuspendPoint: function () {
                const emu = emulatorStack.get(0)
                emu.loadLastSuspendPoint()
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

    GameLaunchPopup {
        objectName: "Game Launch Popup"
        id: gameLaunchPopup

        Connections {
            target: achievement_manager

            function onGameLoadSucceeded(imageUrl, title, numEarned, numTotal) {
                gameLaunchPopup.openWith(imageUrl, title, numEarned, numTotal, achievement_manager.defaultToHardcore)
            }
        }
    }
}
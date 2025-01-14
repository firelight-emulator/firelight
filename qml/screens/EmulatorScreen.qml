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

    Action {
        id: resetGameAction
        onTriggered: {
            if (emulatorStack.depth > 1) {
                emulatorStack.get(0).resetGame()
                emulatorStack.popCurrentItem()
            }
        }
    }

    function loadGame(id, hash) {
        emulatorStack.pushItem(emulatorComponent, {entryId: id, contentHash: hash}, StackView.Immediate)
        // emulator.loadGame(id)
    }

    function startGame(gameData, saveData, corePath, contentHash, saveSlotNumber, platformId, contentPath) {
        emulatorStack.pushItem(emulatorComponent, {
            gameData: gameData,
            saveData: saveData,
            corePath: corePath,
            contentHash: contentHash,
            saveSlotNumber: saveSlotNumber,
            platformId: platformId,
            contentPath: contentPath
        }, StackView.Immediate)
    }

    Rectangle {
        id: background
        color: "black"
        anchors.fill: parent
        visible: true
    }

    StackView {
        id: emulatorStack
        anchors.fill: parent
        focus: true

        background: Rectangle {
            color: "black"
            anchors.fill: parent
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

        NewEmulatorPage {
            id: emuPage

            property real dimmerOpacity: 0

            Keys.onEscapePressed: function (event) {
                emulatorStack.pushItem(nowPlayingPage, {
                    entryId: 0,
                    undoEnabled: false,
                    contentHash: emuPage.contentHash
                }, StackView.PushTransition)

                event.accepted = true
            }

            Keys.onPressed: function (event) {
                if (event.key === Qt.Key_Home) {
                    emulatorStack.pushItem(nowPlayingPage, {
                        entryId: 0,
                        undoEnabled: false,
                        contentHash: emuPage.contentHash
                    }, StackView.PushTransition)
                    event.accepted = true
                }
            }

            Rectangle {
                id: emulatorDimmer
                width: root.width
                height: root.height
                color: "black"
                opacity: emuPage.dimmerOpacity
            }

            onRewindPointsReady: function (points) {
                // console.log("Points ready: " + JSON.stringify(points))
                if (emuPage.StackView.status === StackView.Active) {
                    emulatorStack.pushItem(rewindPage, {
                        model: points
                    }, StackView.Immediate)
                } else {
                    emulatorStack.replaceCurrentItem(rewindPage, {
                        model: points
                    }, StackView.Immediate)
                }
            }


            Connections {
                target: window_resize_handler

                function onWindowResizeStarted() {
                    if (emuPage.StackView.status === StackView.Active) {
                        emuPage.paused = true
                    }
                }

                function onWindowResizeFinished() {
                    if (emuPage.StackView.status === StackView.Active) {
                        emuPage.paused = false
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
                resetGameAction.trigger()
                // const emu = emulatorStack.get(0)
                // emu.resetGame()
                // // emulator.resetGame()
                // emulatorStack.pop()
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
import QtQuick
import QtQuick.Controls
// import QtQuick.Controls.Material
import QtQuick.Dialogs
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Effects
import QtMultimedia
import Firelight 1.0

FocusScope {
    id: root

    property bool blurEnabled: false

    signal closing()

    clip: true
    focus: true

    Timer {
        id: ctrlTimer
        interval: 500
        repeat: false

        onTriggered: {
            redThing.opacity = 0.7
        }
    }

    Keys.onDigit9Pressed: {
        emulator.incrementPlaybackMultiplier()
    }

    Keys.onDigit8Pressed: {
        emulator.decrementPlaybackMultiplier()
    }

    // Keys.onPressed: function(event) {
    //     console.log("pressed: " + event.key)
    //     if (event.key === Qt.Key_Control) {
    //         ctrlTimer.start()
    //     }
    // }
    //
    // Keys.onReleased: function(event) {
    //     console.log("released: " + event.key)
    //     if (event.key === Qt.Key_Control) {
    //         if (redThing.visible) {
    //             redThing.opacity = 0
    //         }
    //
    //         ctrlTimer.stop()
    //     }
    // }

    property alias videoAspectRatio: emulator.videoAspectRatio
    property alias contentHash: emulator.contentHash
    property alias gameName: emulator.gameName
    property alias entryId: emulator.entryId
    property alias canUndoLoadSuspendPoint: emulator.canUndoLoadSuspendPoint

    function loadGame(entryId) {
        emulator.loadGame(entryId)
    }

    function startGame() {
        if (!emulator.started) {
            emulator.startGame()
        }
    }

    signal rewindPointsReady(var points)

    signal gameReady()

    function resetGame() {
        emulator.resetGame()
    }

    function createRewindPoints() {
        emulator.createRewindPoints()
    }

    function loadRewindPoint(index) {
        emulator.loadRewindPoint(index)
    }

    function loadSuspendPoint(index) {
        emulator.loadSuspendPoint(index)
    }

    function writeSuspendPoint(index) {
        emulator.writeSuspendPoint(index)
    }

    function undoLoadSuspendPoint() {
        emulator.undoLastLoadSuspendPoint()
    }

    property alias paused: emulator.paused

    // StackView.visible: true

    // StackView.onDeactivating: function () {
    //     emulator.paused = true
    // }
    // StackView.onActivated: function () {
    //     emulator.paused = false
    //     debugWindow2.visible = true
    // }

    Rectangle {
        id: background
        color: "black"
        anchors.fill: parent
    }

    EmulatorItem {
        id: emulator
        focus: true
        anchors.centerIn: parent
        width: parent.height * videoAspectRatio
        height: parent.height
        smooth: false

        onVideoAspectRatioChanged: {
            console.log("new aspect ratio: " + videoAspectRatio)
        }

        // onGameStarted: {
        //     emulator.paused = false
        // }

        onRewindPointsReady: function (points) {
            root.rewindPointsReady(points)
        }

        // Keys.onDigit1Pressed: {
        //     emulator.loadSuspendPoint(1)
        // }
    }

    Rectangle {
        id: redThing
        color: "red"
        opacity: 0
        width: 600
        height: 400
        anchors.centerIn: parent

        Behavior on opacity {
            NumberAnimation {
                duration: 200
                easing.type: Easing.InOutQuad
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

    ChallengeIndicatorList {
        id: challengeIndicators
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.topMargin: 12
        anchors.rightMargin: 12
    }

    // FrameAnimation {
    //     id: frameAnimation
    //     running: true
    //     onTriggered: {
    //         bufferGraph.addValue(emulator.audioBufferLevel)
    //         frametimeGraph.addValue(frameAnimation.frameTime)
    //     }
    //
    // }
    // Pane {
    //     id: frametimeGraph
    //
    //     property var bufferValues: []
    //
    //     function addValue(value) {
    //         bufferValues.push(value)
    //         if (bufferValues.length > 100) {
    //             bufferValues.shift()
    //         }
    //         frametimeGraphCanvas.requestPaint()
    //         // bufferGraph.update()
    //         // canvas.paint()
    //     }
    //
    //     width: 250
    //     height: 120
    //     anchors.top: parent.top
    //     anchors.right: parent.right
    //
    //     padding: 12
    //     background: Rectangle {
    //         color: "black"
    //     }
    //
    //     contentItem: Canvas {
    //         id: frametimeGraphCanvas
    //         width: parent.width
    //         height: parent.height
    //         onPaint: {
    //             var ctx = getContext("2d");
    //             ctx.clearRect(0, 0, width, height);
    //             ctx.fillStyle = "white";
    //             for (var i = 0; i < frametimeGraph.bufferValues.length; i++) {
    //                 var x = (i / frametimeGraph.bufferValues.length) * width;
    //                 var y = height - ((frametimeGraph.bufferValues[i] * 1000 / 128) * height);
    //                 ctx.fillRect(x, y, width / frametimeGraph.bufferValues.length, height);
    //             }
    //             ctx.fillStyle = "green";
    //             ctx.fillRect(0, height * 7 / 8, width, 1); // Draw a middle line
    //         }
    //
    //     }
    //
    // }
    // Pane {
    //     id: bufferGraph
    //
    //     property var bufferValues: []
    //
    //     function addValue(value) {
    //         bufferValues.push(value)
    //         if (bufferValues.length > 100) {
    //             bufferValues.shift()
    //         }
    //         canvas.requestPaint()
    //         // bufferGraph.update()
    //         // canvas.paint()
    //     }
    //
    //     width: 250
    //     height: 120
    //     anchors.top: frametimeGraph.bottom
    //     anchors.right: parent.right
    //
    //     padding: 12
    //     background: Rectangle {
    //         color: "black"
    //     }
    //
    //     contentItem: Canvas {
    //         id: canvas
    //         width: parent.width
    //         height: parent.height
    //         onPaint: {
    //             var ctx = getContext("2d");
    //             ctx.clearRect(0, 0, width, height);
    //             ctx.fillStyle = "white";
    //             for (var i = 0; i < bufferGraph.bufferValues.length; i++) {
    //                 var x = (i / bufferGraph.bufferValues.length) * width;
    //                 var y = height - (bufferGraph.bufferValues[i] * height);
    //                 ctx.fillRect(x, y, width / bufferGraph.bufferValues.length, height);
    //             }
    //             ctx.fillStyle = "green";
    //             ctx.fillRect(0, height / 2, width, 1); // Draw a middle line
    //         }
    //
    //     }
    //
    // }
    //
    // Text {
    //     id: buffer
    //     text: emulator.audioBufferLevel
    //     color: "white"
    //     font.pixelSize: 14
    //     font.family: Constants.regularFontFamily
    // }
}
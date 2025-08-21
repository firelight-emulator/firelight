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

    objectName: "emulatorPage"

    Connections {
        target: InputService

        function onShortcutToggled(player, shortcut) {
            if (root.parent.StackView.status !== StackView.Active) {
                return
            }

            if (shortcut === 0) {
                root.createRewindPoints()
            } else if (shortcut === 3) {
                emulator.incrementPlaybackMultiplier()
            } else if (shortcut === 4) {
                emulator.decrementPlaybackMultiplier()
            }
        }
    }

    required property StackView stackView
    property GameSettings2 gameSettings: GameSettings2 {
        platformId: root.platformId
        contentHash: root.contentHash
    }

    property bool blurEnabled: false
    property alias audioBufferLevel: emulator.audioBufferLevel

    property alias paused: emulator.paused

    signal closing()
    signal aboutToRunFrame()

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

    Keys.onDigit7Pressed: {
        if (gameSettings.pictureMode === "stretch") {
            gameSettings.pictureMode = "aspect-ratio-fill"
        } else if (gameSettings.pictureMode === "aspect-ratio-fill") {
            gameSettings.pictureMode = "integer-scale"
        } else if (gameSettings.pictureMode === "integer-scale") {
            gameSettings.pictureMode = "stretch"
        }
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
    property alias platformId: emulator.platformId
    property alias saveSlotNumber: emulator.saveSlotNumber
    property alias canUndoLoadSuspendPoint: emulator.canUndoLoadSuspendPoint

    function startGame() {
        emulator.startGame()
    }

    signal rewindPointsReady(var points)

    signal gameReady()

    function resetGame() {
        emulator.resetGame()
    }

    function createRewindPoints() {
        if (!gameSettings.rewindEnabled) {
            return
        }
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

    Rectangle {
        id: background
        color: "black"
        anchors.fill: root
    }

    EmulatorItem {
        id: emulator
        focus: true
        anchors.centerIn: root

        muted: paused || playbackMultiplier != 1

        layer.enabled: true

        property string pictureMode: gameSettings.pictureMode
        property real aspectRatio: {
            let mode = gameSettings.aspectRatioMode

             if (mode === "core-corrected") {
                 return emulator.videoAspectRatio
             } else {
                 return emulator.trueAspectRatio
             }
        }

        width: {
            if (emulator.pictureMode === "stretch") {
                return root.width
            } else if (emulator.pictureMode === "aspect-ratio-fill") {
                return height * aspectRatio
            } else if (emulator.pictureMode === "integer-scale") {
                return height * aspectRatio
            }

            return emulator.videoWidth
        }
        height: {
            if (emulator.pictureMode === "stretch") {
                return root.height
            } else if (emulator.pictureMode === "aspect-ratio-fill") {
                return root.height
            } else if (emulator.pictureMode === "integer-scale"){
                let num = root.height / emulator.videoHeight
                return Math.floor(num) * emulator.videoHeight
            }

            return emulator.videoHeight
        }

        rewindEnabled: root.gameSettings.rewindEnabled

        smooth: false

        onAboutToRunFrame: {
            root.aboutToRunFrame()
        }

        onRewindPointsReady: function (points) {
            root.stackView.pushItem(rewindPage, {
                model: points,
                aspectRatio: emulator.aspectRatio
            }, StackView.Immediate)
            // root.rewindPointsReady(points)
        }

        onPlaybackMultiplierChanged: function() {
            if (emulator.playbackMultiplier === 1) {
                timer.running = true
            } else {
                speedIndicator.opacity = 1
            }
        }
    }

    Pane {
        id: speedIndicator
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins:16
        width: 100
        height: 40
        padding: 12

        opacity: 0

        Behavior on opacity {
            NumberAnimation {
                easing.type: Easing.InOutQuad
                duration: 200
            }
        }

        Timer {
            id: timer
            interval: 1200
            running: false
            repeat: false
            onTriggered: {
                speedIndicator.opacity = 0
            }
        }

        background: Rectangle {
            color: ColorPalette.neutral900
        }

        contentItem: Text {
            text: emulator.playbackMultiplier + "x"
            color: "white"
            font.pixelSize: 20
            font.family: Constants.regularFontFamily
            anchors.centerIn: parent
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
        }
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

    Component {
        id: rewindPage
        RewindMenu {
            onRewindPointSelected: function(index) {
                emulator.loadRewindPoint(index)
                root.stackView.popCurrentItem()
            }
        }
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
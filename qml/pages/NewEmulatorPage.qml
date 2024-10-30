import QtQuick
import QtQuick.Controls
// import QtQuick.Controls.Material
import QtQuick.Dialogs
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Effects
import Firelight 1.0

FocusScope {
    id: root

    clip: true

    required property var gameData
    required property var saveData
    required property var corePath
    required property var contentHash
    required property var saveSlotNumber
    required property var platformId
    required property var contentPath


    signal rewindPointsReady(var points)

    function resetGame() {
        emulator.resetGame()
    }

    function createRewindPoints() {
        emulator.createRewindPoints()
    }

    function loadSuspendPoint(index) {
        emulator.loadSuspendPoint(index)
    }

    function writeSuspendPoint(index) {
        emulator.writeSuspendPoint(index)
    }

    property real blurAmount: 0

    property alias paused: emulator.paused

    StackView.visible: true

    StackView.onDeactivating: function () {
        emulator.paused = true
    }
    StackView.onActivated: function () {
        emulator.paused = false
    }

    layer.enabled: blurAmount !== 0

    layer.effect: MultiEffect {
        // enabled: root.blurAmount !== 0
        source: root
        anchors.fill: root
        blurEnabled: true
        blurMultiplier: 0
        blurMax: 64
        blur: root.blurAmount
    }

    Rectangle {
        id: background
        color: "black"
        anchors.fill: parent
    }

    // layer.enabled: root.blurAmount !== 0
    // layer.effect:

    EmulatorItem {
        id: emulator
        // visible: root.blurAmount === 0
        focus: true
        anchors.centerIn: parent
        width: parent.height * videoAspectRatio
        height: parent.height
        smooth: false

        onRewindPointsReady: function (points) {
            root.rewindPointsReady(points)
        }

        Keys.onDigit1Pressed: {
            console.log("Digit 1 pressed")
            emulator.loadSuspendPoint(1)
        }

        Component.onCompleted: function () {
            startGame(gameData, saveData, corePath, contentHash, saveSlotNumber, platformId, contentPath)
            console.log("WE DID IT")
        }
    }

    FrameAnimation {
        id: frameAnimation
        running: true
        onTriggered: {
            bufferGraph.addValue(emulator.audioBufferLevel)
            frametimeGraph.addValue(frameAnimation.frameTime)
        }

    }
    Pane {
        id: frametimeGraph

        property var bufferValues: []

        function addValue(value) {
            bufferValues.push(value)
            if (bufferValues.length > 100) {
                bufferValues.shift()
            }
            frametimeGraphCanvas.requestPaint()
            // bufferGraph.update()
            // canvas.paint()
        }

        width: 250
        height: 120
        anchors.top: parent.top
        anchors.right: parent.right

        padding: 12
        background: Rectangle {
            color: "black"
        }

        contentItem: Canvas {
            id: frametimeGraphCanvas
            width: parent.width
            height: parent.height
            onPaint: {
                var ctx = getContext("2d");
                ctx.clearRect(0, 0, width, height);
                ctx.fillStyle = "white";
                for (var i = 0; i < frametimeGraph.bufferValues.length; i++) {
                    var x = (i / frametimeGraph.bufferValues.length) * width;
                    var y = height - ((frametimeGraph.bufferValues[i] * 1000 / 128) * height);
                    ctx.fillRect(x, y, width / frametimeGraph.bufferValues.length, height);
                }
                ctx.fillStyle = "green";
                ctx.fillRect(0, height * 7 / 8, width, 1); // Draw a middle line
            }

        }

    }
    Pane {
        id: bufferGraph

        property var bufferValues: []

        function addValue(value) {
            bufferValues.push(value)
            if (bufferValues.length > 100) {
                bufferValues.shift()
            }
            canvas.requestPaint()
            // bufferGraph.update()
            // canvas.paint()
        }

        width: 250
        height: 120
        anchors.top: frametimeGraph.bottom
        anchors.right: parent.right

        padding: 12
        background: Rectangle {
            color: "black"
        }

        contentItem: Canvas {
            id: canvas
            width: parent.width
            height: parent.height
            onPaint: {
                var ctx = getContext("2d");
                ctx.clearRect(0, 0, width, height);
                ctx.fillStyle = "white";
                for (var i = 0; i < bufferGraph.bufferValues.length; i++) {
                    var x = (i / bufferGraph.bufferValues.length) * width;
                    var y = height - (bufferGraph.bufferValues[i] * height);
                    ctx.fillRect(x, y, width / bufferGraph.bufferValues.length, height);
                }
                ctx.fillStyle = "green";
                ctx.fillRect(0, height / 2, width, 1); // Draw a middle line
            }

        }

    }

    Text {
        id: buffer
        text: emulator.audioBufferLevel
        color: "white"
        font.pixelSize: 14
        font.family: Constants.regularFontFamily
    }
}
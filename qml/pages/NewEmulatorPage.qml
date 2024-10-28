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

    function resetGame() {
        emulator.resetGame()
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

        Component.onCompleted: function () {
            startGame(gameData, saveData, corePath, contentHash, saveSlotNumber, platformId, contentPath)
            console.log("WE DID IT")
        }
    }
}
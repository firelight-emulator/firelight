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

    property real blurAmount: 0

    property alias paused: emulator.paused

    StackView.visible: true

    StackView.onDeactivating: function () {
        emulator.paused = true
    }
    StackView.onActivated: function () {
        emulator.paused = false
    }

    Rectangle {
        id: background
        color: "black"
        anchors.fill: parent
    }

    EmulatorItem {
        id: emulator
        anchors.centerIn: parent
        width: videoWidth * 3
        height: videoHeight * 3
        smooth: false

        layer.enabled: root.blurAmount !== 0
        layer.effect: MultiEffect {
            source: emulator
            anchors.fill: emulator
            blurEnabled: true
            blurMultiplier: 0
            blurMax: 64
            blur: root.blurAmount
        }

        Component.onCompleted: function () {
            startGame(gameData, saveData, corePath, contentHash, saveSlotNumber, platformId, contentPath)
            console.log("WE DID IT")
        }
    }
}